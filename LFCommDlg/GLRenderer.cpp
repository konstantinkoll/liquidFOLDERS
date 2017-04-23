
// GLRenderer.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include <math.h>


// GLRenderer
//

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;
PFNGLMULTITEXCOORD2FPROC glMultiTexCoord2f = NULL;
PFNGLDRAWBUFFERSPROC glDrawBuffers = NULL;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers = NULL;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer = NULL;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = NULL;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample = NULL;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers = NULL;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = NULL;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = NULL;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = NULL;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = NULL;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer = NULL;

GLRenderer theRenderer;

GLRenderer::GLRenderer()
{
	m_Initialized = m_BlueMarbleCompressed = FALSE;

	m_nPictureBlueMarble = m_nPictureClouds = 0;
	m_pPictureBlueMarble = m_pPictureClouds = NULL;
}

GLRenderer::~GLRenderer()
{
	if (m_pPictureBlueMarble)
		m_pPictureBlueMarble->Release();

	if (m_pPictureClouds)
		m_pPictureClouds->Release();
}

BOOL GLRenderer::Initialize()
{
	if (m_Initialized)
		return TRUE;

	// Create dummy window
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	CWnd wndDummy;
	wndDummy.CreateEx(0, className, _T(""), WS_POPUP, CRect(0, 0, 16, 16), NULL, 0);

	// Get DC
	HDC hDC = GetDC(wndDummy);
	if (SetupPixelFormat(hDC))
	{
		// Create rendering context
		HGLRC hRC = wglCreateContext(hDC);
		if (hRC)
		{
			wglMakeCurrent(hDC, hRC);

			// Basic info
			p_Extensions = (LPCSTR)glGetString(GL_EXTENSIONS);
			p_Vendor = (LPCSTR)glGetString(GL_VENDOR);
			p_Version = (LPCSTR)glGetString(GL_VERSION);

			UINT VersionMajor = 0;
			UINT VersionMinor = 0;
			if (p_Version)
				sscanf_s(p_Version, "%u.%u", &VersionMajor, &VersionMinor);

			// Hardware info
			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_MaxTextureSize);
			glGetIntegerv(GL_MAX_TEXTURE_UNITS, &m_TextureUnits);

			// Extension info
			m_SupportsFilterAnisotropic = strstr(p_Extensions, "GL_EXT_texture_filter_anisotropic")!=NULL;
			m_SupportsFramebufferMultisample = (strstr(p_Extensions, "GL_EXT_framebuffer_multisample")!=NULL) && (strstr(p_Extensions, "GL_EXT_framebuffer_blit")!=NULL);
			m_SupportsFramebufferObject = (strstr(p_Extensions, "GL_ARB_framebuffer_object")!=NULL) || ((strstr(p_Extensions, "GL_EXT_framebuffer_object")!=NULL));
			m_SupportsGenerateMipmap = strstr(p_Extensions, "SGIS_generate_mipmap")!=NULL;
			m_SupportsRescaleNormal = strstr(p_Extensions, "GL_EXT_rescale_normal")!=NULL;
			m_SupportsTextureCombine = strstr(p_Extensions, "GL_EXT_texture_env_combine")!=NULL;
			m_SupportsTextureCompression = strstr(p_Extensions, "GL_EXT_texture_compression_s3tc")!=NULL;

			if (m_SupportsFramebufferMultisample)
				glGetIntegerv(GL_MAX_SAMPLES_EXT, &m_MaxSamples);

			if (m_SupportsFilterAnisotropic)
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_MaxAnisotropy);

			// Functions
			glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
			glMultiTexCoord2f = (PFNGLMULTITEXCOORD2FPROC)wglGetProcAddress("glMultiTexCoord2f");
			glDrawBuffers = (PFNGLDRAWBUFFERSPROC)wglGetProcAddress("glDrawBuffers");
			glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)wglGetProcAddress("glGenRenderbuffers");
			glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)wglGetProcAddress("glBindRenderbuffer");
			glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)wglGetProcAddress("glRenderbufferStorage");
			glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)wglGetProcAddress("glRenderbufferStorageMultisample");
			glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)wglGetProcAddress("glDeleteRenderbuffers");
			glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
			glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
			glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffers");
			glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress("glFramebufferRenderbuffer");
			glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
			glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)wglGetProcAddress("glBlitFramebuffer");

			// Highlevel info
			m_SupportsFramebuffer = m_SupportsFramebufferObject && (glDrawBuffers!=NULL) &&
				(glGenRenderbuffers!=NULL) && (glBindRenderbuffer!=NULL) && (glRenderbufferStorage!=NULL) && (glDeleteRenderbuffers!=NULL) &&
				(glGenFramebuffers!=NULL) && (glBindFramebuffer!=NULL) && (glDeleteFramebuffers!=NULL) &&
				(glFramebufferRenderbuffer!=NULL) && (glCheckFramebufferStatus!=NULL);
			m_SupportsGlobeClouds = m_SupportsTextureCombine && (m_TextureUnits>=3) && (glActiveTexture!=NULL) && (glMultiTexCoord2f!=NULL);
			m_SupportsMultisample = m_SupportsFramebuffer && m_SupportsFramebufferMultisample && (glRenderbufferStorageMultisample!=NULL) && (glBlitFramebuffer!=NULL);

			m_MaxModelQuality = m_SupportsGlobeClouds ? MODELULTRA : MODELHIGH;
			m_MaxTextureQuality = (VersionMajor>=3) && (m_MaxTextureSize>=8192) ? TEXTUREULTRA : (m_MaxTextureSize>=4096) ? TEXTUREMEDIUM : TEXTURELOW;

			if (m_MaxTextureQuality==TEXTUREULTRA)
			{
				glTexImage2D(GL_PROXY_TEXTURE_2D, 1, 4, 8192, 8192, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

				GLint ProxyWidth;
				glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 1, GL_TEXTURE_WIDTH, &ProxyWidth);

				GLint ProxyHeight;
				glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 1, GL_TEXTURE_HEIGHT, &ProxyHeight);

				if ((ProxyWidth!=8192) || (ProxyHeight!=8192))
					m_MaxTextureQuality = TEXTUREMEDIUM;
			}

			// Delete temporary rendering context
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hRC);
		}

		m_Initialized = TRUE;
	}

	ReleaseDC(wndDummy, hDC);
	wndDummy.DestroyWindow();

	return m_Initialized;
}

