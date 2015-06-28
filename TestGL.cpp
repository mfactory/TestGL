#include  <windows.h>
#include  <GL/gl.h>

HGLRC g_hGLRC = NULL;
LRESULT CALLBACK WindowProc( HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam );

void OnCreate( HWND hWnd );
void OnPaint( HWND hWnd );

int WINAPI WinMain( HINSTANCE hCurrInstance, HINSTANCE hPrevInstance, LPSTR szArgs, int nWinMode )
{
	WNDCLASSEX wc = { sizeof (WNDCLASSEX) };
	wc.hInstance = hCurrInstance;
	wc.lpszClassName = L"TestGL";
	wc.lpfnWndProc = WindowProc;
	wc.style = 0;
	wc.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = NULL;
	wc.hCursor = ::LoadCursor( NULL, IDC_ARROW );
	wc.lpszMenuName = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
	if (!RegisterClassEx(&wc)) {
		return 0;
	}
	
	HWND hWnd = ::CreateWindow( L"TestGL", L"TestGL", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			CW_USEDEFAULT, CW_USEDEFAULT, 250, 250, NULL, NULL, hCurrInstance, NULL );
	
	if (!hWnd) {
		return 0;
	}
	
	::ShowWindow( hWnd, nWinMode );
	MSG msg = { 0 };
	while ( ::GetMessage( &msg, NULL, 0, 0 ) ) {
		::TranslateMessage( &msg );
		::DispatchMessage( &msg );
	}
	
	return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
		case WM_CREATE:
			OnCreate( hWnd );
			break;
		case WM_PAINT:
			OnPaint( hWnd );
			break;
		case WM_DESTROY:
			if (g_hGLRC) {
				::wglDeleteContext(g_hGLRC);
			}
			::PostQuitMessage(0);
			break;
		default:
			return ::DefWindowProc( hWnd, nMessage, wParam, lParam );
	}
	return 0;
}

void OnCreate(HWND hWnd)
{
	static PIXELFORMATDESCRIPTOR pfd = { 
		sizeof (PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0,
		0, 0, 0,
		0, 0,
		0, 0, 0, 0, 0,
		32,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0,
		0,
		0
	};
	
	HDC hDC = GetDC(hWnd);
	int pfdID = ::ChoosePixelFormat( hDC, &pfd );
	if ( !pfdID ) {
		return;
	}
	
	BOOL bResult = ::SetPixelFormat( hDC, pfdID, &pfd );
	if ( !bResult ) {
		return;
	}
	
	g_hGLRC = ::wglCreateContext( hDC );
	if ( !g_hGLRC ) {
		return;
	}
	
	::ReleaseDC( hWnd, hDC );
}

void OnPaint(HWND hWnd)
{
	PAINTSTRUCT ps = { 0 };
	HDC hDC = ::BeginPaint( hWnd, &ps );
	
	BYTE* pDataSrc = nullptr;
	BITMAP BM = { 0 };
	{
		RECT rcClient;
		::GetClientRect( hWnd, &rcClient );
		int nCX = rcClient.right - rcClient.left;
		int nCY = rcClient.bottom - rcClient.top;
		
		HWND hwndDesk = ::GetDesktopWindow();
		HDC hdcDesk = ::GetWindowDC( hwndDesk );
		
		HBITMAP hBmp = ::CreateCompatibleBitmap( hdcDesk, nCX, nCY );
		
		HDC hdcMem = ::CreateCompatibleDC( hdcDesk );
		::SelectObject( hdcMem, hBmp );
		::BitBlt( hdcMem, 0, 0, nCX, nCY, hdcDesk, 0, 0, SRCCOPY );
		
		::GetObject( hBmp, sizeof(BM), &BM);
		
		int sizeSrc = BM.bmWidthBytes * BM.bmHeight;
		pDataSrc = new BYTE[sizeSrc];
		DWORD dwSize = ::GetBitmapBits( hBmp, sizeSrc, pDataSrc );
		
		::ReleaseDC( hwndDesk, hdcDesk );
		::DeleteObject( hdcMem );
		::DeleteDC( hdcMem );
	}
	
	::wglMakeCurrent( hDC, g_hGLRC );
	
	GLuint tempID = 0;
	// make texture
	{
		::glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
		::glClear( GL_COLOR_BUFFER_BIT );
		::glColor3f( 1.0f, 1.0f, 1.0f );
		
		::glGenTextures( 1, &tempID );
		::glBindTexture( GL_TEXTURE_2D, tempID );
		::glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );	
		::glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		if ( BM.bmBitsPixel == 24 ) {
			::glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, BM.bmWidth,BM.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pDataSrc );
		}
		else if ( BM.bmBitsPixel == 32 ) {
			::glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, BM.bmWidth,BM.bmHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pDataSrc );
		}
		
		::glBindTexture(GL_TEXTURE_2D, 0);
	}
	
	// bind texture
	::glEnable( GL_TEXTURE_2D );
	::glBindTexture( GL_TEXTURE_2D, tempID );
	::glBegin( GL_QUADS );
		::glTexCoord2d( 0.0, 0.0 );		glVertex3d(-1.0,  1.0,  0.0);
		::glTexCoord2d( 0.0, 1.0 );		glVertex3d(-1.0, -1.0,  0.0);
		::glTexCoord2d( 1.0, 1.0 );		glVertex3d( 1.0, -1.0,  0.0);
		::glTexCoord2d( 1.0, 0.0 );		glVertex3d( 1.0,  1.0,  0.0);
	::glEnd();
	::glDisable( GL_TEXTURE_2D );
	
	::glFlush();
	::wglMakeCurrent(NULL,NULL);
	
	if ( pDataSrc ) {
		delete []pDataSrc;
	}
	
	::EndPaint( hWnd, &ps );
}
