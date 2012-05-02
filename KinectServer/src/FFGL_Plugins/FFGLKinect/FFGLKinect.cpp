#include <FFGL.h>
#include <FFGLLib.h>





#include "FFGLKinect.h"



#define FFPARAM_TileX (0)
#define FFPARAM_TileY (1)

GLuint gl_depth_tex;






/*



void *clientThread(void *ptr) {
	
	// init the client code
	printf("Connecting to the Kinect Server...\n");
	socket.connect ("tcp://localhost:5555");
	clientRunning=true;
	
	// init the texure
	depth = (uint8_t*) malloc(textureSize);
	
	
	while (clientRunning==true) {
		//printf("getting depth image.\n");
		s_send(socket, "getDepthmap");
		std::string requestResult = s_recv(socket);
		clientThreadLocked=true;
		//depthMap = (uint8_t *) strdup(requestResult.c_str());
		memcpy((void *) depth, (void *) requestResult.c_str(), textureSize);
		
		clientThreadLocked=false;
		//free (depthMap);		
	}
	free((void *) depth);
	 
}
 */



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
	FF_EFFECT,						// Plugin type
	"Sample FFGL Tile plugin",		// Plugin description
	"by Daniel Almeida for Resolume - www.resolume.com" // About
);

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


////////////////////////////////////////////////////////////////////////////////////////////////////
//  Constructor and destructor
////////////////////////////////////////////////////////////////////////////////////////////////////

ResolumeKinect::ResolumeKinect()
:CFreeFrameGLPlugin(),
 m_initResources(1),
 m_inputTextureLocation(-1),
 m_maxCoordsLocation(-1),
 m_tileAmountLocation(-1)
{
	// Input properties
	SetMinInputs(0);
	SetMaxInputs(0);	
	
}

ResolumeKinect::~ResolumeKinect(){
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Methods
////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD ResolumeKinect::InitGL(const FFGLViewportStruct *vp)
{
  m_extensions.Initialize();

  if (m_extensions.ARB_shader_objects==0)
	return FF_FAIL;

  m_shader.SetExtensions(&m_extensions);
  m_shader.Compile(vertexShaderCode,fragmentShaderCode);

  //activate our shader
  m_shader.BindShader();

  //to assign values to parameters in the shader, we have to lookup
  //the "location" of each value.. then call one of the glUniform* methods
  //to assign a value
  m_inputTextureLocation = m_shader.FindUniform("inputTexture");
  
  //the 0 means that the 'inputTexture' in
  //the shader will use the texture bound to GL texture unit 0
  m_extensions.glUniform1iARB(m_inputTextureLocation, 0);

  m_shader.UnbindShader();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_TEXTURE_2D);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_FLAT);

	// prepare the texture
	glGenTextures(1, &gl_depth_tex);
	glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
		// init the server
	initKinectServer();


	
  return FF_SUCCESS;
}

DWORD ResolumeKinect::DeInitGL()
{
	stopKinectServer();
	m_shader.FreeGLResources();
	return FF_SUCCESS;
}

DWORD ResolumeKinect::ProcessOpenGL_old(ProcessOpenGLStruct *pGL)
{
  if (pGL->numInputTextures<1)
    return FF_FAIL;

  if (pGL->inputTextures[0]==NULL)
    return FF_FAIL;
	


  //activate our shader
  m_shader.BindShader();

  FFGLTextureStruct &Texture = *(pGL->inputTextures[0]);

  //activate rendering with the input texture
  //note that when using shaders, no glEnable(Texture.Target) is required
  glBindTexture(GL_TEXTURE_2D, Texture.Handle);

  //get the max s,t that correspond to the
  //width,height of the used portion of the allocated texture space
  FFGLTexCoords maxCoords = GetMaxGLTexCoords(Texture);

  //assign the maxCoords value in the shader
  //(glUniform2f assigns to a vec2)
  m_extensions.glUniform2fARB(m_maxCoordsLocation, maxCoords.s, maxCoords.t);

  //assign the tileAmount
  m_extensions.glUniform2fARB(m_tileAmountLocation, 20.0*m_TileX, 20.0*m_TileY);
  
  //draw the quad that will be painted by the shader/texture
  glBegin(GL_QUADS);

	
  //lower left
  glTexCoord2f(0,0);
  glVertex2f(-1,-1);

  //upper left
  glTexCoord2f(0, maxCoords.t);
  glVertex2f(-1,1);

  //upper right
  glTexCoord2f(maxCoords.s, maxCoords.t);
  glVertex2f(1,1);

  //lower right
  glTexCoord2f(maxCoords.s, 0);
  glVertex2f(1,-1);
  glEnd();

  //unbind the shader and texture
  glBindTexture(GL_TEXTURE_2D, 0);
  m_shader.UnbindShader();
  
  return FF_SUCCESS;
}




DWORD ResolumeKinect::ProcessOpenGL(ProcessOpenGLStruct *pGL)
{
	/*
	if (pGL->numInputTextures<1)
		return FF_FAIL;
	
	if (pGL->inputTextures[0]==NULL)
		return FF_FAIL;
	*/
	
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
	uint8_t *depthMap = getKinectDepthMap();
	if (depthMap!=NULL) {
		glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, depthMap);
	}

	
	glBegin(GL_QUADS);
	glTexCoord2f(0.0,0.0);
	glVertex2f(-1.0,-1.0);
	glTexCoord2f(0.0, 1.0);
	glVertex2f(-1.0,1.0);
	glTexCoord2f(1.0, 1.0);
	glVertex2f(1.0,1.0);
	glTexCoord2f(1.0, 0.0);
	glVertex2f(1.0,-1.0);
	glEnd();
	

	//unbind the shader and texture
	glBindTexture(GL_TEXTURE_2D, 0);
	//m_shader.UnbindShader();
	
	glPopMatrix();
	
	glFlush();
	glDisable(GL_TEXTURE_2D);

	
	return FF_SUCCESS;
}




DWORD ResolumeKinect::GetParameter(DWORD dwIndex)
{
	DWORD dwRet;
	/*
	switch (dwIndex) {

	case FFPARAM_TileX:
    *((float *)(unsigned)&dwRet) = m_TileX;
		return dwRet;
	case FFPARAM_TileY:
    *((float *)(unsigned)&dwRet) = m_TileY;
		return dwRet;

	default:
		return FF_FAIL;
	}
	*/
	return FF_FAIL;
}

DWORD ResolumeKinect::SetParameter(const SetParameterStruct* pParam)
{
	/*
	if (pParam != NULL) {
		
		switch (pParam->ParameterNumber) {

		case FFPARAM_TileX:
			m_TileX = *((float *)(unsigned)&(pParam->NewParameterValue));
			break;
		case FFPARAM_TileY:
			m_TileY = *((float *)(unsigned)&(pParam->NewParameterValue));
			break;

		default:
			return FF_FAIL;
		}

		return FF_SUCCESS;
	
	}
	 */
	return FF_FAIL;
}
