#include "Mesh.h"
#include "Shader.h"
// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

// GL Includes
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Mesh::Mesh(float vertices[])
{
	this->vertices.resize(36);
	memcpy(&(this->vertices[0]), vertices, 36 * 8 * sizeof(float));

	setUpMesh();
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;
	setUpMesh();
}

Mesh::~Mesh()
{
}

//void Mesh::Draw(Shader* shader)
//{
//	for (unsigned int i = 0; i < textures.size(); i++) {
//		if (textures[i].type == "texture_diffuse") {
//			glActiveTexture(GL_TEXTURE0);
//			glBindTexture(GL_TEXTURE_2D, textures[i].id);
//			// we suppose that one object only have one diffuse texture
//			glUniform1i(glGetUniformLocation(shader->Program, "material.diffuse"), 0);
//		}
//		else if (textures[i].type == "texture_specular") {
//			glActiveTexture(GL_TEXTURE1);
//			glBindTexture(GL_TEXTURE_2D, textures[i].id);
//			// we suppose that one object only have one specular texture
//			glUniform1i(glGetUniformLocation(shader->Program, "material.specular"), 1);
//		}
//	}
//	glBindVertexArray(VAO);
//	//glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
//	glDrawArrays(GL_TRIANGLES, 0, 99999);
//	glBindVertexArray(0);
//	glActiveTexture(GL_TEXTURE0);
//}

void Mesh::Draw(Shader* shader)
{
	// Bind appropriate textures
	GLuint diffuseNr = 1;
	GLuint specularNr = 1;
	for (GLuint i = 0; i < this->textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
		// Retrieve texture number (the N in diffuse_textureN)
		std::stringstream ss;
		std::string number;
		std::string name = this->textures[i].type;
		if (name == "texture_diffuse")
			ss << diffuseNr++; // Transfer GLuint to stream
		else if (name == "texture_specular")
			ss << specularNr++; // Transfer GLuint to stream
		number = ss.str();
		// Now set the sampler to the correct texture unit
		glUniform1i(glGetUniformLocation(shader->Program, (name + number).c_str()), i);
		// And finally bind the texture
		glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
	}

	// Also set each mesh's shininess property to a default value (if you want you could extend this to another mesh property and possibly change this value)
	glUniform1f(glGetUniformLocation(shader->Program, "material.shininess"), 16.0f);

	// Draw mesh
	glBindVertexArray(this->VAO);
	glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// Always good practice to set everything back to defaults once configured.
	for (GLuint i = 0; i < this->textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

//void Mesh::Draw(Shader* shader)
//{
//    // bind appropriate textures
//    unsigned int diffuseNr = 1;
//    unsigned int specularNr = 1;
//    unsigned int normalNr = 1;
//    unsigned int heightNr = 1;
//    for (unsigned int i = 0; i < textures.size(); i++)
//    {
//        glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
//        // retrieve texture number (the N in diffuse_textureN)
//        std::string number;
//        std::string name = textures[i].type;
//        if (name == "texture_diffuse")
//            number = std::to_string(diffuseNr++);
//        else if (name == "texture_specular")
//            number = std::to_string(specularNr++); // transfer unsigned int to string
//        else if (name == "texture_normal")
//            number = std::to_string(normalNr++); // transfer unsigned int to string
//        else if (name == "texture_height")
//            number = std::to_string(heightNr++); // transfer unsigned int to string
//
//        // now set the sampler to the correct texture unit
//        glUniform1i(glGetUniformLocation(shader->Program, (name + number).c_str()), i);
//        // and finally bind the texture
//        glBindTexture(GL_TEXTURE_2D, textures[i].id);
//    }
//
//    // draw mesh
//    glBindVertexArray(VAO);
//    //glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
//    glDrawArrays(GL_TRIANGLES, 0, 99999);
//    glBindVertexArray(0);
//
//    // always good practice to set everything back to defaults once configured.
//    glActiveTexture(GL_TEXTURE0);
//}

void Mesh::setUpMesh()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(GL_FLOAT)));

	glBindVertexArray(0);
}
