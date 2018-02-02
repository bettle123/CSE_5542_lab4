#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>
#include <GL/glm/glm.hpp>
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtx/transform2.hpp>
#include <GL/glm/gtc/type_ptr.hpp>


using namespace std;
using glm::mat4;
using glm::vec3;
// Vertex Array Object for Cube
GLuint cube_vao;
GLuint program;
GLuint windowWidth = 600;
GLuint windowHeight = 600;
GLint rotate_angle = 45;
//GLint rotate_angle = 0;
GLuint frameBuffer;
// transfer function
GLuint transferFun_TextureObj;
GLuint backface_TextureObj;
GLuint textureWidth;
GLuint textureHeight;
GLuint volumetextureObj;
GLuint raycastingVertexShader;
GLuint raycastingFragmentShader;
GLuint backfaceVertexShader;
GLuint backfaceFragmentShader;
float stepSize = 0.001f;



void keyboard(unsigned char key, int x, int y);
void display(void);
void createVBO();
void createshader();
void createFrameBuffer(GLuint, GLuint, GLuint);
GLuint createTFF1DTexture(const char* filename);
GLuint createFace2DTexture(GLuint bftextureWidth, GLuint bftextureHeight);
GLuint createVol3DTexture(const char* filename, GLuint width, GLuint height, GLuint depth);
void render(GLenum cullFace);
void init()
{
	textureWidth = windowWidth;
	textureHeight = windowHeight;
	createVBO();
	createshader();
	transferFun_TextureObj = createTFF1DTexture("..\\hw3\\data\\tff.dat");
	backface_TextureObj = createFace2DTexture(textureWidth, textureHeight);
	//volumetextureObj = createVol3DTexture("..\\hw3\\data\\cube32.vol", 32, 32, 32);
	//volumetextureObj = createVol3DTexture("..\\hw3\\data\\uncBrain.vol", 256, 256, 145);
	volumetextureObj = createVol3DTexture("..\\hw3\\data\\statueLeg.raw", 341, 341, 93);
	createFrameBuffer(backface_TextureObj, textureWidth, textureHeight);
}
// create vertex buffer object
void createVBO()
{	
	GLfloat positions[6 * 4 * 3] = {
		// Front face
		0.0, 0.0, 1.0,
		1.0, 0.0, 1.0,
		1.0, 1.0, 1.0,
		0.0, 1.0, 1.0,
		// Back face
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		1.0, 1.0, 0.0,
		1.0, 0.0, 0.0,
		// Top face
		0.0, 1.0, 0.0,
		0.0, 1.0, 1.0,
		1.0, 1.0, 1.0,
		1.0, 1.0, 0.0,
		// Bottom face
		0.0, 0.0, 0.0,
		1.0, 0.0, 0.0,
		1.0, 0.0, 1.0,
		0.0, 0.0, 1.0,
		// Right face
		1.0, 0.0, 0.0,
		1.0, 1.0, 0.0,
		1.0, 1.0, 1.0,
		1.0, 0.0, 1.0,
		// Left face
		0.0, 0.0, 0.0,
		0.0, 0.0, 1.0,
		0.0, 1.0, 1.0,
		0.0, 1.0, 0.0,
	};
	GLuint indices[36] = {
		0, 1, 2, 0, 2, 3,    // front
		4, 5, 6, 4, 6, 7,    // back
		8, 9, 10, 8, 10, 11,   // top
		12, 13, 14, 12, 14, 15,   // bottom
		16, 17, 18, 16, 18, 19,   // right
		20, 21, 22, 20, 22, 23,   // left
	};
	unsigned int vaoCube, vboVertices, vboIndices;
	// Create a Vertex Array Object to organize all of the bindings.
	glGenVertexArrays(1, &vaoCube);
	glBindVertexArray(vaoCube);
	// Vertex Position
	glGenBuffers(1, &vboVertices);
	glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
	glBufferData(GL_ARRAY_BUFFER, 6 * 4 * 3 * sizeof(GLfloat), positions, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// Vertex Indices
	glGenBuffers(1, &vboIndices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * 2 * 3 * sizeof(GLuint), indices, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
	cube_vao = vaoCube;
	
}
void DrawCube(GLenum glFaces)
{
	glEnable(GL_CULL_FACE);
	glCullFace(glFaces);
	glBindVertexArray(cube_vao);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLuint *)NULL);
	glDisable(GL_CULL_FACE);
}
// check the compilation result
GLboolean compileCheck(GLuint shader)
{
	GLint error;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &error);
	if (GL_FALSE == error)
	{
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0)
		{
			char* log = (char *)malloc(logLen);
			GLsizei written;
			glGetShaderInfoLog(shader, logLen, &written, log);
			cerr << "Shader log: " << log << endl;
			free(log);
		}
	}
	return error;
}
// create shader object
GLuint createshaderObj(const GLchar* filename, GLenum shaderType)
{
	ifstream inFile(filename, ifstream::in);
	if (!inFile)
	{
		cerr << "Error openning file: " << filename << endl;
		exit(EXIT_FAILURE);
	}

	const int MAX_CNT = 10000;
	GLchar *shaderCode = (GLchar *)calloc(MAX_CNT, sizeof(GLchar));
	inFile.read(shaderCode, MAX_CNT);
	if (inFile.eof())
	{
		size_t bytecnt = inFile.gcount();
		*(shaderCode + bytecnt) = '\0';
	}
	else
	{
		cout << "read "<<filename <<" failed." << endl;
	}
	// create the shader Object
	GLuint shader = glCreateShader(shaderType);
	if (0 == shader)
	{
		cerr << "can't create shader." << endl;
	}
	const GLchar* codeArray[] = { shaderCode };
	glShaderSource(shader, 1, codeArray, NULL);
	free(shaderCode);

	// compile the shader
	glCompileShader(shader);
	if (GL_FALSE == compileCheck(shader))
	{
		cerr << "shader compilation failed" << endl;
	}
	return shader;
}
GLint checkShaderLinkStatus(GLuint pgmHandle)
{
	GLint status;
	glGetProgramiv(pgmHandle, GL_LINK_STATUS, &status);
	if (GL_FALSE == status)
	{
		GLint logLen;
		glGetProgramiv(pgmHandle, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0)
		{
			GLchar * log = (GLchar *)malloc(logLen);
			GLsizei written;
			glGetProgramInfoLog(pgmHandle, logLen, &written, log);
			cerr << "Program log: " << log << endl;
		}
	}
	return status;
}

