#ifndef _MODEL_H_
#define _MODEL_H_

#include "Mesh.h"
#include "Actor.h"

namespace HoshioEngine {
	enum class MODEL_IMPORT_TYPE {
		MODLE_TYPE_OBJ,
		MODLE_TYPE_ERR
	};

	class Model : public Actor {
	public:
		Model() = default;
		Model(const char* file_path);
		Model(std::vector<Mesh>& meshes);
		void LoadModel(std::string path);
		void LoadModel(std::vector<Mesh>& meshes);
		std::vector<VertexInputAttribute> GetVertexInputeAttributes();
		uint32_t GetVertexInputAttributesStride();
		void Render(ShaderInfo& shader_info);
		void SetupModel(ShaderInfo& shader_info);
	private:
		std::vector<Mesh> meshes;
		std::string directory = "";
		MODEL_IMPORT_TYPE model_import_type = MODEL_IMPORT_TYPE::MODLE_TYPE_OBJ;

		void CheckModelImportType(std::string& path);
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<TextureInfo> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, TEXTURE_TYPE type_enum);
	};
}

#endif // !_MODEL_H_

