#include <FFGL.h>
#include <FFGLLib.h>





#include "FFGLKinect.h"



#define FFPARAM_Mode (0)

GLuint gl_depth_tex;







////////////////////////////////////////////////////////////////////////////////////////////////////
//  Plugin information
////////////////////////////////////////////////////////////////////////////////////////////////////

static CFFGLPluginInfo PluginInfo (
	ResolumeKinect::CreateInstance,	// Create method
	"GLRK",							// Plugin unique ID
	"ResolumeKinect",				// Plugin name
	1,								// API major version number
	000,							// API minor version number
	1,								// Plugin major version number
	000,							// Plugin minor version number
	FF_SOURCE,						// Plugin type
	"Sample FFGL Tile plugin",		// Plugin description
	"by Daniel Almeida for Resolume - www.resolume.com" // About
);

/*
char *vertexShaderCode = (char *)
"void main()"
"{"
"  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
"  gl_TexCoord[0] = gl_MultiTexCoord0;"
"  gl_FrontColor = gl_Color;"
"}";


char *fragmentShaderCode = (char *)
"uniform sampler2D inputTexture;"
"void main(void)"
"{"
"  vec2 texCoord = gl_TexCoord[0].st;"
"  gl_FragColor = texture2D(inputTexture, texCoord);"
"}";
*/

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Constructor and destructor
////////////////////////////////////////////////////////////////////////////////////////////////////

ResolumeKinect::ResolumeKinect()
:CFreeFrameGLPlugin()
{
	// Input properties
	SetMinInputs(1);
	SetMaxInputs(1);	
	SetParamInfo(FFPARAM_Mode, "Mode", FF_TYPE_STANDARD, 0.0f);
	mode = 0;
}

ResolumeKinect::~ResolumeKinect(){
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Methods
////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD ResolumeKinect::InitGL(const FFGLViewportStruct *vp)
{
	// prepare the texture
	glGenTextures(1, &gl_depth_tex);
	glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	// init the server
	initKinectServer();
	setKinectMode(mode);
 return FF_SUCCESS;
}

DWORD ResolumeKinect::DeInitGL()
{
	stopKinectServer();
	return FF_SUCCESS;
}



DWORD ResolumeKinect::ProcessOpenGL(ProcessOpenGLStruct *pGL)
{

	if (pGL->numInputTextures<1)
		return FF_FAIL;
	
	if (pGL->inputTextures[0]==NULL)
		return FF_FAIL;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);

	
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_FLAT);
	
	// get the data
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glPushMatrix();
	
	//glScalef(0.5, 0.5, 1.0);
	if (mode == 0 ) {
		uint8_t *rangeMap = getKinectDepthMap();
		if (rangeMap!=NULL) {
			glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, rangeMap);
		}
	}
	
	if (mode == 1 ) {
		uint8_t *rgbMap = getKinectRGB();
		if (rgbMap!=NULL) {
			glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbMap);
		}
	}
	
	glBegin(GL_QUADS);
	glTexCoord2f(0.0,0.0);
	glVertex2f(-1.0,1.0);
	glTexCoord2f(0.0, 1.0);
	glVertex2f(-1.0,-1.0);
	glTexCoord2f(1.0, 1.0);
	glVertex2f(1.0,-1.0);
	glTexCoord2f(1.0, 0.0);
	glVertex2f(1.0,1.0);
	glEnd();
	
	//unbind the shader and texture
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glPopMatrix();
	
	glFlush();
	glDisable(GL_TEXTURE_2D);

	return FF_SUCCESS;
}




DWORD ResolumeKinect::GetParameter(DWORD dwIndex)
{		
	DWORD dwRet;
	switch (dwIndex) {

	case FFPARAM_Mode:
    *((float *)(unsigned)&dwRet) = (float)mode;
		return dwRet;

	default:
		return FF_FAIL;
	}
		 
	return FF_FAIL;
}

DWORD ResolumeKinect::SetParameter(const SetParameterStruct* pParam)
{
	float val;
	if (pParam != NULL) {
		
		switch (pParam->ParameterNumber) {

		case FFPARAM_Mode:
				val = *((float *)(unsigned)&(pParam->NewParameterValue));
			//mode = *((int *)(unsigned)&(pParam->NewParameterValue));
				if(val<0.5) mode=0;
				if(val>=0.5) mode=1;
				setKinectMode(mode);
			
			break;


		default:
			return FF_FAIL;
		}

		return FF_SUCCESS;
	
	}
	 
	return FF_FAIL;
}
