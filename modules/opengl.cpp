// SvenInt (c) Sw1ft
// opengl.cpp

#include "stdafx.h"
#include "opengl.h"

#include <regex>

namespace Modules { static COpenGLModule openglModule; COpenGLModule *opengl = &openglModule; }

#ifndef SINT_USE_GLEW
GLFunctions glfuncs = { 0 };
#endif

//-----------------------------------------------------------------------------
// Textures
//-----------------------------------------------------------------------------

GLuint GL_GenTexture( void )
{
	GLuint tex;
	glGenTextures( 1, &tex );
	return tex;
}

void GL_UploadDepthStencilTexture( int texId, int w, int h )
{
	glBindTexture( GL_TEXTURE_2D, texId );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexStorage2D( GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, w, h );
	glBindTexture( GL_TEXTURE_2D, 0 );
}

GLuint GL_GenDepthStencilTexture( int w, int h )
{
	GLuint texid = GL_GenTexture();
	GL_UploadDepthStencilTexture( texid, w, h );
	return texid;
}

GLuint GL_GenDepthTexture( int w, int h )
{
	GLuint texid = GL_GenTexture();

	glBindTexture( GL_TEXTURE_2D, texid );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL );
	glBindTexture( GL_TEXTURE_2D, 0 );

	return texid;
}

void GL_UploadTextureColorFormat( int texid, int w, int h, int iInternalFormat )
{
	glBindTexture( GL_TEXTURE_2D, texid );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	//glTexStorage2D doesnt work with qglCopyTexImage2D so we use glTexImage2D here
	glTexImage2D( GL_TEXTURE_2D, 0, iInternalFormat, w, h, 0, GL_RGBA, ( iInternalFormat != GL_RGBA && iInternalFormat != GL_RGBA8 ) ? GL_FLOAT : GL_UNSIGNED_BYTE, 0 );

	glBindTexture( GL_TEXTURE_2D, 0 );
}

GLuint GL_GenTextureColorFormat( int w, int h, int iInternalFormat )
{
	GLuint texid = GL_GenTexture();
	GL_UploadTextureColorFormat( texid, w, h, iInternalFormat );
	return texid;
}

GLuint GL_GenTextureRGBA8( int w, int h )
{
	return GL_GenTextureColorFormat( w, h, GL_RGBA8 );
}

GLuint GL_GenTextureRGB8( int w, int h )
{
	return GL_GenTextureColorFormat( w, h, GL_RGB8 );
}

void GL_BlitFrameBufferToFrameBufferColorOnly( GLuint src, GLuint dst, int w1, int h1, int w2, int h2 )
{
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, dst );
	glBindFramebuffer( GL_READ_FRAMEBUFFER, src );
	glBlitFramebuffer( 0, 0, w1, h1, 0, 0, w2, h2, GL_COLOR_BUFFER_BIT, GL_LINEAR );
}

//-----------------------------------------------------------------------------
// Shader program abstraction layer
//-----------------------------------------------------------------------------

GLuint CShaderProgram::m_currentProgram = -1;
std::vector<CShaderProgram::CGLShader> CShaderProgram::m_shaders;

CShaderProgram::CShaderProgram() : m_program( 0 )
{
}

const char *CShaderProgram::GetInternalName() const
{
	static const char *empty_string = "";

	return empty_string;
}

bool CShaderProgram::Compile( const char *pszVertexCode, const char *pszFragmentCode, const char *pszVertexDefine /* = NULL */, const char *pszFragmentDefine /* = NULL */ )
{
	std::string vs = std::string( pszVertexCode );

	AppendDefine( vs, "#define IS_VERTEX_SHADER\n" );

	if ( pszVertexDefine )
	{
		AppendDefine( vs, pszVertexDefine );
	}

	std::string fs = std::string( pszFragmentCode );

	AppendDefine( fs, "#define IS_FRAGMENT_SHADER\n" );

	if ( pszFragmentDefine )
	{
		AppendDefine( fs, pszFragmentDefine );
	}

	// Not supported
/*
	if ( vs.find("#include") != std::string::npos )
	{
		AppendInclude(vs, pszVertexFile);
	}

	if ( fs.find("#include") != std::string::npos )
	{
		AppendInclude(fs, pszFragmentFile);
	}
*/

	return ( m_program = InternalCompile( vs.c_str(), fs.c_str(), NULL, NULL ) ) != 0;
}

