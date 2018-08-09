
// GLRenderer.h: Hauptheaderdatei für die Anwendung
//

#pragma once
#include "CFrontstageWnd.h"
#include <GL/gl.h>
#include <GL/glu.h>


// GLRenderer
//

#define GL_TEXTURE_ENV_COLOR                          0x2201
#define GL_RESCALE_NORMAL                             0x803A
#define GL_MULTISAMPLE                                0x809D
#define GL_BGR                                        0x80E0
#define GL_BGRA                                       0x80E1
#define GL_GENERATE_MIPMAP_SGIS                       0x8191
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT               0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT              0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT              0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT              0x83F3
#define GL_TEXTURE0                                   0x84C0
#define GL_TEXTURE1                                   0x84C1
#define GL_TEXTURE2                                   0x84C2
#define GL_TEXTURE3                                   0x84C3
#define GL_MAX_TEXTURE_UNITS                          0x84E2
#define GL_SUBTRACT                                   0x84E7
#define GL_TEXTURE_MAX_ANISOTROPY_EXT                 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT             0x84FF
#define GL_COMBINE                                    0x8570
#define GL_COMBINE_RGB                                0x8571
#define GL_COMBINE_ALPHA                              0x8572
#define GL_RGB_SCALE                                  0x8573
#define GL_ADD_SIGNED                                 0x8574
#define GL_INTERPOLATE                                0x8575
#define GL_CONSTANT                                   0x8576
#define GL_PRIMARY_COLOR                              0x8577
#define GL_PREVIOUS                                   0x8578
#define GL_SOURCE0_RGB                                0x8580
#define GL_SOURCE1_RGB                                0x8581
#define GL_SOURCE2_RGB                                0x8582
#define GL_SOURCE3_RGB                                0x8583
#define GL_SOURCE0_ALPHA                              0x8588
#define GL_SOURCE1_ALPHA                              0x8589
#define GL_SOURCE2_ALPHA                              0x858A
#define GL_SOURCE3_ALPHA                              0x858B
#define GL_OPERAND0_RGB                               0x8590
#define GL_OPERAND1_RGB                               0x8591
#define GL_OPERAND2_RGB                               0x8592
#define GL_OPERAND3_RGB                               0x8593
#define GL_OPERAND0_ALPHA                             0x8598
#define GL_OPERAND1_ALPHA                             0x8599
#define GL_OPERAND2_ALPHA                             0x859A
#define GL_OPERAND3_ALPHA                             0x859B
#define GL_READ_FRAMEBUFFER_EXT                       0x8CA8
#define GL_DRAW_FRAMEBUFFER_EXT                       0x8CA9
#define GL_RENDERBUFFER_SAMPLES_EXT                   0x8CAB
#define GL_FRAMEBUFFER_COMPLETE_EXT                   0x8CD5
#define GL_COLOR_ATTACHMENT0_EXT                      0x8CE0
#define GL_COLOR_ATTACHMENT1_EXT                      0x8CE1
#define GL_COLOR_ATTACHMENT2_EXT                      0x8CE2
#define GL_COLOR_ATTACHMENT3_EXT                      0x8CE3
#define GL_COLOR_ATTACHMENT4_EXT                      0x8CE4
#define GL_COLOR_ATTACHMENT5_EXT                      0x8CE5
#define GL_COLOR_ATTACHMENT6_EXT                      0x8CE6
#define GL_COLOR_ATTACHMENT7_EXT                      0x8CE7
#define GL_DEPTH_ATTACHMENT_EXT                       0x8D00
#define GL_FRAMEBUFFER_EXT                            0x8D40
#define GL_RENDERBUFFER_EXT                           0x8D41
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT     0x8D56
#define GL_MAX_SAMPLES_EXT                            0x8D57

#define PI     3.14159265358979323846f

enum GLModelQuality
{
	MODELLOW=0,
	MODELMEDIUM=1,
	MODELHIGH=2,
	MODELULTRA=3
};

enum GLTextureQuality
{
	TEXTURELOW=0,
	TEXTUREMEDIUM=1,
	TEXTUREULTRA=2
};

typedef GLfloat GLcolor[4];