void GLRenderer::MatrixMultiplication4f(GLfloat Result[4][4], const GLfloat Left[4][4], const GLfloat Right[4][4])
{
	Result[0][0] = Left[0][0]*Right[0][0] + Left[0][1]*Right[1][0] + Left[0][2]*Right[2][0] + Left[0][3]*Right[3][0];
	Result[0][1] = Left[0][0]*Right[0][1] + Left[0][1]*Right[1][1] + Left[0][2]*Right[2][1] + Left[0][3]*Right[3][1];
	Result[0][2] = Left[0][0]*Right[0][2] + Left[0][1]*Right[1][2] + Left[0][2]*Right[2][2] + Left[0][3]*Right[3][2];
	Result[0][3] = Left[0][0]*Right[0][3] + Left[0][1]*Right[1][3] + Left[0][2]*Right[2][3] + Left[0][3]*Right[3][3];
	Result[1][0] = Left[1][0]*Right[0][0] + Left[1][1]*Right[1][0] + Left[1][2]*Right[2][0] + Left[1][3]*Right[3][0];
	Result[1][1] = Left[1][0]*Right[0][1] + Left[1][1]*Right[1][1] + Left[1][2]*Right[2][1] + Left[1][3]*Right[3][1];
	Result[1][2] = Left[1][0]*Right[0][2] + Left[1][1]*Right[1][2] + Left[1][2]*Right[2][2] + Left[1][3]*Right[3][2];
	Result[1][3] = Left[1][0]*Right[0][3] + Left[1][1]*Right[1][3] + Left[1][2]*Right[2][3] + Left[1][3]*Right[3][3];
	Result[2][0] = Left[2][0]*Right[0][0] + Left[2][1]*Right[1][0] + Left[2][2]*Right[2][0] + Left[2][3]*Right[3][0];
	Result[2][1] = Left[2][0]*Right[0][1] + Left[2][1]*Right[1][1] + Left[2][2]*Right[2][1] + Left[2][3]*Right[3][1];
	Result[2][2] = Left[2][0]*Right[0][2] + Left[2][1]*Right[1][2] + Left[2][2]*Right[2][2] + Left[2][3]*Right[3][2];
	Result[2][3] = Left[2][0]*Right[0][3] + Left[2][1]*Right[1][3] + Left[2][2]*Right[2][3] + Left[2][3]*Right[3][3];
	Result[3][0] = Left[3][0]*Right[0][0] + Left[3][1]*Right[1][0] + Left[3][2]*Right[2][0] + Left[3][3]*Right[3][0];
	Result[3][1] = Left[3][0]*Right[0][1] + Left[3][1]*Right[1][1] + Left[3][2]*Right[2][1] + Left[3][3]*Right[3][1];
	Result[3][2] = Left[3][0]*Right[0][2] + Left[3][1]*Right[1][2] + Left[3][2]*Right[2][2] + Left[3][3]*Right[3][2];
	Result[3][3] = Left[3][0]*Right[0][3] + Left[3][1]*Right[1][3] + Left[3][2]*Right[2][3] + Left[3][3]*Right[3][3];
}