bool CShaderProgram::CompileFile( const char *pszVertexFile, const char *pszFragmentFile, const char *pszVertexDefine /* = NULL */, const char *pszFragmentDefine /* = NULL */ )
{
	auto vscode = (char *)Globals::cl_enginefuncs->COM_LoadFile( (char *)pszVertexFile, 5, 0 );

	std::string vs;

	if ( !vscode )
	{
		Warning( "[SvenInt] CShaderProgram::CompileFile: \"%s\" not found\n", pszVertexFile );
		return false;
	}
	else
	{
		vs = std::string( vscode );
	}

	AppendDefine( vs, "#define IS_VERTEX_SHADER\n" );

	if ( pszVertexDefine )
	{
		AppendDefine( vs, pszVertexDefine );
	}

	Globals::cl_enginefuncs->COM_FreeFile( vscode );

	auto fscode = (char *)Globals::cl_enginefuncs->COM_LoadFile( (char *)pszFragmentFile, 5, 0 );

	std::string fs;

	if ( !fscode )
	{
		Warning( "[SvenInt] CShaderProgram::CompileFile: \"%s\" not found\n", pszFragmentFile );
		return false;
	}
	else
	{
		fs = std::string( fscode );
	}

	AppendDefine( fs, "#define IS_FRAGMENT_SHADER\n" );

	if ( pszFragmentDefine )
	{
		AppendDefine( fs, pszFragmentDefine );
	}

	Globals::cl_enginefuncs->COM_FreeFile( fscode );

	if ( vs.find( "#include" ) != std::string::npos )
	{
		AppendInclude( vs, pszVertexFile );
	}

	if ( fs.find( "#include" ) != std::string::npos )
	{
		AppendInclude( fs, pszFragmentFile );
	}

	return ( m_program = InternalCompile( vs.c_str(), fs.c_str(), pszVertexFile, pszFragmentFile ) ) != 0;
}

void CShaderProgram::Bind( void ) const
{
	if ( m_currentProgram != m_program )
	{
		m_currentProgram = m_program;
		glUseProgramObjectARB( m_program );
	}
}

void CShaderProgram::Unbind( void )
{
	if ( m_currentProgram != 0 )
	{
		m_currentProgram = 0;
		glUseProgramObjectARB( 0 );
	}
}

bool CShaderProgram::Free( void )
{
	if ( !Compiled() )
		return false;

	for ( size_t i = 0; i < m_shaders.size(); ++i )
	{
		if ( m_shaders[ i ].program == m_program )
		{
			auto &objs = m_shaders[ i ].shader_objects;

			for ( size_t j = 0; j < objs.size(); ++j )
			{
				glDetachObjectARB( m_program, objs[ j ] );
				glDeleteObjectARB( objs[ j ] );
			}

			glDeleteProgramsARB( 1, &m_program );

			m_program = 0;
			m_shaders.erase( m_shaders.begin() + i );
			return true;
		}
	}

	return false;
}

void CShaderProgram::FreeShaders( void )
{
	for ( size_t i = 0; i < m_shaders.size(); ++i )
	{
		auto &objs = m_shaders[ i ].shader_objects;

		for ( size_t j = 0; j < objs.size(); ++j )
		{
			glDetachObjectARB( m_shaders[ i ].program, objs[ j ] );
			glDeleteObjectARB( objs[ j ] );
		}

		glDeleteProgramsARB( 1, &m_shaders[ i ].program );
	}

	m_shaders.clear();
}

