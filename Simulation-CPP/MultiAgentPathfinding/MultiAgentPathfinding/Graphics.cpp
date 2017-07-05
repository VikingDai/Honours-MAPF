#include "Graphics.h"

#include <common/shader.hpp>
#include <iostream>

Graphics::Graphics()
{
	
}



Graphics::~Graphics()
{
}

bool Graphics::initGraphics()
{
	glGenVertexArrays(1, &vertexarray);
	glBindVertexArray(vertexarray);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("SimpleTransform.vertexshader", "SingleColor.fragmentshader");

	// Get a handle for our "MVP" uniform
	MVPID = glGetUniformLocation(programID, "MVP");

	// Get a handle for our "Model" uniform
	ModelID = glGetUniformLocation(programID, "Model");

	// Get a handle for our "Color" uniform
	ColorID = glGetUniformLocation(programID, "Color");

	static const GLfloat g_vertex_buffer_data[] = {
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		0.5f,  -0.5f, 0.0f,
	};

	// Create vertex buffer object (VBO)
	vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// Create a texture buffer object (TBO)
	texturebuffer;
	glGenTextures(1, &texturebuffer);
	glBindTexture(GL_TEXTURE_2D, texturebuffer);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 600, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Create render buffer object (RBO)
	renderbuffer;
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 600, 600);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

	// Create frame buffer object (FBO)
	framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

								   // Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texturebuffer, 0);

	// attach the renderbuffer to depth attachment point
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,	// 1. fbo target: GL_FRAMEBUFFER
		GL_DEPTH_ATTACHMENT,					// 2. attachment point
		GL_RENDERBUFFER,						// 3. rbo target: GL_RENDERBUFFER
		renderbuffer);							// 4. rbo ID

												// Check if the frame buffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		return false;
	}
}

void Graphics::CleanUp()
{
	glDeleteProgram(programID);

	// Cleanup vertex buffer VBO
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &vertexarray);

	// Cleanup texture buffer TBO
	glDeleteBuffers(1, &texturebuffer);
	glDeleteBuffers(1, &renderbuffer);
	glDeleteFramebuffers(1, &framebuffer);
}

void Graphics::DrawSquare()
{
	// Use our shader
	glUseProgram(programID);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*) 0            // array buffer offset
	);

	// Draw the triangle strip!
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // 3 indices starting at 0 -> 1 triangle

	glDisableVertexAttribArray(0);
}