BOOL GLRenderer::SetupPixelFormat(HDC hDC, DWORD dwFlags)
{
	const PIXELFORMATDESCRIPTOR PixelFormatDescriptor =
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// Size of this pfd
		1,								// Version number
		PFD_DRAW_TO_WINDOW |			// Support window
		PFD_SUPPORT_OPENGL |			// Support OpenGL
		dwFlags,						// Other flags
		32,								// 32-bit color depth
		0, 0, 0, 0, 0, 0,				// Color bits ignored
		0,								// No alpha buffer
		0,								// Shift bit ignored
		0,								// No accumulation buffer
		0, 0, 0, 0,						// Accumulcation bits ignored
		0,								// No z-buffer
		0,								// No stencil buffer
		0,								// No auxiliary buffer
		PFD_MAIN_PLANE,					// Main layer
		0,								// Reserved
		0, 0, 0							// Layer masks ignored
	};

	INT PixelFormat = ChoosePixelFormat(hDC, &PixelFormatDescriptor);

	return PixelFormat ? SetPixelFormat(hDC, PixelFormat, &PixelFormatDescriptor) : FALSE;
}

void GLRenderer::ColorRef2GLColor(GLcolor& DstColor, COLORREF SrcColor, GLfloat Alpha)
{
	DstColor[0] = (SrcColor & 0xFF)/255.0f;
	DstColor[1] = ((SrcColor>>8) & 0xFF)/255.0f;
	DstColor[2] = ((SrcColor>>16) & 0xFF)/255.0f;
	DstColor[3] = Alpha;
}

void GLRenderer::SetColor(COLORREF SrcColor, GLfloat Alpha)
{
	glColor4f((SrcColor & 0xFF)/255.0f, ((SrcColor>>8) & 0xFF)/255.0f, ((SrcColor>>16) & 0xFF)/255.0f, Alpha);
}

void GLRenderer::Project2D() const
{
	GLint Viewport[4];
	glGetIntegerv(GL_VIEWPORT, Viewport);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(Viewport[0], Viewport[0]+Viewport[2], Viewport[1]+Viewport[3], Viewport[1], -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.375, 0.375, 0);
}

void GLRenderer::Vertex5f(GLfloat x, GLfloat y, GLfloat z, GLfloat s, GLfloat t)
{
	if (glMultiTexCoord2f)
	{
		glMultiTexCoord2f(GL_TEXTURE0, s, t);
		glMultiTexCoord2f(GL_TEXTURE1, s, t);
	}
	else
	{
		glTexCoord2f(s, t);
	}

	glNormal3f(x, y, z);
	glVertex3f(x, y, z);
}