GLuint CShaderProgram::InternalCompile( const char *vscode, const char *fscode, const char *vsfile, const char *fsfile )
{
	GLuint shader_objects[ 32 ];
	int shader_object_used = 0;

	shader_objects[ shader_object_used ] = CompileShaderObject( GL_VERTEX_SHADER_ARB, vscode, vsfile );
	shader_object_used++;

	shader_objects[ shader_object_used ] = CompileShaderObject( GL_FRAGMENT_SHADER_ARB, fscode, fsfile );
	shader_object_used++;

	GLuint program = glCreateProgramObjectARB();
	for ( int i = 0; i < shader_object_used; ++i )
		glAttachObjectARB( program, shader_objects[ i ] );
	glLinkProgramARB( program );

	int iStatus;
	glGetProgramiv( program, GL_LINK_STATUS, &iStatus );

	if ( !iStatus )
	{
		int nInfoLength;
		char szCompilerLog[ 1024 ] = { 0 };
		glGetProgramInfoLog( program, sizeof( szCompilerLog ), &nInfoLength, szCompilerLog );

		Warning( "[SvenInt] Shader \"%s\" compiled with error:\n%s\n", GetInternalName(), szCompilerLog );

		for ( int j = 0; j < shader_object_used; ++j )
		{
			glDetachObjectARB( program, shader_objects[ j ] );
			glDeleteObjectARB( shader_objects[ j ] );
		}
		glDeleteProgramsARB( 1, &program );
		return 0;
	}

	m_shaders.emplace_back( program, shader_objects, shader_object_used );

	return program;
}

GLuint CShaderProgram::CompileShaderObject( int type, const char *code, const char *filename )
{
	auto obj = glCreateShaderObjectARB( type );

	glShaderSource( obj, 1, &code, NULL );

	glCompileShader( obj );

	// Check for errors
	int iStatus;
	glGetShaderiv( obj, GL_COMPILE_STATUS, &iStatus );

	if ( !iStatus )
	{
		int nInfoLength;
		char szCompilerLog[ 1024 ] = { 0 };
		glGetInfoLogARB( obj, sizeof( szCompilerLog ) - 1, &nInfoLength, szCompilerLog );
		szCompilerLog[ nInfoLength ] = 0;

		Warning( "[SvenInt] %s shader \"%s\" compiled with error:\n%s\n", type == GL_VERTEX_SHADER_ARB ? "Vertex" : "Fragment", GetInternalName(), szCompilerLog );
	}

	return obj;
}

void CShaderProgram::AppendInclude( std::string &str, const char *filename )
{
	std::regex pattern( "#include[< \"]+([a-zA-Z_\\.]+)[> \"]" );
	std::smatch result;
	std::regex_search( str, result, pattern );

	std::string skipped;

	std::string::const_iterator searchStart( str.cbegin() );

	while ( std::regex_search( searchStart, str.cend(), result, pattern ) && result.size() >= 2 )
	{
		std::string prefix = result.prefix();
		std::string suffix = result.suffix();

		auto includeFileName = result[ 1 ].str();

		char slash = 0;

		std::string includePath = filename;

		for ( size_t j = includePath.length() - 1; j > 0; --j )
		{
			if ( includePath[ j ] == '\\' || includePath[ j ] == '/' )
			{
				slash = includePath[ j ];
				includePath.resize( j );

				break;
			}
		}

		includePath += slash;
		includePath += includeFileName;

		auto pFile = Globals::cl_enginefuncs->COM_LoadFile( (char *)includePath.c_str(), 5, NULL );

		if ( pFile )
		{
			std::string wbinding( (char *)pFile );

			Globals::cl_enginefuncs->COM_FreeFile( pFile );

			if ( searchStart != str.cbegin() )
			{
				str = skipped + prefix;
			}
			else
			{
				str = prefix;
			}

			str += wbinding;

			auto currentLength = str.length();

			str += suffix;

			skipped = str.substr( 0, currentLength );
			searchStart = str.cbegin() + currentLength;

			continue;
		}

		searchStart = result.suffix().first;
	}
}

void CShaderProgram::AppendDefine( std::string &str, const std::string &def )
{
	std::regex pattern( "(#version [0-9a-z ]+)" );
	std::smatch result;
	std::regex_search( str, result, pattern );

	if ( result.size() >= 1 )
	{
		std::string prefix = result[ 0 ];
		std::string suffix = result.suffix();

		str = prefix;
		str += "\n\n";
		str += def;
		str += "\n\n";
		str += suffix;
	}
	else
	{
		std::string suffix = str;

		str = def;
		str += "\n\n";
		str += suffix;
	}
}

