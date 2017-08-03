#include "Camera.h"


void Camera::updateMVP()
{
	Projection = 
		glm::ortho(-zoom, zoom, -zoom, zoom, 0.0f, 100.0f) * 
		glm::scale(mat4(1.f), vec3(9.0f / 16.0f, 1.f, 1.f));

	// Camera matrix
	View = glm::lookAt(position, position - vec3(0, 0, 1), vec3(0, 1, 0));

	// Model matrix : an identity matrix (model will be at the origin)
	//Model = glm::mat4(1.0f);

	// Our ModelViewProjection : multiplication of our 3 matrices
	MVP = Projection * View; //* Model; // Remember, matrix multiplication is the other way around
}

Camera::Camera()
{
	position = vec3(0, 0, 1);
	zoom = 50.f;
	updateMVP();
}

void Camera::update(float deltaTime)
{
	updateMVP();
}
