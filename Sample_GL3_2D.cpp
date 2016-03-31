#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
	float CX;
	float CY;
	float r;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
float rot_ang=0; double key_press_time = 0;double key_release_time = 0 , u_f;
int Obs1_o = 0 ;
int Obs2_o = 0 ;
int Obs3_o = 0 ;
double x=0 , y = 0 , u = 15 , t =0 ;
int flag = 0;
int score=0;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_C:
				rectangle_rot_status = !rectangle_rot_status;
				break;
			case GLFW_KEY_P:
				triangle_rot_status = !triangle_rot_status;
				break;
			case GLFW_KEY_SPACE:
				key_release_time = glfwGetTime();
				u_f = key_release_time - key_press_time;
				u = u*u_f*2.5;
				flag=1;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			case GLFW_KEY_W:
				if( rot_ang >= 30 )
					rot_ang = 30 ;
				else
					rot_ang += 10;
				break;
			case GLFW_KEY_S:
				if ( rot_ang <= -20)
					rot_ang = -20;
				else
					rot_ang -= 10;
				break;
			case GLFW_KEY_SPACE:
				key_press_time = glfwGetTime();
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			default:
				break;
		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_RELEASE)
				triangle_rot_dir *= -1;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE) {
				rectangle_rot_dir *= -1;
			}
			break;
		default:
			break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	Matrices.projection = glm::ortho(-9.0f, 9.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *obj[20] , *triangle, *rectangle, *rectangle3 , *circleBall , *circleSun , *Obs1 , *Obs2 , *Obs3 ; 

// Creates the triangle object used in this sample code
void createTriangle ()
{
	/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

	/* Define vertex array as used in glBegin (GL_TRIANGLES) */
	static const GLfloat vertex_buffer_data [] = {
		-7.6,-1.1 ,0, // vertex 0
		-8.6,-2.5,0, // vertex 1
		-6.6,-2.5,0, // vertex 2
	};

	static const GLfloat color_buffer_data [] = {
		0,0,0, // color 0
		0,0,0, // color 1
		0,0,0, // color 2
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}


void createRectangle3()
{
	static const GLfloat vertex_buffer_data [] = {
		-7.8, -1.1 , 0 ,
		-7.8, -1.5 , 0 ,
		-6.0, -1.5 , 0 ,	

		-6.0, -1.5 , 0 ,	
		-6.0, -1.1 , 0 ,
		-7.8, -1.1, 0 ,
	};

	rectangle3 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data,0,0,0);
}
// Creates the rectangle object used in this sample code
void createRectangle ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-12,-2.5,0, // vertex 1
		9,-2.5,0, // vertex 2
		9, -4.5,0, // vertex 3

		9, -4.5,0, // vertex 3
		-12, -4.5,0, // vertex 4
		-12,-2.5,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		0.1,0.7,0.1,//1,0,0, // color 1
		0.1,0.7,0.1,//0,0,1, // color 2
		0.1,0.7,0.1,//0,1,0, // color 3

		0.1,0.7,0.1,//0,1,0, // color 3
		0.1,0.7,0.1,//0.3,0.3,0.3, // color 4
		0.1,0.7,0.1,//1,0,0  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createObs1 ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		1,1,0, // vertex 1
		1.5,1,0, // vertex 2
		1, 1.5,0, // vertex 3

		1, 1.5,0, // vertex 3
		1.5, 1.5,0, // vertex 4
		1,1,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0.3,0.3,0.3, // color 4
		1,0,0  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	Obs1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data,0,0,0, GL_FILL);

	Obs1->r = 0.353;
	Obs1->CX = 1.25;
	Obs1->CY = 1.25;

	if(Obs1_o == 0 )
	{
		obj[1] = Obs1;
		
	}
}

void createObs2 ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		1,1.9,0, // vertex 1
		1.5,1.9,0, // vertex 2
		1, 2.4,0, // vertex 3

		1, 2.4,0, // vertex 3
		1.5, 2.4,0, // vertex 4
		1,1.9,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0.3,0.3,0.3, // color 4
		1,0,0  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	Obs2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data,0,0,0, GL_FILL);

	Obs2->r = 0.353;
	Obs2->CX = 1.25;
	Obs2->CY = 1.25;

	if(Obs2_o == 0 )
	{
		obj[2] = Obs2;
		
	}
}