//-----------------------------------------------------------------------------
// Reorganize sequence of calls for shaders
//-----------------------------------------------------------------------------

#include "features/shader_depth_buffer.h"
#include "features/shader_ssao.h"
#include "features/shader_color_correction.h"
#include "features/shader_bloom.h"
#include "features/shader_chromatic_aberration.h"
#include "features/shader_dof_blur.h"
#include "features/shader_motion_blur.h"
#include "features/shader_vignette.h"
#include "features/shader_menu_background_blur.h"

void COpenGLModule::OnShaderEnabled( void )
{
	HooksCallChain nonShaders;

	IHookEventListener *pDepthBuffer = NULL;
	IHookEventListener *pSSAO = NULL;
	IHookEventListener *pColorCorrection = NULL;
	IHookEventListener *pBloom = NULL;
	IHookEventListener *pChromaticAberration = NULL;
	IHookEventListener *pDoFBlur = NULL;
	IHookEventListener *pMotionBlur = NULL;
	IHookEventListener *pVignette = NULL;
	IHookEventListener *pMenuBgBlur = NULL;

	HooksCallChain &hooks = const_cast<HooksCallChain &>( Globals::hookevents->GetCallChain( kHUD_Redraw_HookEvent, kHookCall ) );
	for ( IHookEventListener *pListener : hooks )
	{
		if ( pListener == (IHookEventListener *)Features::depthbuffer )
			pDepthBuffer = (IHookEventListener *)Features::depthbuffer;
		else if ( pListener == (IHookEventListener *)Features::ssao )
			pSSAO = (IHookEventListener *)Features::ssao;
		else if ( pListener == (IHookEventListener *)Features::colorcorrection )
			pColorCorrection = (IHookEventListener *)Features::colorcorrection;
		else if ( pListener == (IHookEventListener *)Features::bloom )
			pBloom = (IHookEventListener *)Features::bloom;
		else if ( pListener == (IHookEventListener *)Features::chromaticaberration )
			pChromaticAberration = (IHookEventListener *)Features::chromaticaberration;
		else if ( pListener == (IHookEventListener *)Features::dofblur )
			pDoFBlur = (IHookEventListener *)Features::dofblur;
		else if ( pListener == (IHookEventListener *)Features::motionblur )
			pMotionBlur = (IHookEventListener *)Features::motionblur;
		else if ( pListener == (IHookEventListener *)Features::vignette )
			pVignette = (IHookEventListener *)Features::vignette;
		else if ( pListener == (IHookEventListener *)Features::menubgblur )
			pMenuBgBlur = (IHookEventListener *)Features::menubgblur;
		else
			nonShaders.push_back( pListener );
	}

	hooks.clear();

	if ( pDepthBuffer != NULL )
		hooks.push_back( pDepthBuffer );
	if ( pSSAO != NULL )
		hooks.push_back( pSSAO );
	if ( pColorCorrection != NULL )
		hooks.push_back( pColorCorrection );
	if ( pBloom != NULL )
		hooks.push_back( pBloom );
	if ( pChromaticAberration != NULL )
		hooks.push_back( pChromaticAberration );
	if ( pDoFBlur != NULL )
		hooks.push_back( pDoFBlur );
	if ( pMotionBlur != NULL )
		hooks.push_back( pMotionBlur );
	if ( pVignette != NULL )
		hooks.push_back( pVignette );
	if ( pMenuBgBlur != NULL )
		hooks.push_back( pMenuBgBlur );

	hooks.insert( hooks.end(), nonShaders.begin(), nonShaders.end() );
}

//-----------------------------------------------------------------------------
// Utility draw functions
//-----------------------------------------------------------------------------

void COpenGLModule::DrawQuadPos( int x, int y, int w, int h )
{
	glBegin( GL_QUADS );
		glTexCoord2i( 0, 0 );
		glVertex3i( x, y + h, -1 );
		glTexCoord2i( 0, 1 );
		glVertex3i( x, y, -1 );
		glTexCoord2i( 1, 1 );
		glVertex3i( x + w, y, -1 );
		glTexCoord2i( 1, 0 );
		glVertex3i( x + w, y + h, -1 );
	glEnd();
}

