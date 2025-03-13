#pragma once

#ifdef EMSCRIPTEN
// For emscripten, instead of using glad we use its built-in support for OpenGL:
#include <emscripten.h>
#include <GLES3/gl3.h>
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#else
#include <glad/glad.h>
#endif
