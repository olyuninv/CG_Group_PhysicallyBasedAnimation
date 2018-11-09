#define _CRT_SECURE_NO_WARNINGS

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp> 
#include <glm/gtc/constants.hpp> 

#include "ParticleSimulation.h"

// OpenGL 
// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
GLuint shaderProgramID;
GLuint vbo = 0;
float last_time, present_time;

// Initial window size
int width = 1000;
int height = 800;

// Particles
const int MAX_NUM_PARTICLES = 1000;
const int NUM_COLORS = 8;

particle particles[MAX_NUM_PARTICLES];
float speed = 1;

glm::vec4 colors[8] = { glm::vec4(0.0, 0.0, 0.0, 1.0), glm::vec4(1.0,0.0,0.0,1.0),
glm::vec4(1.0, 1.0, 0.0, 1.0), glm::vec4(0.0,1.0,0.0,1.0),
glm::vec4(0.0, 0.0, 1.0, 1.0), glm::vec4(1.0,0.0,1.0,1.0),
glm::vec4(0.0, 1.0, 1.0, 1.0), glm::vec4(1.0,1.0,1.0,1.0) };

// Physics
bool gravity = TRUE;
float coef = 0.8f;  // coefficient of restitution

float forces(int i, int j)
{
	if (!gravity)
		return (0.0);
	else if (j == 1)
		return (-1.0);
	else
		return (0.0);
}

void collision(int n)
{
	for (int i = 0; i < 3; i++)
	{
		if (particles[n].position[i] >= 1.0)
		{
			particles[n].velocity[i] = -coef * particles[n].velocity[i];
			particles[n].position[i] = 1.0 - coef * (particles[n].position[i] - 1.0);
		}

		if (particles[n].position[i] <= -1.0)
		{
			particles[n].velocity[i] = -coef * particles[n].velocity[i];
			particles[n].position[i] = -1.0 - coef * (particles[n].position[i] + 1.0);
		}
	}
}

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS

// Create a NULL-terminated string by reading the provided file
char* readShaderSource(const char* shaderFile) {
	FILE* fp = fopen(shaderFile, "rb"); //!->Why does binary flag "RB" work and not "R"... wierd msvc thing?

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, "../PhysicallyBasedAnimation/Shaders/simpleVertexShader.txt", GL_VERTEX_SHADER);
	AddShader(shaderProgramID, "../PhysicallyBasedAnimation/Shaders/simpleFragmentShader.txt", GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

void updateScene() 
{
	int i, j;
	float dt;
	present_time = glutGet(GLUT_ELAPSED_TIME); // in ms
	dt = 0.001 * (present_time - last_time); // in sec

	for (int i = 0; i < MAX_NUM_PARTICLES; i++)
	{
		for (j = 0; j < 3; j++)
		{
			particles[i].position[j] += dt * particles[i].velocity[j];
			particles[i].velocity[j] += dt * forces(i, j) / particles[i].mass;
		}

		collision(i);
	}

	last_time = present_time;
	glutPostRedisplay();
}

void keypress(unsigned char key, int x, int y)
{

}

void specialKeys(int key, int x, int y)
{

}

void createParticles(int num_particles)
{
	for (int i = 0; i<num_particles; i++)
	{
		particles[i].mass = 1.0;
		particles[i].color = i%NUM_COLORS;
		for (int j = 0; j < 3; j++)
		{
			particles[i].position[j] = 2.0 * ((float)rand() / RAND_MAX) - 1.0;
			particles[i].velocity[j] = speed * 2.0 * ((float)rand() / RAND_MAX) - 1.0;
		}
		particles[i].position[3] = 1.0;
		particles[i].velocity[3] = 0.0;
	}
}

void createUniformVariables()
{

}

void linkBuffer()
{
	glm::vec4 point_colors[MAX_NUM_PARTICLES];
	glm::vec4 points[MAX_NUM_PARTICLES];

	for (int i = 0; i < MAX_NUM_PARTICLES; i++)
	{
		point_colors[i] = colors[particles[i].color];
		points[i] = particles[i].position;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(point_colors), NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(point_colors), point_colors);	
}

void init()
{
	// Set up the shaders
	shaderProgramID = CompileShaders();

	// create objects
	int num_particles = MAX_NUM_PARTICLES;
	createParticles(num_particles);

	// create Uniform variables that do not change
	createUniformVariables();
		
	// Object buffer		
	glGenBuffers(1, &vbo);	
	linkBuffer();

	GLuint positionID = glGetAttribLocation(shaderProgramID, "vPosition");
	GLuint colorID = glGetAttribLocation(shaderProgramID, "vColor");

	// Have to enable this
	glEnableVertexAttribArray(positionID);
	// Tell it where to find the position data in the currently active buffer (at index positionID)
	glVertexAttribPointer(positionID, 4, GL_FLOAT, GL_FALSE, 0, 0);

	// Similarly, for the color data.
	glEnableVertexAttribArray(colorID);
	glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(num_particles * 4 * sizeof(float)));
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);
	// NB: Make the call to draw the geometry in the currently activated vertex buffer. This is where the GPU starts to work!	

	linkBuffer();

	glDrawArrays(GL_POINTS, 24, MAX_NUM_PARTICLES);

	glutSwapBuffers();
}


int initOpenGL(int argc, char** argv)
{
	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Physically Based Animation");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);
	glutSpecialFunc(specialKeys);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	return 0;
}

int main(int argc, char** argv) {

	if (initOpenGL(argc, argv) != 0)
		return 1;
	
	init();

	// Begin infinite event loop
	glutMainLoop();
	return 0;
}