void createObs3 ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		1,1,0, // vertex 1
		1.5,1,0, // vertex 2
		1, 1.5,0, // vertex 3

		1, 1.5,0, // vertex 3
		1.5, 1.5,0, // vertex 4
		1,1,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0.3,0.3,0.3, // color 4
		1,0,0  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	Obs3 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data,0,0,0, GL_FILL);

	Obs3->r = 0.353;
	Obs3->CX = 1.25;
	Obs3->CY = 1.25;

	if(Obs3_o == 0 )
	{
		obj[3] = Obs3;
		
	}
}

/*
   void createRectangle ()
   {
// GL3 accepts only Triangles. Quads are not supported
static const GLfloat vertex_buffer_data [] = {
-12,-2.5,0, // vertex 1
9,-2.5,0, // vertex 2
9, -4.5,0, // vertex 3

9, -4.5,0, // vertex 3
-12, -4.5,0, // vertex 4
-12,-2.5,0  // vertex 1
};

static const GLfloat color_buffer_data [] = {
0.1,0.7,0.1,//1,0,0, // color 1
0.1,0.7,0.1,//0,0,1, // color 2
0.1,0.7,0.1,//0,1,0, // color 3

0.1,0.7,0.1,//0,1,0, // color 3
0.1,0.7,0.1,//0.3,0.3,0.3, // color 4
0.1,0.7,0.1,//1,0,0  // color 1
};

// create3DObject creates and returns a handle to a VAO that can be used later
rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
 */


void checkCollision()
{
	float x1 = -7.6+x/10;
	float y1 = -1.1+y/10;
	float r1 = 0.2;
	float x2 , y2 , d1 , r2 , x3 , y3 , d2 , r3 , x4 , y4, d3 , r4;
	x2 = obj[1]->CX;
	y2 = obj[1]->CY; 
	d1 = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
	r2 = obj[1]->r;
	if ( d1 < r1 + r2)
	{
		Obs1_o = 1;
		score++;
	}
	
	x3 = obj[1]->CX+3.0;
	y3 = obj[1]->CY+2.5; 
	d2 = sqrt((x3-x1)*(x3-x1) + (y3-y1)*(y3-y1));
	r3 = obj[1]->r;
	if ( d2 < r1 + r3)
	{
		Obs2_o = 1;
		score++;
	}

	x4 = obj[1]->CX+2.0;
	y4 = obj[1]->CY-1.5; 
	d3 = sqrt((x4-x1)*(x4-x1) + (y4-y1)*(y4-y1));
	r4 = obj[1]->r;
	if ( d3 < r1 + r4)
	{
		Obs3_o = 1;
		score++;
	}


}




void drawCircle( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides )
{
	int numberOfVertices = numberOfSides + 2;

	GLfloat twicePi = 2.0f * M_PI;

	GLfloat circleVerticesX[numberOfVertices];
	GLfloat circleVerticesY[numberOfVertices];
	GLfloat circleVerticesZ[numberOfVertices];

	circleVerticesX[0] = x;
	circleVerticesY[0] = y;
	circleVerticesZ[0] = z;

	for ( int i = 1; i < numberOfVertices; i++ )
	{
		circleVerticesX[i] = x + ( radius * cos( i *  twicePi / numberOfSides ) );
		circleVerticesY[i] = y + ( radius * sin( i * twicePi / numberOfSides ) );
		circleVerticesZ[i] = z;
	}

	GLfloat allCircleVertices[( numberOfVertices ) * 3];

	for ( int i = 0; i < numberOfVertices; i++ )
	{
		allCircleVertices[i * 3] = circleVerticesX[i];
		allCircleVertices[( i * 3 ) + 1] = circleVerticesY[i];
		allCircleVertices[( i * 3 ) + 2] = circleVerticesZ[i];
	}




	// create3DObject creates and returns a handle to a VAO that can be used later

	circleBall = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,1,1,1);
	circleSun = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,1,1,0);


}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;



