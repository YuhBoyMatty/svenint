// SvenInt (c) Sw1ft
// Shaders code was taken from MetaHookSv (c) hzqst
// opengl.h

#ifndef SINT_OPENGL_MODULE_H
#define SINT_OPENGL_MODULE_H

#ifdef _WIN32
#pragma once
#endif

#include "GL/glew.h"

//-----------------------------------------------------------------------------
// Mini helpers to make fragment Shaders implementation a bit faster
//-----------------------------------------------------------------------------

#define POST_PROCESSING_DEFINE_VARS(name) GLuint name##FBO; GLuint name##Tex
#define POST_PROCESSING_RESET_VARS(name) name##FBO = 0; name##Tex = 0
#define POST_PROCESSING_FREE_VARS(name) if ( name##Tex ) glDeleteTextures(1, &(name##Tex)); \
	if ( name##FBO ) glDeleteFramebuffersEXT(1, &(name##FBO));
#define POST_PROCESSING_INIT_VARS_COLOR(name, w, h) glGenFramebuffersEXT(1, &(name##FBO)); name##Tex = GL_GenTextureRGB8(w, h);
#define POST_PROCESSING_INIT_VARS_DEPTH(name, w, h) glGenFramebuffersEXT(1, &(name##FBO)); name##Tex = GL_GenDepthTexture(w, h);

#define POST_PROCESSING_EXPAND_VARS(name) name##FBO, name##Tex
#define POST_PROCESSING_FBO(name) name##FBO
#define POST_PROCESSING_TEX(name) name##Tex

//-----------------------------------------------------------------------------
// Shader program macro definitions
//-----------------------------------------------------------------------------

#define SHADER_BEGIN_DESC(classname, name) \
	class classname; extern classname name; \
	class classname : public CShaderProgram \
	{ \
	public:
#define SHADER_BEGIN_DESC_MEMBER(classname) \
	class classname : public CShaderProgram \
	{ \
	public:
#define SHADER_DEFINE_INTERNAL_NAME(name) inline const char *GetInternalName() const override { return name; }
#define SHADER_DEFINE_UNIFORM(uniform) GLuint uniform = 0;
#define SHADER_END_DESC() }

#define SHADER_CREATE(classname, name) classname name

#define SHADER_BEGIN_COMPILE(shader, vertexCode, fragmentCode) if ( shader.Compile(vertexCode, fragmentCode) ) {
#define SHADER_BEGIN_COMPILE_DEF(shader, vertexCode, fragmentCode, vertexDefine, fragmentDefine) if ( shader.CompileEx(vertexCode, fragmentCode, vertexDefine, fragmentDefine) ) {
#define SHADER_BEGIN_COMPILE_FILE(shader, vertexFile, fragmentFile) if ( shader.CompileFile(vertexFile, fragmentFile) ) {
#define SHADER_BEGIN_COMPILE_FILE_DEF(shader, vertexFile, fragmentFile, vertexDefine, fragmentDefine) if ( shader.CompileFileEx(vertexFile, fragmentFile, vertexDefine, fragmentDefine) ) {
#define SHADER_LOCATE_UNIFORM(shader, uniform) shader.uniform = glGetUniformLocationARB(shader.m_program, #uniform);
#define SHADER_LOCATE_ATTRIBUTE(shader, attribute) shader.attribute = glGetAttribLocationARB(shader.m_program, #attribute);
#define SHADER_END_COMPILE() }

#define SHADER_COMPILED(shader) shader.Compiled()
#define SHADER_PROGRAM(shader) shader.GetProgram()
#define SHADER_UNIFORM_LOCATION(shader, uniform) glGetUniformLocationARB(shader.m_program, #uniform)
#define SHADER_ATTRIBUTE_LOCATION(shader, attribute) glGetAttribLocationARB(shader.m_program, #attribute)
#define SHADER_BIND(shader) shader.Bind()
#define SHADER_UNBIND() CShaderProgram::Unbind()

#define SHADER_UNIFORM_1i(shader, uniform, v0) glUniform1i(shader.uniform, v0)
#define SHADER_UNIFORM_2i(shader, uniform, v0, v1) glUniform2iARB(shader.uniform, v0, v1)
#define SHADER_UNIFORM_3i(shader, uniform, v0, v1, v2) glUniform3iARB(shader.uniform, v0, v1, v2)
#define SHADER_UNIFORM_4i(shader, uniform, v0, v1, v2, v3) glUniform4iARB(shader.uniform, v0, v1, v2, v3)
#define SHADER_UNIFORM_1f(shader, uniform, v0) glUniform1f(shader.uniform, v0)
#define SHADER_UNIFORM_2f(shader, uniform, v0, v1) glUniform2fARB(shader.uniform, v0, v1)
#define SHADER_UNIFORM_3f(shader, uniform, v0, v1, v2) glUniform3f(shader.uniform, v0, v1, v2)
#define SHADER_UNIFORM_4f(shader, uniform, v0, v1, v2, v3) glUniform4f(shader.uniform, v0, v1, v2, v3)
#define SHADER_VERTEX_ATTRIBUTE_3f(shader, index, x, y, z) glVertexAttrib3f(index, x, y, z)
#define SHADER_VERTEX_ATTRIBUTE_3fv(shader, index, v) glVertexAttrib3fv(index, v)
#define SHADER_MULTI_TEX_COORD_2f(shader, target, s, t) glMultiTexCoord2fARB(target, s, t)
#define SHADER_MULTI_TEX_COORD_3f(shader, target, s, t, r) glMultiTexCoord3fARB(target, s, t, r)

#define SHADER_FREE(shader) shader.Free()
#define SHADER_FREE_ALL() CShaderProgram::FreeShaders()

//-----------------------------------------------------------------------------
// Shader program abstraction layer
//-----------------------------------------------------------------------------

class CShaderProgram
{
public:
	CShaderProgram();

	virtual const char *GetInternalName( void ) const;

	bool Compile( const char *pszVertexCode, const char *pszFragmentCode, const char *pszVertexDefine = NULL, const char *pszFragmentDefine = NULL );
	bool CompileFile( const char *pszVertexFile, const char *pszFragmentFile, const char *pszVertexDefine = NULL, const char *pszFragmentDefine = NULL );

	inline bool Compiled( void ) const { return m_program != 0; }

	inline GLuint GetProgram( void ) const { return m_program; }
	inline GLuint GetUniformLocation( const char *pszUniformName ) const { return glGetUniformLocationARB( m_program, pszUniformName ); }
	inline GLuint GetAttributeLocation( const char *pszAttributeName ) const { return glGetAttribLocationARB( m_program, pszAttributeName ); }

	void Bind( void ) const;

	inline void Uniform1i( GLuint loc, int v0 ) const { glUniform1i( loc, v0 ); }
	inline void Uniform2i( GLuint loc, int v0, int v1 ) const { glUniform2iARB( loc, v0, v1 ); }
	inline void Uniform3i( GLuint loc, int v0, int v1, int v2 ) const { glUniform3iARB( loc, v0, v1, v2 ); }
	inline void Uniform4i( GLuint loc, int v0, int v1, int v2, int v3 ) const { glUniform4iARB( loc, v0, v1, v2, v3 ); }

	inline void Uniform1f( GLuint loc, float v0 ) const { glUniform1f( loc, v0 ); }
	inline void Uniform2f( GLuint loc, float v0, float v1 ) const { glUniform2fARB( loc, v0, v1 ); }
	inline void Uniform3f( GLuint loc, float v0, float v1, float v2 ) const { glUniform3f( loc, v0, v1, v2 ); }
	inline void Uniform4f( GLuint loc, float v0, float v1, float v2, float v3 ) const { glUniform4f( loc, v0, v1, v2, v3 ); }

	inline void VertexAttrib3f( GLuint index, float x, float y, float z ) const { glVertexAttrib3f( index, x, y, z ); }
	inline void VertexAttrib3fv( GLuint index, float *v ) const { glVertexAttrib3fv( index, v ); }

	inline void MultiTexCoord2f( GLenum target, float s, float t ) const { glMultiTexCoord2fARB( target, s, t ); }
	inline void MultiTexCoord3f( GLenum target, float s, float t, float r ) const { glMultiTexCoord3fARB( target, s, t, r ); }

	bool Free( void );

	static void Unbind( void );
	static void FreeShaders( void );

private:
	GLuint InternalCompile( const char *vscode, const char *fscode, const char *vsfile, const char *fsfile );
	GLuint CompileShaderObject( int type, const char *code, const char *filename );

	static void AppendInclude( std::string &str, const char *filename );
	static void AppendDefine( std::string &str, const std::string &def );

public:
	GLuint m_program;

private:
	class CGLShader
	{
	public:
		CGLShader( GLuint prog, GLuint *objs, int used )
		{
			program = prog;
			shader_objects.resize( used );
			memcpy( shader_objects.data(), objs, used * sizeof( GLuint ) );
		}

		GLuint program;
		std::vector<GLuint> shader_objects;
	};

	static std::vector<CGLShader> m_shaders;
	static GLuint m_currentProgram;
};

//-----------------------------------------------------------------------------
// Textures
//-----------------------------------------------------------------------------

GLuint GL_GenTexture( void );

void GL_UploadDepthStencilTexture( int texId, int w, int h );
GLuint GL_GenDepthStencilTexture( int w, int h );
GLuint GL_GenDepthTexture( int w, int h );

void GL_UploadTextureColorFormat( int texid, int w, int h, int iInternalFormat );
GLuint GL_GenTextureColorFormat( int w, int h, int iInternalFormat );
GLuint GL_GenTextureRGBA8( int w, int h );
GLuint GL_GenTextureRGB8( int w, int h );

void GL_BlitFrameBufferToFrameBufferColorOnly( GLuint src, GLuint dst, int w1, int h1, int w2, int h2 );

//-----------------------------------------------------------------------------
// OpenGL module
//-----------------------------------------------------------------------------

class COpenGLModule
{
public:
	COpenGLModule();

	bool			Init( void );
	void			Shutdown( void );
	inline bool		IsInitialized( void ) const { return m_bInitialized; }

public:
	void			OnShaderEnabled( void );

	void			DrawQuadPos( int x, int y, int w, int h );
	void			DrawQuad( int w, int h );
	void			InitColorTexPostProcessing( GLuint hFBO, GLuint hTex );
	void			InitDepthTexPostProcessing( GLuint hFBO, GLuint hTex );
	void			BindTexture( GLuint texture );

	inline int		GetScreenWidth( void ) const { return m_width; }
	inline int		GetScreenHeight( void ) const { return m_height; }
	
	inline float	GetScreenWidthf( void ) const { return m_fwidth; }
	inline float	GetScreenHeightf( void ) const { return m_fheight; }
	
	inline float	GetScreenAspectRatio( void ) const { return m_aspect; }

private:
	bool m_bInitialized;

	int m_width;
	int m_height;

	float m_fwidth;
	float m_fheight;

	float m_aspect;

	GLint m_hOldBuffer;
};

namespace Modules { extern COpenGLModule *opengl; }

#endif // SINT_OPENGL_MODULE_H
