#include "LightDirectional.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

LightDirectional::LightDirectional(glm::vec3 _direction, glm::vec3 _ambient, glm::vec3 _diffuse, glm::vec3 _specular):
	direction(_direction),
	ambient(_ambient),
	diffuse(_diffuse),
	specular(_specular)
{
}

LightDirectional::LightDirectional(glm::vec3 _direction, float _ambient, float _diffuse, float _specular):
	direction(_direction),
	ambient(_ambient, _ambient, _ambient),
	diffuse(_diffuse, _diffuse, _diffuse),
	specular(_specular, _specular, _specular)
{
}