typedef void (APIENTRY* PFNGLACTIVETEXTUREPROC)(GLenum texture);
typedef void (APIENTRY* PFNGLMULTITEXCOORD2FPROC)(GLenum texture, GLfloat s, GLfloat t);
typedef void (APIENTRY* PFNGLDRAWBUFFERSPROC)(GLsizei n, const GLenum * bufs);
typedef void (APIENTRY* PFNGLGENRENDERBUFFERSPROC)(GLsizei n, GLuint* renderbuffers);
typedef void (APIENTRY* PFNGLBINDRENDERBUFFERPROC)(GLenum target, GLuint renderbuffer);
typedef void (APIENTRY* PFNGLRENDERBUFFERSTORAGEPROC)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRY* PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRY* PFNGLDELETERENDERBUFFERSPROC)(GLsizei n, const GLuint* renderbuffers);
typedef void (APIENTRY* PFNGLGENFRAMEBUFFERSPROC)(GLsizei n, GLuint* framebuffers);
typedef void (APIENTRY* PFNGLBINDFRAMEBUFFERPROC)(GLenum target, GLuint framebuffer);
typedef void (APIENTRY* PFNGLDELETEFRAMEBUFFERSPROC)(GLsizei n, const GLuint* framebuffers);
typedef void (APIENTRY* PFNGLFRAMEBUFFERRENDERBUFFERPROC)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef GLenum (APIENTRY* PFNGLCHECKFRAMEBUFFERSTATUSPROC)(GLenum target);
typedef void (APIENTRY* PFNGLBLITFRAMEBUFFERPROC)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

extern PFNGLACTIVETEXTUREPROC glActiveTexture;
extern PFNGLMULTITEXCOORD2FPROC glMultiTexCoord2f;
extern PFNGLDRAWBUFFERSPROC glDrawBuffers;
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
extern PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample;
extern PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
extern PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;

struct GLRenderContext
{
	CClientDC* pDC;
	HGLRC hRC;
	INT MaxWidth;
	INT MaxHeight;
	INT Width;
	INT Height;
	HBITMAP hBitmap;
	GLuint fbResolve;
	GLuint rbColorResolve;
	GLuint rbDepthResolve;
	GLuint fbMultisample;
	GLuint rbColorMultisample;
	GLuint rbDepthMultisample;
};

class GLRenderer sealed
{
public:
	GLRenderer();
	~GLRenderer();

	BOOL Initialize();

	// Basic info
	LPCSTR p_Extensions;
	LPCSTR p_Vendor;
	LPCSTR p_Version;

	// Hardware info
	GLint m_MaxTextureSize;
	GLint m_TextureUnits;

	// Extension info
	BOOL m_SupportsFilterAnisotropic;
	BOOL m_SupportsFramebufferMultisample;
	BOOL m_SupportsFramebufferObject;
	BOOL m_SupportsGenerateMipmap;
	BOOL m_SupportsRescaleNormal;
	BOOL m_SupportsTextureCombine;
	BOOL m_SupportsTextureCompression;

	GLint m_MaxSamples;
	GLfloat m_MaxAnisotropy;

	// Highlevel info
	BOOL m_SupportsFramebuffer;
	BOOL m_SupportsGlobeClouds;
	BOOL m_SupportsMultisample;

	GLModelQuality m_MaxModelQuality;
	GLTextureQuality m_MaxTextureQuality;

