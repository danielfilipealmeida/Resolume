#ifndef RESOLUME_KINECT_PLUGIN_H
#define RESOLUME_KINECT_PLUGIN_H

#include <FFGLShader.h>
#include "../FFGLPluginSDK.h"
#include "KinectClient.h"

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
	int mode;

};


#endif