GLuint GLRenderer::CreateGlobe(UINT cx) const
{
	ASSERT(cx>=8);

	UINT cy = cx/2;

	// Create grid
	//
	GLfloat* pGrid = new GLfloat[cx*(cy-1)*3];
	UINT GridPos = 0;

	GLfloat StepAlpha = 2.0f*PI/(GLfloat)cx;
	GLfloat StepBeta = PI/(GLfloat)cy;
	GLfloat Beta = -PI/2.0f+StepBeta;

	for (UINT y=0; y<(cy-1); y++)
	{
		GLfloat Alpha = -PI;

		for (UINT x=0; x<cx; x++)
		{
			pGrid[GridPos++] = sin(Alpha)*cos(Beta);
			pGrid[GridPos++] = sin(Beta);
			pGrid[GridPos++] = cos(Alpha)*cos(Beta);

			Alpha += StepAlpha;
		}

		Beta += StepBeta;
	}

	// Create display list
	//
	GLuint Index = glGenLists(1);
	glNewList(Index, GL_COMPILE);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_TRIANGLES);

	// Top
	for (UINT x=0; x<cx; x++)
	{
		Vertex5f(0.0f, -1.0f, 0.0f, (GLfloat)(x+0.5f)/(GLfloat)cx, 0.0f);
		Vertex5f(&pGrid[((x+1)%cx*3)], (GLfloat)(x+1)/(GLfloat)cx, 1.0f/(GLfloat)cy);
		Vertex5f(&pGrid[(x%cx)*3], (GLfloat)x/(GLfloat)cx, 1.0f/(GLfloat)cy);
	}

	// Side
	for(UINT y=0; y<cy-2; y++)
		for(UINT x=0; x<cx; x++)
		{
			Vertex5f(&pGrid[(y*cx+x%cx)*3], (GLfloat)x/(GLfloat)cx, (GLfloat)(y+1)/(GLfloat)cy);
			Vertex5f(&pGrid[(y*cx+(x+1)%cx)*3], (GLfloat)(x+1)/(GLfloat)cx, (GLfloat)(y+1)/(GLfloat)cy);
			Vertex5f(&pGrid[((y+1)*cx+(x+1)%cx)*3], (GLfloat)(x+1)/(GLfloat)cx, (GLfloat)(y+2)/(GLfloat)cy);

			Vertex5f(&pGrid[((y+1)*cx+(x+1)%cx)*3], (GLfloat)(x+1)/(GLfloat)cx, (GLfloat)(y+2)/(GLfloat)cy);
			Vertex5f(&pGrid[((y+1)*cx+x%cx)*3], (GLfloat)x/(GLfloat)cx, (GLfloat)(y+2)/(GLfloat)cy);
			Vertex5f(&pGrid[(y*cx+x%cx)*3], (GLfloat)x/(GLfloat)cx, (GLfloat)(y+1)/(GLfloat)cy);
		}

	// Bottom
	for (UINT x=0; x<cx; x++)
	{
		Vertex5f(&pGrid[((cy-2)*cx+x%cx)*3], (GLfloat)x/(GLfloat)cx, (GLfloat)(cy-1)/(GLfloat)cy);
		Vertex5f(&pGrid[((cy-2)*cx+(x+1)%cx)*3], (GLfloat)(x+1)/(GLfloat)cx, (GLfloat)(cy-1)/(GLfloat)cy);
		Vertex5f(0.0f, 1.0f, 0.0f, (GLfloat)(x+0.5f)/(GLfloat)cx, 1.0f);
	}

	// Finish
	//
	glEnd();
	glEndList();

	delete[] pGrid;

	return Index;
}

