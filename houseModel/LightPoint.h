#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class LightPoint
{
public:
	glm::vec3 position;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float constant;
	float linear;
	float quadratic;

	LightPoint(glm::vec3 _position, glm::vec3 _ambient, glm::vec3 _diffuse, glm::vec3 _specular, float _constant = 1.0f, float _linear = 0.09f, float _quadratic = 0.032f);
	LightPoint(glm::vec3 _position, float _ambient, float _diffuse, float _specular, float _constant = 1.0f, float _linear = 0.09f, float _quadratic = 0.032f);
};