GLuint createShaderProgram()
{
	// Create the shader program
	GLuint shaderprogram = glCreateProgram();
	if (0 == shaderprogram)
	{
		cerr << "Error create shader program" << endl;
		exit(EXIT_FAILURE);
	}
	return shaderprogram;
}


// create the 1 dimentional texture for transfer function
GLuint createTFF1DTexture(const char* filename)
{
	// read in the user defined data of transfer function
	ifstream inFile(filename, ifstream::in);
	if (!inFile)
	{
		cerr << "Error openning file: " << filename << endl;
		exit(EXIT_FAILURE);
	}

	const int MAX_CNT = 10000;
	GLubyte *tff = (GLubyte *)calloc(MAX_CNT, sizeof(GLubyte));
	inFile.read(reinterpret_cast<char *>(tff), MAX_CNT);
	if (inFile.eof())
	{
		size_t bytecnt = inFile.gcount();
		*(tff + bytecnt) = '\0';
		cout << "bytecnt " << bytecnt << endl;
	}
	else
	{
		cout << filename << " read failed " << endl;
	}
	GLuint tff1DTexture;
	glGenTextures(1, &tff1DTexture);
	glBindTexture(GL_TEXTURE_1D, tff1DTexture);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, tff);
	free(tff);
	return tff1DTexture;
}
// craete the 2D texture for render backface 'bf' stands for backface
GLuint createFace2DTexture(GLuint bftextureWidth, GLuint bftextureHeight)
{
	GLuint backFace2DTexture;
	glGenTextures(1, &backFace2DTexture);
	glBindTexture(GL_TEXTURE_2D, backFace2DTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, bftextureWidth, bftextureHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	return backFace2DTexture;
}
// create 3D texture to store the volume data used fo ray casting
GLuint createVol3DTexture(const char* filename, GLuint w, GLuint h, GLuint d)
{
	
	FILE *fp;
	size_t size = w * h * d;
	GLubyte *data = new GLubyte[size];			  // 8bit
	if (!(fp = fopen(filename, "rb")))
	{
		cout << "Error: opening .raw file failed" << endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		cout << "OK: open .raw file successed" << endl;
	}
	if (fread(data, sizeof(char), size, fp) != size)
	{
		cout << "Error: read .raw file failed" << endl;
		exit(1);
	}
	else
	{
		cout << "OK: read .raw file successed" << endl;
	}
	fclose(fp);
	glGenTextures(1, &volumetextureObj);
	// bind 3D texture target
	glBindTexture(GL_TEXTURE_3D, volumetextureObj);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY, w, h, d, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);

	delete[]data;
	cout << "volume texture created" << endl;
	return volumetextureObj;
	
}