GLuint GLRenderer::CreateTexture(UINT Width, UINT Height, UINT BPP, void* pData, BOOL ForceIntensity, BOOL Repeat, BOOL Mipmap, BOOL Compress) const
{
	ASSERT((BPP==4) || (BPP==3) || (BPP==1));

	GLuint nID = 0;

	// Textur erzeugen
	glGenTextures(1, &nID);
	if (!nID)
		return 0;

	// Textur-Parameter und -Daten
	Compress &= LFGetApp()->m_TextureCompress && m_SupportsTextureCompression;

	GLint InternalFormat = ((BPP==1) || ForceIntensity) ? GL_INTENSITY : Compress ? (BPP==4) ? GL_COMPRESSED_RGBA_S3TC_DXT3_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT : BPP;
	GLenum PixelMode = (BPP==4) ? GL_BGRA : (BPP==3) ? GL_BGR : GL_INTENSITY;

	glBindTexture(GL_TEXTURE_2D, nID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Repeat ? GL_REPEAT : GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Repeat ? GL_REPEAT : GL_CLAMP);

	if (Mipmap)
	{
		// Mipmap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		// Anisotropic filtering
		if (m_SupportsFilterAnisotropic)
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_MaxAnisotropy);

		if (m_SupportsGenerateMipmap)
		{
			// Hardware-Erzeugung von Mipmaps aktiv
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
			glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, PixelMode, GL_UNSIGNED_BYTE, pData);
		}
		else
		{
			// Mipmaps von der CPU erzeugen lassen
			gluBuild2DMipmaps(GL_TEXTURE_2D, InternalFormat, Width, Height, PixelMode, GL_UNSIGNED_BYTE, pData);
		}
	}
	else
	{
		// No mipmap
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, PixelMode, GL_UNSIGNED_BYTE, pData);
	}

	return nID;
}

GLuint GLRenderer::CreateTexture(HBITMAP hBitmap, BOOL ForceIntensity, BOOL Repeat, BOOL Mipmap, BOOL Compress) const
{
	ASSERT(hBitmap);

	BITMAP BMP;
	GetObject(hBitmap, sizeof(BMP), &BMP);

	return CreateTexture(BMP.bmWidth, BMP.bmHeight, BMP.bmBitsPixel>>3, BMP.bmBits, ForceIntensity, Repeat, Mipmap, Compress);
}

GLuint GLRenderer::CreateTexture(Bitmap* pBitmap, BOOL ForceIntensity, BOOL Repeat, BOOL Mipmap, BOOL Compress) const
{
	ASSERT(pBitmap);

	GLuint nID = 0;

	HBITMAP hBitmap;
	pBitmap->GetHBITMAP(NULL, &hBitmap);

	if (hBitmap)
	{
		nID = theRenderer.CreateTexture(hBitmap, ForceIntensity, Repeat, Mipmap, Compress);
		DeleteObject(hBitmap);
	}

	return nID;
}

void GLRenderer::CreateTexture(UINT nResID, UINT& nPictureID, IPicture*& pPicture, GLuint& nTextureID, BOOL ForceIntensity, BOOL Repeat, BOOL Mipmap, BOOL Compress) const
{
	nResID += min((UINT)m_MaxTextureQuality, (UINT)LFGetApp()->m_TextureQuality);

	if (nResID!=nPictureID)
	{
		if (pPicture)
		{
			pPicture->Release();
			pPicture = NULL;

			nPictureID = 0;
		}

		if (nTextureID)
		{
			glDeleteTextures(1, &nTextureID);
			nTextureID = 0;
		}

		HRSRC hResource = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nResID), RT_RCDATA);
		if (hResource)
		{
			HGLOBAL hMemory = LoadResource(AfxGetResourceHandle(), hResource);
			if (hMemory)
			{
				LPVOID pResourceData = LockResource(hMemory);
				if (pResourceData)
				{
					DWORD Size = SizeofResource(AfxGetResourceHandle(), hResource);
					if (Size)
					{
						IStream* pStream = SHCreateMemStream((BYTE*)pResourceData, Size);

						OleLoadPicture(pStream, 0, FALSE, IID_IPicture, (void**)&pPicture);

						pStream->Release();
					}
				}
			}
		}

		if (pPicture)
			nPictureID = nResID;
	}

	if (!nTextureID)
	{
		HBITMAP hBitmap;

		if (SUCCEEDED(pPicture->get_Handle((OLE_HANDLE*)&hBitmap)))
			nTextureID = CreateTexture(hBitmap, ForceIntensity, Repeat, Mipmap, Compress);
	}
}

void GLRenderer::CreateTextureBlueMarble(GLuint& nTextureID, BOOL Mipmap)
{
	if (m_BlueMarbleCompressed!=LFGetApp()->m_TextureCompress)
		m_nPictureBlueMarble = 0;

	CreateTexture(IDB_BLUEMARBLE_1024, m_nPictureBlueMarble, m_pPictureBlueMarble, nTextureID, FALSE, TRUE, Mipmap);

	m_BlueMarbleCompressed = LFGetApp()->m_TextureCompress;
}

