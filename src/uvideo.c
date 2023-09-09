#include <uvideo.h>
#include <ushape.h>
#include <utext.h>
#include <stdio.h>

video_struct* video_;

static LONG WINAPI WndProc(HWND,UINT,WPARAM,LPARAM);
static int create_window(int,int,int);
static void destroy_window();
static void reset_size();

int init_video()
{
	video_ = (video_struct*)malloc(sizeof(video_struct));
	memset(video_, 0, sizeof(video_struct));
	video_->hInstance = 0;
	video_->w = 1920;
	video_->h = 1080;
	
	if(!create_window(0,WS_OVERLAPPEDWINDOW,PFD_DOUBLEBUFFER))
	{
		free(video_);
		return 0;
	}
	
	if (!gladLoadGL())
    {
		fprintf(stderr,"Failed to initialize GLAD\n");
		destroy_video();
        return 0;
    }
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	
	reset_size();
	
	ShowWindow(video_->hWnd, SW_SHOW);
	
	return 1;
}

void destroy_video()
{
	if(video_) {
		destroy_window();
		free(video_);
	}
	
	video_ = NULL;
}

int create_program(const char* vert, const char* frag) {
    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	
    glShaderSource(vertexShader, 1, &vert, NULL);
    glCompileShader(vertexShader);
	
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
		return -1;
    }
	
    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &frag, NULL);
    glCompileShader(fragmentShader);
	
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
		return -1; 
    }
	
	// link shaders
    int vertexProgram = glCreateProgram();
	
    glAttachShader(vertexProgram, vertexShader);
    glAttachShader(vertexProgram, fragmentShader);
    glLinkProgram(vertexProgram);
	
    glDeleteShader(vertexShader);
	
	return vertexProgram;
}

static LONG WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
    switch(uMsg) {
		case WM_PAINT:
			PAINTSTRUCT ps;
			BeginPaint(video_->hWnd, &ps);
			
			EndPaint(video_->hWnd, &ps);
		return 0;
		
		case WM_KEYDOWN:
		switch (wParam) {
				default:
				break;
			}
		return 0;
		
		case WM_CLOSE:
			PostQuitMessage(0);
		return 0;
		
		case WM_SIZE:
		case WM_EXITSIZEMOVE:
			reset_size();
		return 0;
		
		case WM_SETCURSOR:
		return TRUE;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam); 
}

static int create_window(int exFlags, int wsFlags, int pfdflags)
{
	WNDCLASS	wc;

    /* only register the window class once - use hInstance as a flag. */
	if(!video_->hInstance)
	{
		video_->hInstance = GetModuleHandle(NULL);
		memset(&wc, 0, sizeof(WNDCLASS));
		wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc   = (WNDPROC)WndProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = video_->hInstance;
		wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
		wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = "OpenGL";

		if (!RegisterClass(&wc)) {
			MessageBox(NULL, "RegisterClass() failed:  "
				   "Cannot register window class.", "Error", MB_OK);
			return 0;
		}
	}
	
	HWND hWnd;
	memset(&hWnd, 0, sizeof(HWND));
    hWnd = CreateWindowEx(WS_EX_ACCEPTFILES|exFlags, "OpenGL", "UNAS", WS_CLIPSIBLINGS|WS_CLIPCHILDREN|wsFlags,
			0, 0, video_->w, video_->h, NULL, NULL, video_->hInstance, NULL);
	
    if (hWnd == NULL) {
	MessageBox(NULL, "CreateWindow() failed:  Cannot create a window.",
		   "Error", MB_OK);
	return 0;
    }
	
	HDC hDC;
	memset(&hDC, 0, sizeof(HDC));
    hDC = GetDC(hWnd);

    /* there is no guarantee that the contents of the stack that become
       the pfd are zeroed, therefore _make sure_ to clear these bits. */
    PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | pfdflags,  //Flags
		PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
		32,                   // Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,                   // Number of bits for the depthbuffer
		8,                    // Number of bits for the stencilbuffer
		0,                    // Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

    int pf = ChoosePixelFormat(hDC, &pfd);
    if (pf == 0) {
	MessageBox(NULL, "ChoosePixelFormat() failed:  "
		   "Cannot find a suitable pixel format.", "Error", MB_OK); 
	return 0;
    } 
 
    if (SetPixelFormat(hDC, pf, &pfd) == FALSE) {
	MessageBox(NULL, "SetPixelFormat() failed:  "
		   "Cannot set format specified.", "Error", MB_OK);
	return 0;
    } 

	HGLRC hRC;
	
    hDC = GetDC(hWnd);
    
	hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);
	
	video_->hWnd = hWnd;
	video_->hDC = hDC;
	video_->hRC = hRC;
	
	return 1;
}

void set_cursor(int flag)
{
	switch(flag)
	{
		case 0:
			SetCursor(NULL);
			break;
		case 1:
			SetCursor(LoadCursor(NULL,IDC_ARROW));
			break;
		case 2:
			SetCursor(LoadCursor(NULL,IDC_HAND));
			break;
	}
}

void destroy_window()
{
	wglMakeCurrent(NULL, NULL);
    ReleaseDC(video_->hWnd, video_->hDC);
    wglDeleteContext(video_->hRC);
    DestroyWindow(video_->hWnd);
}

static void reset_size() {
	GetClientRect(video_->hWnd,&video_->lPrect);
	video_->w = video_->lPrect.right - video_->lPrect.left;
	video_->h = video_->lPrect.bottom - video_->lPrect.top;
	glViewport(video_->lPrect.left,video_->lPrect.top,video_->lPrect.right,video_->lPrect.bottom);
	
	update_textvp();
	update_shapesvp();
			
	video_->shouldReload_ = 1;
}