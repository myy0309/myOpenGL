#include "LightPoint.h"

LightPoint::LightPoint(glm::vec3 _position, glm::vec3 _ambient, glm::vec3 _diffuse, glm::vec3 _specular, float _constant, float _linear, float _quadratic):
	position(_position),
	ambient(_ambient),
	diffuse(_diffuse),
	specular(_specular),
	constant(_constant),
	linear(_linear),
	quadratic(_quadratic)
{
}

LightPoint::LightPoint(glm::vec3 _position, float _ambient, float _diffuse, float _specular, float _constant, float _linear, float _quadratic):
	position(_position),
	ambient(_ambient, _ambient, _ambient),
	diffuse(_diffuse, _diffuse, _diffuse),
	specular(_specular, _specular, _specular),
	constant(_constant),
	linear(_linear),
	quadratic(_quadratic)
{
}