void GLRenderer::CreateTextureLocationIndicator(GLuint& nTextureID) const
{
	HBITMAP hBitmap = CreateTransparentBitmap(64, 64);

	CDC dc;
	dc.CreateCompatibleDC(NULL);

	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	Graphics g(dc);
	DrawLocationIndicator(g, 1, 1, 62);

	dc.SelectObject(hOldBitmap);

	nTextureID = CreateTexture(hBitmap);

	DeleteObject(hBitmap);
}

BOOL GLRenderer::CreateRenderContext(CWnd* pWnd, GLRenderContext& RenderContext) const
{
	ASSERT(pWnd);

	ZeroMemory(&RenderContext, sizeof(RenderContext));
	RenderContext.MaxWidth = GetSystemMetrics(SM_CXSCREEN);
	RenderContext.MaxHeight = GetSystemMetrics(SM_CYSCREEN);

	// Create context
	RenderContext.pDC = new CClientDC(pWnd);
	if (!RenderContext.pDC)
		return FALSE;

	if (!SetupPixelFormat(*RenderContext.pDC, PFD_DOUBLEBUFFER | PFD_TYPE_RGBA))
	{
		delete RenderContext.pDC;
		RenderContext.pDC = NULL;

		return FALSE;
	}

	RenderContext.hRC = wglCreateContext(*RenderContext.pDC);
	if (!RenderContext.hRC)
	{
		delete RenderContext.pDC;
		RenderContext.pDC = NULL;

		return FALSE;
	}

	MakeCurrent(RenderContext);

	// Initialize framebuffers
	if (m_SupportsFramebuffer)
	{
		glGenFramebuffers(1, &RenderContext.fbResolve);
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, RenderContext.fbResolve);

		glGenRenderbuffers(1, &RenderContext.rbColorResolve);
		glBindRenderbuffer(GL_RENDERBUFFER_EXT, RenderContext.rbColorResolve);
		glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_RGBA8, RenderContext.MaxWidth, RenderContext.MaxHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, RenderContext.rbColorResolve);

		glGenRenderbuffers(1, &RenderContext.rbDepthResolve);
		glBindRenderbuffer(GL_RENDERBUFFER_EXT, RenderContext.rbDepthResolve);
		glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, RenderContext.MaxWidth, RenderContext.MaxHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, RenderContext.rbDepthResolve);

		// Allocate additional framebuffer as multisample target (blitting to resolve buffer required)
		if (m_SupportsMultisample)
		{
			glGenFramebuffers(1, &RenderContext.fbMultisample);
			glBindFramebuffer(GL_FRAMEBUFFER_EXT, RenderContext.fbMultisample);

			glGenRenderbuffers(1, &RenderContext.rbColorMultisample);
			glBindRenderbuffer(GL_RENDERBUFFER_EXT, RenderContext.rbColorMultisample);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER_EXT, m_MaxSamples, GL_RGBA8, RenderContext.MaxWidth, RenderContext.MaxHeight);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, RenderContext.rbColorMultisample);

			glGenRenderbuffers(1, &RenderContext.rbDepthMultisample);
			glBindRenderbuffer(GL_RENDERBUFFER_EXT, RenderContext.rbDepthMultisample);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER_EXT, m_MaxSamples, GL_DEPTH_COMPONENT, RenderContext.MaxWidth, RenderContext.MaxHeight);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, RenderContext.rbDepthMultisample);
		}

		glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
	}

	// Basic 3D settings
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glFogi(GL_FOG_MODE, GL_LINEAR);
	glHint(GL_FOG_HINT, GL_NICEST);

	glDepthFunc(GL_LEQUAL);

	return TRUE;
}

