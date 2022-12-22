#include "Material.h"

Material::Material(Shader* _shader, unsigned int _diffuse, unsigned int _specular, float _shininess) :
	shader(_shader),
	diffuse(_diffuse),
	specular(_specular),
	shininess(_shininess)
{

}

Material::~Material()
{
}
