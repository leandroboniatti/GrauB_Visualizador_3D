#include <glad/glad.h>  // biblioteca de funções baseada nas definições/especificações OPENGL - Incluir antes de outros que requerem OpenGL (como GLFW)

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function pointers
PFNGLCLEARPROC glad_glClear = NULL;
PFNGLCLEARCOLORPROC glad_glClearColor = NULL;
PFNGLENABLEPROC glad_glEnable = NULL;
PFNGLVIEWPORTPROC glad_glViewport = NULL;
PFNGLGENVERTEXARRAYSPROC glad_glGenVertexArrays = NULL;
PFNGLBINDVERTEXARRAYPROC glad_glBindVertexArray = NULL;
PFNGLGENBUFFERSPROC glad_glGenBuffers = NULL;
PFNGLBINDBUFFERPROC glad_glBindBuffer = NULL;
PFNGLBUFFERDATAPROC glad_glBufferData = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glad_glVertexAttribPointer = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray = NULL;
PFNGLCREATESHADERPROC glad_glCreateShader = NULL;
PFNGLSHADERSOURCEPROC glad_glShaderSource = NULL;
PFNGLCOMPILESHADERPROC glad_glCompileShader = NULL;
PFNGLCREATEPROGRAMPROC glad_glCreateProgram = NULL;
PFNGLATTACHSHADERPROC glad_glAttachShader = NULL;
PFNGLLINKPROGRAMPROC glad_glLinkProgram = NULL;
PFNGLUSEPROGRAMPROC glad_glUseProgram = NULL;
PFNGLDELETESHADERPROC glad_glDeleteShader = NULL;
PFNGLDELETEPROGRAMPROC glad_glDeleteProgram = NULL;
PFNGLDELETEVERTEXARRAYSPROC glad_glDeleteVertexArrays = NULL;
PFNGLDELETEBUFFERSPROC glad_glDeleteBuffers = NULL;
PFNGLGETSHADERIVPROC glad_glGetShaderiv = NULL;
PFNGLGETSHADERINFOLOGPROC glad_glGetShaderInfoLog = NULL;
PFNGLGETPROGRAMIVPROC glad_glGetProgramiv = NULL;
PFNGLGETPROGRAMINFOLOGPROC glad_glGetProgramInfoLog = NULL;
PFNGLGETUNIFORMLOCATIONPROC glad_glGetUniformLocation = NULL;
PFNGLUNIFORM1IPROC glad_glUniform1i = NULL;
PFNGLUNIFORM1FPROC glad_glUniform1f = NULL;
PFNGLUNIFORM3FPROC glad_glUniform3f = NULL;
PFNGLUNIFORM3FVPROC glad_glUniform3fv = NULL;
PFNGLUNIFORMMATRIX4FVPROC glad_glUniformMatrix4fv = NULL;
PFNGLDRAWELEMENTSPROC glad_glDrawElements = NULL;
PFNGLDRAWARRAYSPROC glad_glDrawArrays = NULL;
PFNGLGENTEXTURESPROC glad_glGenTextures = NULL;
PFNGLBINDTEXTUREPROC glad_glBindTexture = NULL;
PFNGLTEXIMAGE2DPROC glad_glTexImage2D = NULL;
PFNGLTEXPARAMETERIPROC glad_glTexParameteri = NULL;
PFNGLGENERATEMIPMAPPROC glad_glGenerateMipmap = NULL;
PFNGLACTIVETEXTUREPROC glad_glActiveTexture = NULL;
PFNGLDELETETEXTURESPROC glad_glDeleteTextures = NULL;

#ifdef _WIN32
static void* get_proc(const char *namez) {
    void* p = (void*)wglGetProcAddress(namez);
    if(p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1)) {
        HMODULE module = LoadLibraryA("opengl32.dll");
        p = (void*)GetProcAddress(module, namez);
    }
    return p;
}
#else
#include <dlfcn.h>
static void* get_proc(const char *namez) {
    static void* libGL = NULL;
    if(!libGL) {
        libGL = dlopen("libGL.so.1", RTLD_LAZY | RTLD_GLOBAL);
        if(!libGL) {
            libGL = dlopen("libGL.so", RTLD_LAZY | RTLD_GLOBAL);
        }
    }
    return dlsym(libGL, namez);
}
#endif

