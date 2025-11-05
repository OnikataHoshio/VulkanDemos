#include "Engine/Object/Model.h"

namespace HoshioEngine {
	Model::Model(const char* file_path)
	{
		LoadModel(file_path);
	}

	void Model::Render(ShaderInfo& shader_info)
	{
		Pipeline& pipeline = VulkanPlus::Plus().GetPipeline(shader_info.pipeline_id).second[0];
		const CommandBuffer& commandBuffer = VulkanPlus::Plus().CommandBuffer_Graphics();
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		for (auto& mesh : meshes)
			mesh.Render(shader_info);
	}

	void Model::SetupModel(ShaderInfo& shader_info)
	{
		for (auto& mesh : meshes)
			mesh.SetupMesh(shader_info);
	}

	void Model::LoadModel(std::string path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cout << std::format("[ ERROR ] ASSIMP::{}\n", importer.GetErrorString());
			throw std::runtime_error(std::format("[ ERROR ] ASSIMP::{}", importer.GetErrorString()));
			return;
		}

		directory = path.substr(0, path.find_last_of('/'));
		CheckModelImportType(path);


		ProcessNode(scene->mRootNode, scene);

		std::cout << std::format("Successfully load the model : {}\n", path);

	}

	void Model::CheckModelImportType(std::string& path)
	{
		std::string extension;
		size_t pos = path.find_last_of('.');
		if (pos == std::string::npos)
			extension = "";
		else
			extension = path.substr(pos + 1);

		if (extension == "obj")
			model_import_type = MODEL_IMPORT_TYPE::MODLE_TYPE_OBJ;
		else 
			model_import_type = MODEL_IMPORT_TYPE::MODLE_TYPE_ERR;

	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene)
	{
		// 处理节点所有的网格（如果有的话）
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.emplace_back(ProcessMesh(mesh, scene));
		}
		// 接下来对它的子节点重复这一过程
		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}

	Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<TextureInfo> textures;

		for (uint32_t i = 0; i < mesh->mNumVertices; i++) {

			Vertex vertex;

			glm::vec3 vector;
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.SetPosition(vector);

			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.SetNormal(vector);

			uint32_t index = 0;
			glm::vec2 uv;
			while (mesh->HasTextureCoords(index)) {
				uv.y = mesh->mTextureCoords[index][i].y;
				uv.x = mesh->mTextureCoords[index][i].x;
				vertex.AddUV(uv);
				index += 1;
			}

			index = 0;
			glm::vec4 color;
			while (mesh->HasVertexColors(index)) {
				color.x = mesh->mColors[index][i].r;
				color.y = mesh->mColors[index][i].g;
				color.z = mesh->mColors[index][i].b;
				color.w = mesh->mColors[index][i].a;
				vertex.AddColor(color);
				index += 1;
			}

			vertices.push_back(vertex);
		}

		for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		if (mesh->mMaterialIndex >= 0) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			std::vector<TextureInfo> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, TEXTURE_TYPE::DIFFUSE);
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			std::vector<TextureInfo> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, TEXTURE_TYPE::SPECULAR);
			textures.insert(textures.end(),specularMaps.begin(), specularMaps.end());

			std::vector<TextureInfo> normalMaps;
			switch (model_import_type)
			{
			case MODEL_IMPORT_TYPE::MODLE_TYPE_OBJ:
				normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, TEXTURE_TYPE::NORMAL);
				break;
			case MODEL_IMPORT_TYPE::MODLE_TYPE_ERR:
				std::cout << std::format("Unknown Model type!\n");
				break;
			default:
				normalMaps = LoadMaterialTextures(material, aiTextureType_NORMALS, TEXTURE_TYPE::NORMAL);
				std::cout << std::format("New model import type should be added!\n");
				break;
			}

			textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		}

		std::cout << std::format("Successfully load the mesh : {}\n",mesh->mName.C_Str());

		return Mesh(vertices, indices, textures);
	}

	std::vector<VertexInputAttribute> Model::GetVertexInputeAttributes()
	{
		if (meshes.empty())
			return std::vector<VertexInputAttribute>();
		return meshes[0].GetVertexInputAttributes();
	}

	uint32_t Model::GetVertexInputAttributesStride()
	{
		if (meshes.empty())
			return 0;
		return meshes[0].GetVertexInputAttributeStride();
	}

	std::vector<TextureInfo> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, TEXTURE_TYPE type_enum)
	{
		std::vector<TextureInfo> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			TextureInfo texture;
			std::string texture_path = directory + "/" + std::string(str.C_Str());
			if (VulkanPlus::Plus().HasTexture2D(std::string(str.C_Str())))
				texture.id = VulkanPlus::Plus().GetTexture2D(std::string(str.C_Str())).first;
			else
				texture.id = VulkanPlus::Plus().CreateTexture2D(std::string(str.C_Str()), texture_path.c_str(), VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM).first;
			texture.type = type_enum;
			textures.push_back(texture);
		}
		return textures;
	}
}