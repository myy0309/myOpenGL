#include <iostream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include<SOIL2/SOIL2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other includes
#include "Shader.h"

int main() {
	// our goal is to translate vector (1,0,0) by (1,1,0) units
	glm::vec4 vec(1.0f, 0.0f, 0.0f, 1.0f); // define vector (1,0,0)
	glm::mat4 trans = glm::mat4(1.0f); // generate a 4x4 identity matrix
	trans = glm::translate(trans, glm::vec3(1.0f, 1.0f, 0.0f)); // generate the transform matrix
	vec = trans * vec; // translate the vector (1,0,0)
	std::cout << "( " << vec.x << ", " << vec.y << ", " << vec.z << " )" << std::endl;

	getchar();
	return 0;
}
