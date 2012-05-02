#include <FFGL.h>
#include <FFGLExtensions.h>
#include <stdlib.h>
#include <string.h>

#ifdef TARGET_OS_MAC
//for the NS* calls in GetProcAddress
#include <mach-o/dyld.h>
#endif
 
#ifdef __linux__
#include <GL/glx.h>
#endif

FFGLExtensions::FFGLExtensions()
{
  memset(this, 0, sizeof(FFGLExtensions));
}

void FFGLExtensions::Initialize()
{
#ifdef _WIN32
  InitWGLEXTSwapControl();
#endif

  InitMultitexture();
  InitARBShaderObjects();
  InitEXTFramebufferObject();
}

void *FFGLExtensions::GetProcAddress(char *name)
{

#ifdef _WIN32

  void *result = wglGetProcAddress(name);

  if (result!=NULL)
    return result;
    
#else

#ifdef TARGET_OS_MAC
  
  // Prepend a '_' for the Unix C symbol mangling convention
  int symbolLength = strlen(name) + 2; //1 for the _, another for the trailing null
  char symbolName[1024];
  if (symbolLength>sizeof(symbolName))
  {
    //symbol name too long;
    throw;
    return NULL;
  }
  
  strcpy(symbolName + 1, name);
  symbolName[0] = '_';

  NSSymbol symbol = NULL;
  
  if (NSIsSymbolNameDefined(symbolName))
    symbol = NSLookupAndBindSymbol(symbolName);
  
  if (symbol!=NULL)
  {
    return NSAddressOfSymbol(symbol);
  }

#else

#ifdef __linux__

  void *result = (void *)(unsigned)glXGetProcAddress((const GLubyte *)name);

  if (result!=NULL)
    return result;

#else
  
#error Define this for your OS
  
#endif
#endif
#endif
  throw;//this will be caught by one of the Init() functions below
  return NULL;
}

