// Copyright � 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

// Include standard headers
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <cassert>

// Include GLEE
#include "glee.h"

// Include GLFW
#include <GL/glfw.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "GLvbo.h"
#include "utils.h"
#include "shaderHelper.h"
#include "TerrainMesh.h"

namespace NKeyboard {
	enum EKeyStates {
		eKeyUnset=0,
		eKeyPressed,
		eKeyHeld,
		eKeyReleased
	};
	#define NUM_KEYS 256
	EKeyStates g_keys[NUM_KEYS] = {eKeyUnset};
	EKeyStates g_keysPrev[NUM_KEYS] = {eKeyUnset};
	void UpdateKeyStates() {
		for( int k=0; k<NUM_KEYS; k++ ) 
		{
			g_keysPrev[k] = g_keys[k];
			g_keys[k] = (glfwGetKey(k)==GLFW_PRESS) ? eKeyPressed : eKeyReleased;
		}
	}
	EKeyStates GetKeyState( const uint8_t c )
	{
		switch(g_keys[c]) {
		case eKeyUnset:		return eKeyUnset;
		case eKeyPressed:	return g_keysPrev[c]==eKeyPressed ? eKeyHeld : eKeyPressed;
		case eKeyReleased:	return g_keysPrev[c]==eKeyReleased ? eKeyUnset : eKeyReleased;
		default: assert(false && "this shouldn't happen!"); break;
		}
		return eKeyUnset;
	}
};

