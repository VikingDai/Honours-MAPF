#pragma once

#include <GL/glew.h>

class Graphics
{
public:
	// Create and compile our GLSL program from the shaders
	GLuint programID;
	GLuint MVPID;
	GLuint ModelID;
	GLuint ColorID;

	
	GLuint vertexbuffer;
	GLuint texturebuffer;
	GLuint renderbuffer;
	GLuint framebuffer;

	GLuint vertexarray;

public:
	Graphics();
	~Graphics();
	bool initGraphics();
	void CleanUp();
	void DrawSquare();
};