	static DOUBLE DegToRad(DOUBLE Deg);
	static void MatrixMultiplication4f(GLfloat Result[4][4], const GLfloat Left[4][4], const GLfloat Right[4][4]);
	static BOOL SetupPixelFormat(HDC hDC, DWORD dwFlags=0);
	static void ColorRef2GLColor(GLcolor& DstColor, COLORREF SrcColor, GLfloat Alpha=1.0f);
	static void SetColor(COLORREF SrcColor, GLfloat Alpha=1.0f);
	static void SetColor(const GLcolor& SrcColor);
	static void SetColor(const GLcolor& SrcColor, GLfloat Alpha);
	void EnableMultisample() const;
	void DisableMultisample() const;
	void EnableRescaleNormal() const;
	void DisableRescaleNormal() const;
	void Project2D() const;
	GLuint CreateGlobe(UINT cx=256) const;
	GLuint CreateTexture(HBITMAP hBitmap, BOOL ForceIntensity=FALSE, BOOL Repeat=TRUE, BOOL Mipmap=TRUE, BOOL Compress=TRUE) const;
	GLuint CreateTexture(Bitmap* pBitmap, BOOL ForceIntensity=FALSE, BOOL Repeat=TRUE, BOOL Mipmap=TRUE, BOOL Compress=TRUE) const;
	void CreateTextureBlueMarble(GLuint& nTextureID, BOOL Mipmap=TRUE);
	void CreateTextureClouds(GLuint& nTextureID, BOOL Mipmap=TRUE);
	void CreateTextureLocationIndicator(GLuint& nTextureID) const;
	BOOL CreateRenderContext(CWnd* pWnd, GLRenderContext& RenderContext) const;
	void BeginRender(CWnd* pWnd, GLRenderContext& RenderContext) const;
	static void MakeCurrent(GLRenderContext& RenderContext);
	void EndRender(CFrontstageWnd* pFrontstageWnd, GLRenderContext& RenderContext, BOOL Themed) const;
	void DeleteRenderContext(GLRenderContext& RenderContext) const;
	void DrawIcon(GLfloat x, GLfloat y, GLfloat Size, GLfloat Alpha) const;

protected:
	static void Vertex5f(GLfloat x, GLfloat y, GLfloat z, GLfloat s, GLfloat t);
	static void Vertex5f(GLfloat pVertex[], GLfloat s, GLfloat t);
	GLuint CreateTexture(UINT Width, UINT Height, UINT BPP, void* pData, BOOL ForceIntensity=FALSE, BOOL Repeat=TRUE, BOOL Mipmap=TRUE, BOOL Compress=FALSE) const;
	void CreateTexture(UINT nResID, UINT& nPictureID, IPicture*& pPicture, GLuint& nTextureID, BOOL ForceIntensity=FALSE, BOOL Repeat=TRUE, BOOL Mipmap=TRUE, BOOL Compress=TRUE) const;

private:
	BOOL m_Initialized;

	UINT m_nPictureBlueMarble;
	IPicture* m_pPictureBlueMarble;
	BOOL m_BlueMarbleCompressed;

	UINT m_nPictureClouds;
	IPicture* m_pPictureClouds;
};

extern GLRenderer theRenderer;


inline DOUBLE GLRenderer::DegToRad(DOUBLE Deg)
{
	return Deg*PI/180.0;
}

inline void GLRenderer::SetColor(const GLcolor& SrcColor)
{
	glColor4f(SrcColor[0], SrcColor[1], SrcColor[2], SrcColor[3]);
}

inline void GLRenderer::SetColor(const GLcolor& SrcColor, GLfloat Alpha)
{
	glColor4f(SrcColor[0], SrcColor[1], SrcColor[2], Alpha);
}

inline void GLRenderer::EnableMultisample() const
{
	if (m_SupportsMultisample)
		glEnable(GL_MULTISAMPLE);
}

inline void GLRenderer::DisableMultisample() const
{
	if (m_SupportsMultisample)
		glDisable(GL_MULTISAMPLE);
}

inline void GLRenderer::EnableRescaleNormal() const
{
	glEnable(m_SupportsRescaleNormal ? GL_RESCALE_NORMAL : GL_NORMALIZE);
}

inline void GLRenderer::DisableRescaleNormal() const
{
	glDisable(m_SupportsRescaleNormal ? GL_RESCALE_NORMAL : GL_NORMALIZE);
}

inline void GLRenderer::Vertex5f(GLfloat pVertex[], GLfloat s, GLfloat t)
{
	ASSERT(pVertex);

	Vertex5f(pVertex[0], pVertex[1], pVertex[2], s, t);
}

inline void GLRenderer::CreateTextureClouds(GLuint& nTextureID, BOOL Mipmap)
{
	CreateTexture(IDB_CLOUDS_1024, m_nPictureClouds, m_pPictureClouds, nTextureID, TRUE, TRUE, Mipmap, FALSE);
}

inline void GLRenderer::MakeCurrent(GLRenderContext& RenderContext)
{
	wglMakeCurrent(*RenderContext.pDC, RenderContext.hRC);
}
