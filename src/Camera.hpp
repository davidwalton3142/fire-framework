#ifndef CAMERA_H
#define CAMERA_H

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <glew.h>

#include <iostream>

#include "Shader.hpp"

/* Camera Modes
 * FREELOOK: Camera rotates & moves relative to itself.
 * CENTRED: Camera rotates around (0,0,0).
 */
enum CameraModes : char { FREELOOK, CENTERED };

struct cameraBlock
{
	glm::mat4 worldToCamera;
	glm::vec3 cameraPos;
	glm::vec3 cameraDir;
};

/* Camera
 * Every Scene has a Camera object, which describes how world space is transformed into 
 * camera space. 
 * This consists of a rotation, translation and projection (applied in that order).
 */
class Camera
{
public:
	Camera();
	void translate(const glm::vec3& t);
	void fly(const glm::vec3& t);
	void setPos(const glm::vec3& _pos);
	void rotate(const float& theta, const float& phi);
	void setRot(const float& theta, const float& phi);

	void setFOV(const float& newFOV);
	void setAspect(const float& newAspect);
	void setZNear(const float& newZNear);
	void setZFar(const float& newZFar);

	void keyboardInput(unsigned char key, int x, int y);
	void mouseInput(int mouseX, int mouseY);

	cameraBlock& getBlock() {return block;};
private:
	CameraModes mode;
	glm::mat4 projection;
	glm::mat4 translation;
	glm::mat4 rotation;
	
	cameraBlock block;
	GLuint cameraBlock_ubo;

	float theta;
	float phi;
	float FOV; 
	float aspect; 
	float zNear;
	float zFar;
	int lastMouseX;
	int lastMouseY;
	void updateRotation();
	void updateBlock();
	void reset();
	static const float moveDelta;
	static const float rotDelta;
};

#endif