void GLRenderer::BeginRender(CWnd* pWnd, GLRenderContext& RenderContext) const
{
	ASSERT(pWnd);
	ASSERT(RenderContext.hRC);

	// Setup width, height and bitmap buffer
	CRect rectClient;
	pWnd->GetClientRect(rectClient);

	INT Width = rectClient.Width();
	INT Height = rectClient.Height();
	ASSERT(Width<=RenderContext.MaxWidth);
	ASSERT(Height<=RenderContext.MaxHeight);

	if (m_SupportsFramebuffer)
		if ((Width!=RenderContext.Width) || (Height!=RenderContext.Height))
		{
			DeleteObject(RenderContext.hBitmap);
			RenderContext.hBitmap = CreateTransparentBitmap(Width, -Height);
		}

	RenderContext.Width = Width;
	RenderContext.Height = Height;

	// Begin
	MakeCurrent(RenderContext);

	if (m_SupportsFramebuffer)
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_SupportsMultisample ? RenderContext.fbMultisample : RenderContext.fbResolve);

	glViewport(0, 0, RenderContext.Width, RenderContext.Height);
}

void GLRenderer::EndRender(CFrontstageWnd* pFrontstageWnd, GLRenderContext& RenderContext, BOOL Themed) const
{
	ASSERT(pFrontstageWnd);
	ASSERT(RenderContext.hRC);

	if (m_SupportsFramebuffer)
	{
		if (m_SupportsMultisample)
		{
			// Blit multisampled framebuffer to resolving framebuffer
			glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, RenderContext.fbMultisample);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, RenderContext.fbResolve);
			glBlitFramebuffer(0, 0, RenderContext.Width, RenderContext.Height, 0, 0, RenderContext.Width, RenderContext.Height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

			glBindFramebuffer(GL_FRAMEBUFFER_EXT, RenderContext.fbResolve);
		}

		// Get bitmap
		BITMAP Bitmap;
		GetObject(RenderContext.hBitmap, sizeof(Bitmap), &Bitmap);

		glReadPixels(0, 0, RenderContext.Width, RenderContext.Height, GL_BGRA, GL_UNSIGNED_BYTE, Bitmap.bmBits);
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);

		// Use GDI to display bitmap
		CDC dc;
		dc.CreateCompatibleDC(RenderContext.pDC);

		HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(RenderContext.hBitmap);

		pFrontstageWnd->DrawWindowEdge(dc, Themed);

		RenderContext.pDC->BitBlt(0, 0, RenderContext.Width, RenderContext.Height, &dc, 0, 0, SRCCOPY);

		dc.SelectObject(hOldBitmap);
	}
	else
	{
		SwapBuffers(*RenderContext.pDC);
	}
}

void GLRenderer::DeleteRenderContext(GLRenderContext& RenderContext) const
{
	if (RenderContext.hRC)
	{
		MakeCurrent(RenderContext);

		if (m_SupportsFramebuffer)
		{
			glDeleteFramebuffers(1, &RenderContext.fbResolve);
			glDeleteRenderbuffers(1, &RenderContext.rbColorResolve);
			glDeleteRenderbuffers(1, &RenderContext.rbDepthResolve);

			if (m_SupportsMultisample)
			{
				glDeleteFramebuffers(1, &RenderContext.fbMultisample);
				glDeleteRenderbuffers(1, &RenderContext.rbColorMultisample);
				glDeleteRenderbuffers(1, &RenderContext.rbDepthMultisample);
			}
		}

		wglMakeCurrent(NULL, NULL);

		if (!m_SupportsMultisample || (strcmp(p_Vendor, "Intel")!=0))		// Crashes on Intel drivers when multisampling is used
			wglDeleteContext(RenderContext.hRC);
	}

	DeleteObject(RenderContext.hBitmap);

	delete RenderContext.pDC;
}

void GLRenderer::DrawIcon(GLfloat x, GLfloat y, GLfloat Size, GLfloat Alpha) const
{
	x -= 0.375;
	y -= 0.375;
	Size /= 2.0;

	glColor4f(1.0f, 1.0f, 1.0f, Alpha);

	glTexCoord2d(0.0f, 0.0f);
	glVertex2d(x-Size, y-Size);

	glTexCoord2d(1.0f, 0.0f);
	glVertex2d(x+Size, y-Size);

	glTexCoord2d(1.0f, 1.0f);
	glVertex2d(x+Size, y+Size);

	glTexCoord2d(0.0f, 1.0f);
	glVertex2d(x-Size, y+Size);
}
