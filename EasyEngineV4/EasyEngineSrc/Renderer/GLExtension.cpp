#define EXTENSION_CPP

#include "glextension.h"


PFNGLCREATESHADERPROC glCreateShader = NULL;
PFNGLDELETESHADERPROC glDeleteShader = NULL;
PFNGLISSHADERPROC glIsShader = NULL;
PFNGLSHADERSOURCEPROC glShaderSource = NULL;
PFNGLCOMPILESHADERARBPROC glCompileShader = NULL;
PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgram = NULL;
PFNGLATTACHSHADERPROC glAttachShader = NULL;
PFNGLDELETEPROGRAMPROC glDeleteProgram = NULL;
PFNGLUSEPROGRAMPROC glUseProgram = NULL;
PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
PFNGLGETPROGRAMIVARBPROC glGetProgramiv = NULL;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
PFNGLUNIFORM1IVPROC glUniform1iv = NULL;
PFNGLUNIFORM2FVPROC glUniform2fv = NULL;
PFNGLUNIFORM3FVPROC glUniform3fv = NULL;
PFNGLUNIFORM4FVPROC glUniform4fv = NULL;
PFNGLUNIFORM4IVPROC glUniform4iv = NULL;
PFNGLUNIFORM3FPROC glUniform3f = NULL;
PFNGLUNIFORM4FPROC glUniform4f = NULL;
PFNGLUNIFORM1IPROC glUniform1i = NULL;
PFNGLUNIFORM1FPROC glUniform1f = NULL;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = NULL;
PFNGLUNIFORM1FVPROC glUniform1fv = NULL;
PFNGLGENBUFFERSARBPROC glGenBuffers = NULL;
PFNGLBINDBUFFERARBPROC glBindBuffer = NULL;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffers = NULL;	
PFNGLBUFFERDATAARBPROC glBufferData = NULL;
PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameteriv = NULL;
PFNGLGETBUFFERSUBDATAARBPROC glGetBufferSubData = NULL;
PFNGLBUFFERSUBDATAARBPROC glBufferSubData = NULL;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap = NULL;
PFNGLDRAWARRAYSINSTANCEDEXTPROC glDrawArraysInstanced = NULL;
PFNGLDRAWELEMENTSINSTANCEDEXTPROC glDrawElementsInstanced = NULL;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = NULL;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = NULL;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers = NULL;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer = NULL;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = NULL;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer = NULL;
PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture = NULL;
PFNGLDRAWBUFFERSPROC glDrawBuffers = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = NULL;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = NULL;

