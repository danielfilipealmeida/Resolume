#ifndef RESOLUME_KINECT_PLUGIN_H
#define RESOLUME_KINECT_PLUGIN_H

#include <FFGLShader.h>
#include "../FFGLPluginSDK.h"

//#include "../../Extra/KinectControl.h"

class ResolumeKinect :
public CFreeFrameGLPlugin
{
public:
  ResolumeKinect();
	virtual ~ResolumeKinect();
	
	//KinectControl *kinectControl;

	///////////////////////////////////////////////////
	// FreeFrame plugin methods
	///////////////////////////////////////////////////

	DWORD	SetParameter(const SetParameterStruct* pParam);
	DWORD	GetParameter(DWORD dwIndex);
	DWORD	ProcessOpenGL(ProcessOpenGLStruct *pGL);
	DWORD	ProcessOpenGL_old(ProcessOpenGLStruct *pGL);
	DWORD	InitGL(const FFGLViewportStruct *vp);
	DWORD	DeInitGL();

	///////////////////////////////////////////////////
	// Factory method
	///////////////////////////////////////////////////

	static DWORD __stdcall CreateInstance(CFreeFrameGLPlugin **ppOutInstance)
  {
	  *ppOutInstance = new ResolumeKinect();
	  if (*ppOutInstance != NULL)
      return FF_SUCCESS;
	  return FF_FAIL;
  }

protected:
	// Parameters
	float m_TileX;
	float m_TileY;

	int m_initResources;
	FFGLExtensions m_extensions;
	FFGLShader m_shader;
	GLint m_inputTextureLocation;
	GLint m_maxCoordsLocation;
	GLint m_tileAmountLocation;
};


#endif
