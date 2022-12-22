#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class LightDirectional
{
public:
	glm::vec3 direction; // the direction that light source points to
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	LightDirectional(glm::vec3 _direction, glm::vec3 _ambient, glm::vec3 _diffuse, glm::vec3 _specular);
	LightDirectional(glm::vec3 _direction, float _ambient, float _diffuse, float _specular);
};