/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix
	Matrices.model = glm::mat4(1.0f);

	/* Render your scene */

	glm::mat4 translateTriangle = glm::translate (glm::vec3(-0.0f, 0.0f, 0.0f)); // glTranslatef
	//	glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	glm::mat4 triangleTransform = translateTriangle ;//* rotateTriangle;
	Matrices.model *= triangleTransform; 
	MVP = VP * Matrices.model; // MVP = p * V * M

	//  Don't change unless you are sure!!
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(triangle);

	Matrices.model = glm::mat4(1.0f);

	/*	glm::mat4 translateTriangle1 = glm::translate (glm::vec3(-5.0f, 0.0f, 0.0f)); // glTranslatef
	// glm::mat4 rotateTriangle1 = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	glm::mat4 triangleTransform1 = translateTriangle1; //* rotateTriangle1;
	Matrices.model *= triangleTransform1; 
	MVP = VP * Matrices.model; // MVP = p * V * M


	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(triangle);
	 */
	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();
	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
	// glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateRectangle);//rotateRectangle);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(rectangle);
	if( Obs1_o == 0)
	{
		Matrices.model = glm::mat4(1.0f);

		glm::mat4 translateObs1 = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
		// glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateObs1);//rotateRectangle);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// draw3DObject draws the VAO given to it using current MVP matrix
		draw3DObject(Obs1);
	}
	if( Obs2_o == 0)
	{
		Matrices.model = glm::mat4(1.0f);

		glm::mat4 translateObs1 = glm::translate (glm::vec3(4, 1.5, 0));        // glTranslatef
		// glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateObs1);//rotateRectangle);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// draw3DObject draws the VAO given to it using current MVP matrix
		draw3DObject(Obs2);
	}
	
	if( Obs3_o == 0)
	{
		Matrices.model = glm::mat4(1.0f);

		glm::mat4 translateObs1 = glm::translate (glm::vec3(2, -1.5, 0));        // glTranslatef
		// glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= (translateObs1);//rotateRectangle);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// draw3DObject draws the VAO given to it using current MVP matrix
		draw3DObject(Obs3);
	}

	Matrices.model = glm::mat4(1.0f);    
	//  glm::mat4 scaleRectangle5 = glm::scale (glm::vec3(1.4, 1.4, 0.0));
	glm::mat4 tr= glm::translate (glm::vec3(7.8,2.5,0));        // glTranslatef
	glm::mat4 rr = glm::rotate((float)(rot_ang*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	glm::mat4 tr1 = glm::translate (glm::vec3(-7.8,-2.5,0));
	Matrices.model *= (  tr1*rr*tr  );
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(rectangle3);

	// Increment angles
	float increments = 1;

	//camera_rotation_angle++; // Simulating camera rotation
	triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;

	if( flag == 1)
	{
		float z1,z2;
		z1 = -7.6 + x/10;
		z2 = -1.1 + y/10;
		//cout<<"\nz1 = "<<z1<<"\t"<<z2<<endl;
		if( z1 > 8.0 || z2 > 4.0 || z2 < -4.0)
		{
			flag = 0;
			x=0;
			y=0;
			u=15;
			t=0;
		}
		drawCircle ( 0,0,0,0.2,360);
		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translateCircle = glm::translate (glm::vec3(-7.6+x/10, -1.1+y/10, 0));   // glTranslatef
		Matrices.model *= (translateCircle);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(circleBall);
		x = u*cos(rot_ang*M_PI/180)*t;
		y = u*sin(rot_ang*M_PI/180)*t - t*t;
		t += 0.08;
	}
	
	int fsun = 15;
	if( fsun == 15 )
	{
	fsun = 14;
	drawCircle ( 0,0,0,0.4,360);
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateCircle = glm::translate (glm::vec3(-7.6, 3.1, 0));   // glTranslatef
	Matrices.model *= (translateCircle);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(circleSun);
	}
	
	if( Obs1_o == 1 && Obs2_o == 1 && Obs3_o == 1 )
	{
		float z1 = -7.6 + x/10;
		float z2 = -1.1 + y/10;
		if( z1 > 8.0 || z2 > 4.0 || z2 < -4.0)
		{
			cout<<endl<<endl<<"YOU WON!!!  SCORE: "<<score<<endl;
		exit(0);
	}
	}
		
	checkCollision();
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/* Objects should be created before any other gl function and shaders */
	// Create the models
	createRectangle ();
	createRectangle3();
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createObs1();
	createObs2();
	createObs3();
//	cout<<score<<endl;

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.0f, 128.0f, 128.0f, 1.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 1280;
	int height = 720;
	
	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time;

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {

		// OpenGL Draw commands
		draw();

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();

		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			last_update_time = current_time;
		}
	}
	cout<<score<<endl;
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