int gladLoadGL(void) {
    glad_glClear = (PFNGLCLEARPROC)get_proc("glClear");
    glad_glClearColor = (PFNGLCLEARCOLORPROC)get_proc("glClearColor");
    glad_glEnable = (PFNGLENABLEPROC)get_proc("glEnable");
    glad_glViewport = (PFNGLVIEWPORTPROC)get_proc("glViewport");
    glad_glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)get_proc("glGenVertexArrays");
    glad_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)get_proc("glBindVertexArray");
    glad_glGenBuffers = (PFNGLGENBUFFERSPROC)get_proc("glGenBuffers");
    glad_glBindBuffer = (PFNGLBINDBUFFERPROC)get_proc("glBindBuffer");
    glad_glBufferData = (PFNGLBUFFERDATAPROC)get_proc("glBufferData");
    glad_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)get_proc("glVertexAttribPointer");
    glad_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)get_proc("glEnableVertexAttribArray");
    glad_glCreateShader = (PFNGLCREATESHADERPROC)get_proc("glCreateShader");
    glad_glShaderSource = (PFNGLSHADERSOURCEPROC)get_proc("glShaderSource");
    glad_glCompileShader = (PFNGLCOMPILESHADERPROC)get_proc("glCompileShader");
    glad_glCreateProgram = (PFNGLCREATEPROGRAMPROC)get_proc("glCreateProgram");
    glad_glAttachShader = (PFNGLATTACHSHADERPROC)get_proc("glAttachShader");
    glad_glLinkProgram = (PFNGLLINKPROGRAMPROC)get_proc("glLinkProgram");
    glad_glUseProgram = (PFNGLUSEPROGRAMPROC)get_proc("glUseProgram");
    glad_glDeleteShader = (PFNGLDELETESHADERPROC)get_proc("glDeleteShader");
    glad_glDeleteProgram = (PFNGLDELETEPROGRAMPROC)get_proc("glDeleteProgram");
    glad_glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)get_proc("glDeleteVertexArrays");
    glad_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)get_proc("glDeleteBuffers");
    glad_glGetShaderiv = (PFNGLGETSHADERIVPROC)get_proc("glGetShaderiv");
    glad_glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)get_proc("glGetShaderInfoLog");
    glad_glGetProgramiv = (PFNGLGETPROGRAMIVPROC)get_proc("glGetProgramiv");
    glad_glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)get_proc("glGetProgramInfoLog");
    glad_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)get_proc("glGetUniformLocation");
    glad_glUniform1i = (PFNGLUNIFORM1IPROC)get_proc("glUniform1i");
    glad_glUniform1f = (PFNGLUNIFORM1FPROC)get_proc("glUniform1f");
    glad_glUniform3f = (PFNGLUNIFORM3FPROC)get_proc("glUniform3f");
    glad_glUniform3fv = (PFNGLUNIFORM3FVPROC)get_proc("glUniform3fv");
    glad_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)get_proc("glUniformMatrix4fv");
    glad_glDrawElements = (PFNGLDRAWELEMENTSPROC)get_proc("glDrawElements");
    glad_glDrawArrays = (PFNGLDRAWARRAYSPROC)get_proc("glDrawArrays");
    glad_glGenTextures = (PFNGLGENTEXTURESPROC)get_proc("glGenTextures");
    glad_glBindTexture = (PFNGLBINDTEXTUREPROC)get_proc("glBindTexture");
    glad_glTexImage2D = (PFNGLTEXIMAGE2DPROC)get_proc("glTexImage2D");
    glad_glTexParameteri = (PFNGLTEXPARAMETERIPROC)get_proc("glTexParameteri");
    glad_glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)get_proc("glGenerateMipmap");
    glad_glActiveTexture = (PFNGLACTIVETEXTUREPROC)get_proc("glActiveTexture");
    glad_glDeleteTextures = (PFNGLDELETETEXTURESPROC)get_proc("glDeleteTextures");
    
    return 1;
}