void InitExtensions()
{
	glGenBuffers = reinterpret_cast<PFNGLGENBUFFERSARBPROC> (wglGetProcAddress("glGenBuffersARB"));
	glBindBuffer = reinterpret_cast<PFNGLBINDBUFFERARBPROC> (wglGetProcAddress("glBindBufferARB"));
	glDeleteBuffers = reinterpret_cast<PFNGLDELETEBUFFERSARBPROC> (wglGetProcAddress("glDeleteBuffersARB"));
	glBufferData = reinterpret_cast<PFNGLBUFFERDATAARBPROC> (wglGetProcAddress("glBufferDataARB"));		
	glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVARBPROC)wglGetProcAddress("glGetBufferParameterivARB");
	glBufferSubData = (PFNGLBUFFERSUBDATAARBPROC)wglGetProcAddress("glBufferSubDataARB");
	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
	glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDEXTPROC)wglGetProcAddress("glDrawArraysInstanced");
	glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDEXTPROC)wglGetProcAddress("glDrawElementsInstanced");
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
	glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)wglGetProcAddress("glGenRenderbuffers");
	glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)wglGetProcAddress("glBindRenderbuffer");
	glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)wglGetProcAddress("glRenderbufferStorage");
	glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress("glFramebufferRenderbuffer");
	glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC)wglGetProcAddress("glFramebufferTexture");
	glDrawBuffers = (PFNGLDRAWBUFFERSPROC)wglGetProcAddress("glDrawBuffers");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D");

	// shaders
	glCreateShader	= reinterpret_cast< PFNGLCREATESHADERPROC >				( wglGetProcAddress( "glCreateShader" ));
	glDeleteShader	= reinterpret_cast< PFNGLDELETESHADERPROC >				( wglGetProcAddress( "glDeleteShader" ));
	glIsShader		= reinterpret_cast< PFNGLISSHADERPROC >					( wglGetProcAddress( "glIsShader" ));
	glShaderSource	= reinterpret_cast< PFNGLSHADERSOURCEPROC >				( wglGetProcAddress( "glShaderSource" ));
	glCompileShader	= reinterpret_cast< PFNGLCOMPILESHADERARBPROC >			( wglGetProcAddress( "glCompileShader" ));
	glCreateProgram	= reinterpret_cast< PFNGLCREATEPROGRAMOBJECTARBPROC >	( wglGetProcAddress( "glCreateProgram" ) );
	glAttachShader	= reinterpret_cast< PFNGLATTACHSHADERPROC >				( wglGetProcAddress( "glAttachShader" ) );
	glDeleteProgram	= reinterpret_cast< PFNGLDELETEPROGRAMPROC >			( wglGetProcAddress( "glDeleteProgram" ) );	
	glUseProgram		= reinterpret_cast< PFNGLUSEPROGRAMPROC >			( wglGetProcAddress( "glUseProgram" ) );
	glLinkProgram		= reinterpret_cast< PFNGLLINKPROGRAMPROC >			( wglGetProcAddress( "glLinkProgram" ) );
	glGetShaderiv		= reinterpret_cast< PFNGLGETSHADERIVPROC >			( wglGetProcAddress( "glGetShaderiv" ) );
	glGetShaderInfoLog = reinterpret_cast< PFNGLGETSHADERINFOLOGPROC >		( wglGetProcAddress( "glGetShaderInfoLog" ) );
	glGetProgramiv	= reinterpret_cast< PFNGLGETPROGRAMIVARBPROC >			( wglGetProcAddress( "glGetProgramiv" ) );
	glGetProgramInfoLog = reinterpret_cast< PFNGLGETPROGRAMINFOLOGPROC >	( wglGetProcAddress( "glGetProgramInfoLog" ) );
	glGetUniformLocation = reinterpret_cast< PFNGLGETUNIFORMLOCATIONPROC >	( wglGetProcAddress( "glGetUniformLocation" ) );
	glUniform1iv = reinterpret_cast<PFNGLUNIFORM1IVPROC>					( wglGetProcAddress( "glUniform1iv"));
	glUniform2fv = reinterpret_cast< PFNGLUNIFORM2FVPROC >					( wglGetProcAddress( "glUniform2fv" ) );
	glUniform3fv = reinterpret_cast< PFNGLUNIFORM3FVPROC >					( wglGetProcAddress( "glUniform3fv" ) );
	glUniform4fv = reinterpret_cast< PFNGLUNIFORM4FVPROC >					( wglGetProcAddress( "glUniform4fv" ) );
	glUniform4iv = reinterpret_cast< PFNGLUNIFORM4IVPROC >					( wglGetProcAddress( "glUniform4iv" ) );
	glUniform1i = reinterpret_cast< PFNGLUNIFORM1IPROC >					( wglGetProcAddress( "glUniform1i" ) );
	glUniform1f = reinterpret_cast< PFNGLUNIFORM1FPROC >					( wglGetProcAddress( "glUniform1f" ) );


	glUniformMatrix4fv  = reinterpret_cast< PFNGLUNIFORMMATRIX4FVPROC >		( wglGetProcAddress( "glUniformMatrix4fv" ) );
	glGetAttribLocation = reinterpret_cast< PFNGLGETATTRIBLOCATIONPROC >	( wglGetProcAddress( "glGetAttribLocation" ) );
	glEnableVertexAttribArray = reinterpret_cast< PFNGLENABLEVERTEXATTRIBARRAYPROC > ( wglGetProcAddress( "glEnableVertexAttribArray" ) );
	glDisableVertexAttribArray = reinterpret_cast< PFNGLDISABLEVERTEXATTRIBARRAYPROC > ( wglGetProcAddress( "glDisableVertexAttribArray" ) );
	glVertexAttribPointer = reinterpret_cast< PFNGLVERTEXATTRIBPOINTERPROC > ( wglGetProcAddress( "glVertexAttribPointer" ) );
	glUniform1fv = reinterpret_cast< PFNGLUNIFORM1FVPROC  > ( wglGetProcAddress( "glUniform1fv" ) );	
	glActiveTexture = reinterpret_cast< PFNGLACTIVETEXTUREPROC > ( wglGetProcAddress( "glActiveTexture" ) );
}