int main()
{
	const int screen_width = 800;
	const int screen_height = 600;

	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	/*glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/

	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 2);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 1);

	// Open a window and create its OpenGL context
	if( !glfwOpenWindow( screen_width, screen_height, 0,0,0,0,24,0, GLFW_WINDOW ) )
	{
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}

	const GLboolean bInitOk = GLeeInit();
	assert(bInitOk == GL_TRUE);

	int MaxVertexTextureImageUnits;
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &MaxVertexTextureImageUnits);
	int MaxCombinedTextureImageUnits;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &MaxCombinedTextureImageUnits);

	glfwSetWindowTitle( "Sphere" );

	GLFWvidmode mode;
	glfwGetDesktopMode( &mode );
	const int screen_x_offset = (mode.Width - screen_width)>>1;
	const int screen_y_offset = (mode.Height - screen_height)>>1;
	glfwSetWindowPos( screen_x_offset, screen_y_offset );

	// Ensure we can capture the escape key being pressed below
	glfwEnable( GLFW_STICKY_KEYS );
	glfwSetMousePos(screen_width/2, screen_height/2);

	// Set color and depth clear value
    glClearDepth(1.0f);
	// Dark blue background
	//glClearColor(0.0f, 0.05f, 0.1f, 0.0f);
	glClearColor(100/255.f, 149/255.f, 237/255.f, 0.0f);
	checkGLError();

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 
	checkGLError();

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	checkGLError();
	//glDisable(GL_CULL_FACE);

	// a little aside, this calculates the radius(R) of a torus
	// the generates 1g for a given rpm.
	// the numbers get very big very quickly :(
	/*double rpm = 1.0;
	double cal = ((PI*rpm)/30.0);
	double g = 9.81;
	double R = g / (cal*cal);*/

	GeoSphere *pSphere = new GeoSphere();
	checkGLError();

	////////////////////////////////////////////////////////////////
    int		screenWide = screen_width;
	int		screenHigh = screen_height;
	float	screenWidef = float(screen_width);
	float	screenHighf = float(screen_height);
	float	aspect = screenWidef / screenHighf;

	int xprev=0, yprev=0;
	float theta=0.0f, phi=0.0f;
#ifdef _DEBUG
	float cube_theta=0.0f, cube_phi=0.0f;
#endif

	int mouseWPrev = glfwGetMouseWheel();
	float zoomDist = -15.0f;
	const float geoSphereRadius = 25.0f;

	bool bUseWireframe = false;

	do {
		////////////////////////////////////////////////////////////////
		// handle resizing the screen/window
		glfwGetWindowSize(&screenWide, &screenHigh);
		screenWidef = float(screenWide);
		screenHighf = float(screenHigh);
		aspect = screenWidef / screenHighf;

		////////////////////////////////////////////////////////////////
		// update the user input
		{
			const int mouseW = glfwGetMouseWheel();
			const int mouseWDiff = mouseW - mouseWPrev;
			mouseWPrev = mouseW;
			zoomDist += float(mouseWDiff) * 0.1f;
			zoomDist = Clamp<float>(zoomDist, -100.0f, 0.0f);
		}

		{
			int x, y;
			glfwGetMousePos(&x, &y);
			const int xDiff = x - xprev;
			const int yDiff = y - yprev;
			xprev = x;
			yprev = y;
			static const float phi_limit = 80.0f;
			if(GLFW_PRESS==glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT)) {
				theta += float(xDiff) * 0.1f;
				//theta = Clamp<float>(theta, -45.0f, 45.0f);
				phi += float(yDiff) * 0.1f;
				phi = Clamp<float>(phi, -phi_limit, phi_limit);
			} else if(GLFW_PRESS==glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT)) {
#ifdef _DEBUG
				// rotate/move the campos (cube marker)
				cube_theta += float(xDiff) * 0.2f;
				//theta = Clamp<float>(theta, -45.0f, 45.0f);
				cube_phi += float(yDiff) * 0.2f;
				cube_phi = Clamp<float>(cube_phi, -phi_limit, phi_limit);
#endif
			}
		}
		
		if( NKeyboard::GetKeyState('W') == NKeyboard::eKeyPressed ) {
			bUseWireframe = !bUseWireframe;
		}

		////////////////////////////////////////////////////////////////
		// Compute the MVP matrix from keyboard and mouse input
		// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 1000 units
		glm::mat4 ProjectionMatrix = perspective(90.0f, aspect, 0.1f, 1000.f);

		// Polar Camera matrix
		glm::mat4 matty;
		matty = translate(matty, glm::vec3(0.f, 0.f, (-geoSphereRadius) + zoomDist));
		matty = rotate(matty, phi, glm::vec3(1.f, 0.f, 0.f));
		matty = rotate(matty, theta, glm::vec3(0.f, 1.f, 0.f));
		//glm::mat4 ViewMatrix       = lookAt(
		//								glm::vec3(0.f, 0.f, -20.f),	// Camera is here
		//								glm::vec3(0.f, 0.f, 0.f),	// and looks here : at the same position, plus "direction"
		//								glm::vec3(0.f, 1.f, 0.f)	// Head is up (set to 0,-1,0 to look upside-down)
		//						   );
		glm::mat4 ViewMatrix	= matty;
		glm::mat4 ModelMatrix	= glm::mat4(1.0);
		glm::mat4 MVP			= ProjectionMatrix * ViewMatrix * ModelMatrix;

		////////////////////////////////////////////////////////////////
		// update, and possibly render the terrain for the sphere
		const glm::vec3 campos(1.0f, 0.0, 0.0);
		pSphere->Update(campos);

		////////////////////////////////////////////////////////////////
		// Render the main scene
		glViewport(0, 0, screenWide, screenHigh);
		checkGLError();

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		checkGLError();
		
		////////////////////////////////////////////////////////////////
		// render the bloody sphere here
		if(bUseWireframe) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		pSphere->Render(ViewMatrix,ModelMatrix,MVP);
		if(bUseWireframe) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}

		glUseProgram(0);
		checkGLError();

#ifdef _DEBUG
		glm::mat4 cubeView = matty;
		cubeView = translate(cubeView, glm::normalize(glm::vec3(1.f, 0.f, 0.f))*30.0f);
		cubeView = rotate(cubeView, cube_phi, glm::vec3(1.f, 0.f, 0.f));
		cubeView = rotate(cubeView, cube_theta, glm::vec3(0.f, 1.f, 0.f));
		
		pSphere->RenderCube(cubeView);

		pSphere->RenderLOD0Heightmaps();
#endif

		// Swap buffers
		glfwSwapBuffers();
		checkGLError();

		NKeyboard::UpdateKeyStates();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS && glfwGetWindowParam( GLFW_OPENED ) );

	checkGLError();
	if(pSphere) {
		delete pSphere;
		pSphere = nullptr;
	}
	checkGLError();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}