void COpenGLModule::DrawQuad( int w, int h )
{
	DrawQuadPos( 0, 0, w, h );
}

//-----------------------------------------------------------------------------
// Bind framebuffer and texture to prepare post processing
//-----------------------------------------------------------------------------

void COpenGLModule::InitColorTexPostProcessing( GLuint hFBO, GLuint hTex )
{
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &m_hOldBuffer );

		glBindFramebuffer( GL_FRAMEBUFFER, hFBO );

		glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, hTex, 0 );

		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, hFBO );
		glBindFramebuffer( GL_READ_FRAMEBUFFER, m_hOldBuffer );

		glBlitFramebuffer( 0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR );

	glBindFramebuffer( GL_FRAMEBUFFER, m_hOldBuffer );
}

void COpenGLModule::InitDepthTexPostProcessing( GLuint hFBO, GLuint hTex )
{
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &m_hOldBuffer );

		glBindFramebuffer( GL_FRAMEBUFFER, hFBO );

		glFramebufferTexture( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, hTex, 0 );

		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, hFBO );
		glBindFramebuffer( GL_READ_FRAMEBUFFER, m_hOldBuffer );

		glBlitFramebuffer( 0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST );

	glBindFramebuffer( GL_FRAMEBUFFER, m_hOldBuffer );
}

//-----------------------------------------------------------------------------
// BindTexture
//-----------------------------------------------------------------------------

void COpenGLModule::BindTexture( GLuint texture )
{
	if ( GameData::Pointers::Engine::GL_Bind == NULL )
		return;

	function_cast<void ( CALLCONV_CDECL * )( GLuint )>( GameData::Pointers::Engine::GL_Bind )( texture );
}

//-----------------------------------------------------------------------------
// Module constructor
//-----------------------------------------------------------------------------

COpenGLModule::COpenGLModule()
{
	m_width = 0;
	m_height = 0;

	m_fwidth = 0.f;
	m_fheight = 0.f;

	m_aspect = 0.f;

	m_bInitialized = false;
	m_hOldBuffer = 0;
}

//-----------------------------------------------------------------------------
// Init GL funcs
//-----------------------------------------------------------------------------

#ifndef SINT_USE_GLEW
#ifdef LINUX
#include <GL/glx.h>
#endif