void FFGLExtensions::InitMultitexture()
{
  try
  {  
  glActiveTexture = (glActiveTexturePROC)(unsigned long)GetProcAddress("glActiveTexture");
  glClientActiveTexture = (glClientActiveTexturePROC)(unsigned long)GetProcAddress("glClientActiveTexture");

  glMultiTexCoord1d = (glMultiTexCoord1dPROC)(unsigned long)GetProcAddress("glMultiTexCoord1d");
  glMultiTexCoord1dv = (glMultiTexCoord1dvPROC)(unsigned long)GetProcAddress("glMultiTexCoord1dv");
  glMultiTexCoord1f = (glMultiTexCoord1fPROC)(unsigned long)GetProcAddress("glMultiTexCoord1f");
  glMultiTexCoord1fv = (glMultiTexCoord1fvPROC)(unsigned long)GetProcAddress("glMultiTexCoord1fv");
  glMultiTexCoord1i = (glMultiTexCoord1iPROC)(unsigned long)GetProcAddress("glMultiTexCoord1i");
  glMultiTexCoord1iv = (glMultiTexCoord1ivPROC)(unsigned long)GetProcAddress("glMultiTexCoord1iv");
  glMultiTexCoord1s = (glMultiTexCoord1sPROC)(unsigned long)GetProcAddress("glMultiTexCoord1s");
  glMultiTexCoord1sv = (glMultiTexCoord1svPROC)(unsigned long)GetProcAddress("glMultiTexCoord1sv");

  glMultiTexCoord2d = (glMultiTexCoord2dPROC)(unsigned long)GetProcAddress("glMultiTexCoord2d");
  glMultiTexCoord2dv = (glMultiTexCoord2dvPROC)(unsigned long)GetProcAddress("glMultiTexCoord2dv");
  glMultiTexCoord2f = (glMultiTexCoord2fPROC)(unsigned long)GetProcAddress("glMultiTexCoord2f");
  glMultiTexCoord2fv = (glMultiTexCoord2fvPROC)(unsigned long)GetProcAddress("glMultiTexCoord2fv");
  glMultiTexCoord2i = (glMultiTexCoord2iPROC)(unsigned long)GetProcAddress("glMultiTexCoord2i");
  glMultiTexCoord2iv = (glMultiTexCoord2ivPROC)(unsigned long)GetProcAddress("glMultiTexCoord2iv");
  glMultiTexCoord2s = (glMultiTexCoord2sPROC)(unsigned long)GetProcAddress("glMultiTexCoord2s");
  glMultiTexCoord2sv = (glMultiTexCoord2svPROC)(unsigned long)GetProcAddress("glMultiTexCoord2sv");

  glMultiTexCoord3d = (glMultiTexCoord3dPROC)(unsigned long)GetProcAddress("glMultiTexCoord3d");
  glMultiTexCoord3dv = (glMultiTexCoord3dvPROC)(unsigned long)GetProcAddress("glMultiTexCoord3dv");
  glMultiTexCoord3f = (glMultiTexCoord3fPROC)(unsigned long)GetProcAddress("glMultiTexCoord3f");
  glMultiTexCoord3fv = (glMultiTexCoord3fvPROC)(unsigned long)GetProcAddress("glMultiTexCoord3fv");
  glMultiTexCoord3i = (glMultiTexCoord3iPROC)(unsigned long)GetProcAddress("glMultiTexCoord3i");
  glMultiTexCoord3iv = (glMultiTexCoord3ivPROC)(unsigned long)GetProcAddress("glMultiTexCoord3iv");
  glMultiTexCoord3s = (glMultiTexCoord3sPROC)(unsigned long)GetProcAddress("glMultiTexCoord3s");
  glMultiTexCoord3sv = (glMultiTexCoord3svPROC)(unsigned long)GetProcAddress("glMultiTexCoord3sv");

  glMultiTexCoord4d = (glMultiTexCoord4dPROC)(unsigned long)GetProcAddress("glMultiTexCoord4d");
  glMultiTexCoord4dv = (glMultiTexCoord4dvPROC)(unsigned long)GetProcAddress("glMultiTexCoord4dv");
  glMultiTexCoord4f = (glMultiTexCoord4fPROC)(unsigned long)GetProcAddress("glMultiTexCoord4f");
  glMultiTexCoord4fv = (glMultiTexCoord4fvPROC)(unsigned long)GetProcAddress("glMultiTexCoord4fv");
  glMultiTexCoord4i = (glMultiTexCoord4iPROC)(unsigned long)GetProcAddress("glMultiTexCoord4i");
  glMultiTexCoord4iv = (glMultiTexCoord4ivPROC)(unsigned long)GetProcAddress("glMultiTexCoord4iv");
  glMultiTexCoord4s = (glMultiTexCoord4sPROC)(unsigned long)GetProcAddress("glMultiTexCoord4s");
  glMultiTexCoord4sv = (glMultiTexCoord4svPROC)(unsigned long)GetProcAddress("glMultiTexCoord4sv");
  }
  catch (...)
  {
    //not supported
    multitexture = 0;
    return;
  }
  
  //if we get this far w/no exceptions, ARB_shader_objects shoudl be fully
  //supported
  multitexture = 1;
}

