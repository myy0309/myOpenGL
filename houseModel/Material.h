#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"

class Material
{
public:
	Shader* shader;
	unsigned int diffuse;
	unsigned int specular;
	float shininess;

	Material(Shader* _shader, unsigned int _diffuse, unsigned int _specular, float _shininess);
	~Material();
};

