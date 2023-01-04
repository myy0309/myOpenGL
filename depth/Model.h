#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Shader.h"

class Model
{
	public:
		Model(std::string path);
		~Model();
		std::vector<Mesh> meshes;
		std::string directory;
		void Draw(Shader* shader);
	private:
		//std::string directory;
		std::vector<Texture> textures_loaded;
		void loadModel(std::string path);
		void processNode(aiNode* node, const aiScene* scene);
		Mesh processMesh(aiMesh* mesh, const aiScene* scene);
		Mesh processMesh(aiMesh* mesh, const aiScene* scene, std::vector<unsigned int>* tempIndices);
		std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
		GLint TextureFromFile(const char* path, std::string directory);
		unsigned int TextureFromFile1(const char* path, const std::string& directory);
};