void checkFramebufferStatus()
{
	GLenum complete = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (complete != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "framebuffer is not complete" << endl;
		exit(EXIT_FAILURE);
	}
}
// create the framebuffer
void createFrameBuffer(GLuint texObj, GLuint textureWidth, GLuint texHeight)
{
	// create a depth buffer for our framebuffer
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, textureWidth, texHeight);

	// attach the texture and the depth buffer to the framebuffer
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texObj, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	checkFramebufferStatus();
	glEnable(GL_DEPTH_TEST);
}

void raycastingcSetUinforms()
{
	// setting those uniforms into shader as follows.
	// sampler2D ExitPoints; (the backface hold the ExitPoints of ray casting)
	// sampler3D VolumeTex; (the texture that hold the volume data i.e. head256.raw)
	// sampler1D TransferFunc;
	// float StepSize;
	// vec2 ScreenSize;
	GLint screenSizeLoc = glGetUniformLocation(program, "ScreenSize");
	if (screenSizeLoc >= 0)
	{
		glUniform2f(screenSizeLoc, (float)windowWidth, (float)windowHeight);
	}
	else
	{
		cout << "ScreenSize is not bind to the uniform" << endl;
	}
	GLint stepSizeLoc = glGetUniformLocation(program, "StepSize");

	if (stepSizeLoc >= 0)
	{
		glUniform1f(stepSizeLoc, stepSize);
	}
	else
	{
		cout << "StepSize is not bind to the uniform" << endl;
	}
	GLint transferFuncLoc = glGetUniformLocation(program, "TransferFunc");
	if (transferFuncLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_1D, transferFun_TextureObj);
		glUniform1i(transferFuncLoc, 0);
	}
	else
	{
		cout << "TransferFunc is not bind to the uniform" << endl;
	}
	GLint backFaceLoc = glGetUniformLocation(program, "ExitPoints");
	if (backFaceLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, backface_TextureObj);
		glUniform1i(backFaceLoc, 1);
	}
	else
	{
		cout << "ExitPoints is not bind to the uniform" << endl;
	}
	GLint volumeLoc = glGetUniformLocation(program, "VolumeTex");
	if (volumeLoc >= 0)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_3D, volumetextureObj);
		glUniform1i(volumeLoc, 2);
	}
	else
	{
		cout << "VolumeTex is not bind to the uniform"<< endl;
	}

}
// create the shader and shaderprogram
void createshader()
{
	// vs and fs for first rendering
	backfaceVertexShader = createshaderObj("..\\hw3\\shader\\backface.vert", GL_VERTEX_SHADER);
	backfaceFragmentShader = createshaderObj("..\\hw3\\shader\\backface.frag", GL_FRAGMENT_SHADER);
	// vs and fs for second rendering
	raycastingVertexShader = createshaderObj("..\\hw3\\shader\\raycasting.vert", GL_VERTEX_SHADER);
	raycastingFragmentShader = createshaderObj("..\\hw3\\shader\\raycasting.frag", GL_FRAGMENT_SHADER);
	program = createShaderProgram();
}