bool COpenGLModule::InitGlFuncs( GLFunctions *gl )
{
	bool bOK = true;

#ifdef WIN32
	auto pfnglGetProcAddress = wglGetProcAddress;
#else
	auto pfnglGetProcAddress = glXGetProcAddressARB;
#endif

	GL_LOAD_FUNC( glGenFramebuffersEXT, gl->glGenFramebuffersEXT );
	GL_LOAD_FUNC( glDeleteFramebuffersEXT, gl->glDeleteFramebuffersEXT );
	GL_LOAD_FUNC( glDeleteTextures, gl->glDeleteTextures );
	GL_LOAD_FUNC( glGetUniformLocationARB, gl->glGetUniformLocationARB );
	GL_LOAD_FUNC( glGetAttribLocationARB, gl->glGetAttribLocationARB );
	GL_LOAD_FUNC( glUniform1i, gl->glUniform1i );
	GL_LOAD_FUNC( glUniform2iARB, gl->glUniform2iARB );
	GL_LOAD_FUNC( glUniform3iARB, gl->glUniform3iARB );
	GL_LOAD_FUNC( glUniform4iARB, gl->glUniform4iARB );
	GL_LOAD_FUNC( glUniform1f, gl->glUniform1f );
	GL_LOAD_FUNC( glUniform2fARB, gl->glUniform2fARB );
	GL_LOAD_FUNC( glUniform3f, gl->glUniform3f );
	GL_LOAD_FUNC( glUniform4f, gl->glUniform4f );
	GL_LOAD_FUNC( glVertexAttrib3f, gl->glVertexAttrib3f );
	GL_LOAD_FUNC( glVertexAttrib3fv, gl->glVertexAttrib3fv );
	GL_LOAD_FUNC( glMultiTexCoord2fARB, gl->glMultiTexCoord2fARB );
	GL_LOAD_FUNC( glMultiTexCoord3fARB, gl->glMultiTexCoord3fARB );
	GL_LOAD_FUNC( glBindTexture, gl->glBindTexture );
	GL_LOAD_FUNC( glTexStorage2D, gl->glTexStorage2D );
	GL_LOAD_FUNC( glBindFramebuffer, gl->glBindFramebuffer );
	GL_LOAD_FUNC( glBlitFramebuffer, gl->glBlitFramebuffer );
	GL_LOAD_FUNC( glFramebufferTexture, gl->glFramebufferTexture );
	GL_LOAD_FUNC( glUseProgramObjectARB, gl->glUseProgramObjectARB );
	GL_LOAD_FUNC( glDetachObjectARB, gl->glDetachObjectARB );
	GL_LOAD_FUNC( glDeleteObjectARB, gl->glDeleteObjectARB );
	GL_LOAD_FUNC( glDeleteProgramsARB, gl->glDeleteProgramsARB );
	GL_LOAD_FUNC( glGetProgramInfoLog, gl->glGetProgramInfoLog );
	GL_LOAD_FUNC( glCreateShaderObjectARB, gl->glCreateShaderObjectARB );
	GL_LOAD_FUNC( glCreateProgramObjectARB, gl->glCreateProgramObjectARB );
	GL_LOAD_FUNC( glAttachObjectARB, gl->glAttachObjectARB );
	GL_LOAD_FUNC( glLinkProgramARB, gl->glLinkProgramARB );
	GL_LOAD_FUNC( glCompileShader, gl->glCompileShader );
	GL_LOAD_FUNC( glShaderSource, gl->glShaderSource );
	GL_LOAD_FUNC( glGetProgramiv, gl->glGetProgramiv );
	GL_LOAD_FUNC( glGetShaderiv, gl->glGetShaderiv );
	GL_LOAD_FUNC( glGetInfoLogARB, gl->glGetInfoLogARB );
	GL_LOAD_FUNC( glActiveTexture, gl->glActiveTexture );

	return bOK;
}
#endif

//-----------------------------------------------------------------------------
// Init module
//-----------------------------------------------------------------------------

bool COpenGLModule::Init( void )
{
#ifdef SINT_USE_GLEW
	// Init glew
	GLenum status = glewInit();

	if ( status != GLEW_OK )
	{
		Warning( "[SvenInt] GLEW initialization failure (Reason: %s). Shader features are not available\n", glewGetErrorString( status ) );
		return false;
	}

	// Print GL version
	const GLubyte *renderer = glGetString( GL_RENDERER );
	const GLubyte *vendor = glGetString( GL_VENDOR );
	const GLubyte *version = glGetString( GL_VERSION );
	const GLubyte *glslVersion = glGetString( GL_SHADING_LANGUAGE_VERSION );

	GLint major, minor;
	glGetIntegerv( GL_MAJOR_VERSION, &major );
	glGetIntegerv( GL_MINOR_VERSION, &minor );

	DevMsg( "GL Vendor            : %s\n", vendor );
	DevMsg( "GL Renderer          : %s\n", renderer );
	DevMsg( "GL Version (string)  : %s\n", version );
	DevMsg( "GL Version (integer) : %d.%d\n", major, minor );
	DevMsg( "GLSL Version         : %s\n", glslVersion );
#else
	if ( !InitGlFuncs( &glfuncs ) )
	{
		Warning( "[SvenInt] GL initialization failure. Shader features are not available\n" );
		return false;
	}
#endif

	m_bInitialized = true;

	m_width = Globals::gameutils->GetScreenWidth();
	m_height = Globals::gameutils->GetScreenHeight();

	m_fwidth = (float)m_width;
	m_fheight = (float)m_height;

	m_aspect = m_fwidth / m_fheight;
	return true;
}

//-----------------------------------------------------------------------------
// Shutdown module
//-----------------------------------------------------------------------------

void COpenGLModule::Shutdown( void )
{
	SHADER_FREE_ALL();
}