void FFGLExtensions::InitARBShaderObjects()
{
  try
  {

  glDeleteObjectARB = (glDeleteObjectARBPROC)(unsigned long)GetProcAddress("glDeleteObjectARB");
  glGetHandleARB = (glGetHandleARBPROC)(unsigned long)GetProcAddress("glGetHandleARB");
  glDetachObjectARB = (glDetachObjectARBPROC)(unsigned long)GetProcAddress("glDetachObjectARB");
  glCreateShaderObjectARB = (glCreateShaderObjectARBPROC)(unsigned long)GetProcAddress("glCreateShaderObjectARB");
  glShaderSourceARB = (glShaderSourceARBPROC)(unsigned long)GetProcAddress("glShaderSourceARB");
  glCompileShaderARB = (glCompileShaderARBPROC)(unsigned long)GetProcAddress("glCompileShaderARB");
  glCreateProgramObjectARB = (glCreateProgramObjectARBPROC)(unsigned long)GetProcAddress("glCreateProgramObjectARB");
  glAttachObjectARB = (glAttachObjectARBPROC)(unsigned long)GetProcAddress("glAttachObjectARB");
  glLinkProgramARB = (glLinkProgramARBPROC)(unsigned long)GetProcAddress("glLinkProgramARB");
  glUseProgramObjectARB = (glUseProgramObjectARBPROC)(unsigned long)GetProcAddress("glUseProgramObjectARB");
  glValidateProgramARB = (glValidateProgramARBPROC)(unsigned long)GetProcAddress("glValidateProgramARB");
  glUniform1fARB = (glUniform1fARBPROC)(unsigned long)GetProcAddress("glUniform1fARB");
  glUniform2fARB = (glUniform2fARBPROC)(unsigned long)GetProcAddress("glUniform2fARB");
  glUniform3fARB = (glUniform3fARBPROC)(unsigned long)GetProcAddress("glUniform3fARB");
  glUniform4fARB = (glUniform4fARBPROC)(unsigned long)GetProcAddress("glUniform4fARB");
  glUniform1iARB = (glUniform1iARBPROC)(unsigned long)GetProcAddress("glUniform1iARB");
  glUniform2iARB = (glUniform2iARBPROC)(unsigned long)GetProcAddress("glUniform2iARB");
  glUniform3iARB = (glUniform3iARBPROC)(unsigned long)GetProcAddress("glUniform3iARB");
  glUniform4iARB = (glUniform4iARBPROC)(unsigned long)GetProcAddress("glUniform4iARB");
  glUniform1fvARB = (glUniform1fvARBPROC)(unsigned long)GetProcAddress("glUniform1fvARB");
  glUniform2fvARB = (glUniform2fvARBPROC)(unsigned long)GetProcAddress("glUniform2fvARB");
  glUniform3fvARB = (glUniform3fvARBPROC)(unsigned long)GetProcAddress("glUniform3fvARB");
  glUniform4fvARB = (glUniform4fvARBPROC)(unsigned long)GetProcAddress("glUniform4fvARB");
  glUniform1ivARB = (glUniform1ivARBPROC)(unsigned long)GetProcAddress("glUniform1ivARB");
  glUniform2ivARB = (glUniform2ivARBPROC)(unsigned long)GetProcAddress("glUniform2ivARB");
  glUniform3ivARB = (glUniform3ivARBPROC)(unsigned long)GetProcAddress("glUniform3ivARB");
  glUniform4ivARB = (glUniform4ivARBPROC)(unsigned long)GetProcAddress("glUniform4ivARB");
  glUniformMatrix2fvARB = (glUniformMatrix2fvARBPROC)(unsigned long)GetProcAddress("glUniformMatrix2fvARB");
  glUniformMatrix3fvARB = (glUniformMatrix3fvARBPROC)(unsigned long)GetProcAddress("glUniformMatrix3fvARB");
  glUniformMatrix4fvARB = (glUniformMatrix4fvARBPROC)(unsigned long)GetProcAddress("glUniformMatrix4fvARB");
  glGetObjectParameterfvARB = (glGetObjectParameterfvARBPROC)(unsigned long)GetProcAddress("glGetObjectParameterfvARB");
  glGetObjectParameterivARB = (glGetObjectParameterivARBPROC)(unsigned long)GetProcAddress("glGetObjectParameterivARB");
  glGetInfoLogARB = (glGetInfoLogARBPROC)(unsigned long)GetProcAddress("glGetInfoLogARB");
  glGetAttachedObjectsARB = (glGetAttachedObjectsARBPROC)(unsigned long)GetProcAddress("glGetAttachedObjectsARB");
  glGetUniformLocationARB = (glGetUniformLocationARBPROC)(unsigned long)GetProcAddress("glGetUniformLocationARB");
  glGetActiveUniformARB = (glGetActiveUniformARBPROC)(unsigned long)GetProcAddress("glGetActiveUniformARB");
  glGetUniformfvARB = (glGetUniformfvARBPROC)(unsigned long)GetProcAddress("glGetUniformfvARB");
  glGetUniformivARB = (glGetUniformivARBPROC)(unsigned long)GetProcAddress("glGetUniformivARB");
  glGetShaderSourceARB = (glGetShaderSourceARBPROC)(unsigned long)GetProcAddress("glGetShaderSourceARB");

  }
  catch (...)
  {
    //not supported
    ARB_shader_objects = 0;
    return;
  }

  //if we get this far w/no exceptions, ARB_shader_objects shoudl be fully
  //supported
  ARB_shader_objects = 1;
}