// link the shader objects using the shader program
void linkShader(GLuint shaderProgram, GLuint VertexShader, GLuint FragmentShader)
{
	const GLsizei maxCount = 2;
	GLsizei count;
	GLuint shaders[maxCount];
	glGetAttachedShaders(shaderProgram, maxCount, &count, shaders);
	for (int i = 0; i < count; i++) {
		glDetachShader(shaderProgram, shaders[i]);
	}
	glBindAttribLocation(shaderProgram, 0, "position");
	glBindAttribLocation(shaderProgram, 1, "baseColor");
	glAttachShader(shaderProgram, VertexShader);
	glAttachShader(shaderProgram, FragmentShader);
	glLinkProgram(shaderProgram);
	if (GL_FALSE == checkShaderLinkStatus(shaderProgram))
	{
		cerr << "Failed to relink shader program!" << endl;
		exit(EXIT_FAILURE);
	}
}

// the color of the vertex in the back face is also the location
// of the vertex
// save the back face to the user defined framebuffer bound
// with a 2D texture named "backface_TextureObj"
// draw the front face of the box
// in the rendering process, i.e. the ray marching process
// loading the volume "volumetextureObj" as well as the "backface_TextureObj"
// after vertex shader processing we got the color as well as the location of
// the vertex (in the object coordinates, before transformation).
// and the vertex assemblied into primitives before entering
// fragment shader processing stage.
// in fragment shader processing stage. we got "backface_TextureObj"
// (correspond to "VolumeTex" in glsl)and "volumetextureObj"(correspond to "ExitPoints")
// as well as the location of primitives.
// draw the back face of the box
void display()
{
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
	glViewport(0, 0, windowWidth, windowHeight);
	linkShader(program, backfaceVertexShader, backfaceFragmentShader);
	glUseProgram(program);
	// cull front face
	render(GL_FRONT);
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
	linkShader(program, raycastingVertexShader, raycastingFragmentShader);
	glUseProgram(program);
	raycastingcSetUinforms();
	render(GL_BACK);
	glUseProgram(0);
	glutSwapBuffers();
}
// both of the two pass use the "render() function"
// the first pass render the backface of the cube
// the second pass render the frontface of the cube
// together with the frontface, use the backface as a 2D texture in the second pass
// to calculate the entry point and the exit point of the ray in and out the cube.
void render(GLenum cullFace)
{
	glClearColor(1.0f, 0.4f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//  set up modelview and projection matrix
	glm::mat4 projectionmatrix = glm::perspective(60.0f, (GLfloat)windowWidth / windowHeight, 0.1f, 400.f);
	glm::mat4 viewmatrix = glm::lookAt(glm::vec3(0.0f, 1.0f, 2.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 modelmatrix = mat4(1.0f);
	modelmatrix *= glm::rotate((float)rotate_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	modelmatrix *= glm::rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
	modelmatrix *= glm::translate(glm::vec3(-0.5f, -0.5f, -0.5f));
	glm::mat4 mvp = projectionmatrix * viewmatrix * modelmatrix;
	GLuint mvpIdx = glGetUniformLocation(program, "MVP");
	glUniformMatrix4fv(mvpIdx, 1, GL_FALSE, &mvp[0][0]);
	DrawCube(cullFace);
}
void rotateDisplay()
{
	rotate_angle = (rotate_angle + 10) % 360;
	glutPostRedisplay();
}
void reshape(int w, int h)
{
	windowWidth = w;
	windowHeight = h;
	textureWidth = w;
	textureHeight = h;
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{	
		case 'x':
			rotate_angle = 0;
			break;
	}
}

int main(int argc, char** argv)
{
	cout << "keyboard x can reset the rotate position." << endl;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Su_ExtraLab_VolumeRendering");
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}

	glutKeyboardFunc(&keyboard);
	glutDisplayFunc(&display);
	glutReshapeFunc(&reshape);
	glutIdleFunc(&rotateDisplay);
	init();
	glutMainLoop();
	return EXIT_SUCCESS;
}

