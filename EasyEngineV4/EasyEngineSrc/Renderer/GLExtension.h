#ifndef RENDERER_CPP
#ifndef EXTENSION_CPP
#error
#endif
#endif

#ifndef GL_EXTENSION_H
#define GL_EXTENSION_H

#include <windows.h>
#include <gl/gl.h>
#include "gl/glExt.h"





extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLISSHADERPROC glIsShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERARBPROC glCompileShader;
extern PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLGETPROGRAMIVARBPROC glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORM3FPROC glUniform3f;
extern PFNGLUNIFORM4FPROC glUniform4f;
extern PFNGLUNIFORM1IVPROC glUniform1iv;
extern PFNGLUNIFORM2FVPROC glUniform2fv;
extern PFNGLUNIFORM3FVPROC glUniform3fv;
extern PFNGLUNIFORM4FVPROC glUniform4fv;
extern PFNGLUNIFORM4IVPROC glUniform4iv;
extern PFNGLGENBUFFERSARBPROC glGenBuffers;
extern PFNGLBINDBUFFERARBPROC glBindBuffer;
extern PFNGLDELETEBUFFERSARBPROC glDeleteBuffers;	
extern PFNGLBUFFERDATAARBPROC glBufferData;
extern PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameteriv;
extern PFNGLGETBUFFERSUBDATAARBPROC	glGetBufferSubData;
extern PFNGLBUFFERSUBDATAARBPROC glBufferSubData;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM1FVPROC glUniform1fv;
extern PFNGLUNIFORMMATRIX4FVPROC  glUniformMatrix4fv;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
extern PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
extern PFNGLDRAWARRAYSINSTANCEDEXTPROC glDrawArraysInstanced;
extern PFNGLDRAWELEMENTSINSTANCEDEXTPROC glDrawElementsInstanced;
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
extern PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
extern PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture;
extern PFNGLDRAWBUFFERSPROC glDrawBuffers;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;

void InitExtensions();

#endif // GL_EXTENSION_H