void FFGLExtensions::InitEXTFramebufferObject()
{
  try
  {

  glBindFramebufferEXT = (glBindFramebufferEXTPROC)(unsigned long)GetProcAddress("glBindFramebufferEXT");
  glBindRenderbufferEXT = (glBindRenderbufferEXTPROC)(unsigned long)GetProcAddress("glBindRenderbufferEXT");
  glCheckFramebufferStatusEXT = (glCheckFramebufferStatusEXTPROC)(unsigned long)GetProcAddress("glCheckFramebufferStatusEXT");
  glDeleteFramebuffersEXT = (glDeleteFramebuffersEXTPROC)(unsigned long)GetProcAddress("glDeleteFramebuffersEXT");
  glDeleteRenderBuffersEXT = (glDeleteRenderBuffersEXTPROC)(unsigned long)GetProcAddress("glDeleteRenderbuffersEXT");
  glFramebufferRenderbufferEXT = (glFramebufferRenderbufferEXTPROC)(unsigned long)GetProcAddress("glFramebufferRenderbufferEXT");
  glFramebufferTexture1DEXT = (glFramebufferTexture1DEXTPROC)(unsigned long)GetProcAddress("glFramebufferTexture1DEXT");
  glFramebufferTexture2DEXT = (glFramebufferTexture2DEXTPROC)(unsigned long)GetProcAddress("glFramebufferTexture2DEXT");
  glFramebufferTexture3DEXT = (glFramebufferTexture3DEXTPROC)(unsigned long)GetProcAddress("glFramebufferTexture3DEXT");
  glGenFramebuffersEXT = (glGenFramebuffersEXTPROC)(unsigned long)GetProcAddress("glGenFramebuffersEXT");
  glGenRenderbuffersEXT = (glGenRenderbuffersEXTPROC)(unsigned long)GetProcAddress("glGenRenderbuffersEXT");
  glGenerateMipmapEXT = (glGenerateMipmapEXTPROC)(unsigned long)GetProcAddress("glGenerateMipmapEXT");
  glGetFramebufferAttachmentParameterivEXT = (glGetFramebufferAttachmentParameterivEXTPROC)(unsigned long)GetProcAddress("glGetFramebufferAttachmentParameterivEXT");
  glGetRenderbufferParameterivEXT = (glGetRenderbufferParameterivEXTPROC)(unsigned long)GetProcAddress("glGetRenderbufferParameterivEXT");
  glIsFramebufferEXT = (glIsFramebufferEXTPROC)(unsigned long)GetProcAddress("glIsFramebufferEXT");
  glIsRenderbufferEXT = (glIsRenderbufferEXTPROC)(unsigned long)GetProcAddress("glIsRenderbufferEXT");
  glRenderbufferStorageEXT = (glRenderbufferStorageEXTPROC)(unsigned long)GetProcAddress("glRenderbufferStorageEXT");

  }
  catch (...)
  {
    //not supported
    EXT_framebuffer_object = 0;
    return;
  }

  EXT_framebuffer_object = 1;
}

#ifdef _WIN32
void FFGLExtensions::InitWGLEXTSwapControl()
{
  try
  {
  wglSwapIntervalEXT = (wglSwapIntervalEXTPROC) GetProcAddress("wglSwapIntervalEXT");
  wglGetSwapIntervalEXT = (wglGetSwapIntervalEXTPROC) GetProcAddress("wglGetSwapIntervalEXT");
  }
  catch (...)
  {
    //not supported
    WGL_EXT_swap_control = 0;
    return;
  }

  WGL_EXT_swap_control = 1;
}
#endif
