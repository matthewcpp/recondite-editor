#include "rOgreXMLModelLoader.hpp"

#include <iostream>

rOgreXMLModelLoader::rOgreXMLModelLoader(){
	m_modelData = NULL;
	m_imageLoader = NULL;
}

rOgreXMLModelLoader::rOgreXMLModelLoader(rImageLoader* imageLoader){
	m_modelData = NULL;
	SetImageLoader(imageLoader);
}

void rOgreXMLModelLoader::SetImageLoader(rImageLoader* imageLoader){
	m_imageLoader = imageLoader;
}

void rOgreXMLModelLoader::SetImportInfo(const rString& path){
	m_modelDir = rPath::Directory(path);

	rString filename = rPath::Filename(path);
	m_modelName = filename.substr(0, filename.find_last_of("."));
}

void rOgreXMLModelLoader::Clear(){
	m_modelData = NULL;

	m_modelDir.clear();
	m_modelName.clear();
}

//todo make this more robust
bool rOgreXMLModelLoader::PositionElementToVector3(rXMLElement* element, rVector3& v){
	if (element){
		element->GetAttribute<float>("x", v.x);
		element->GetAttribute<float>("y", v.y);
		element->GetAttribute<float>("z", v.z);
		return true;
	}
	else{
		v.Set(0.0f, 0.0f, 0.0f);
		return false;
	}
}

bool rOgreXMLModelLoader::TexCoordElementToVector2(rXMLElement* element, rVector2& v){
	if (element){
		element->GetAttribute<float>("u", v.x);
		element->GetAttribute<float>("v", v.y);
		return true;
	}
	else{
		v.Set(0.0f, 0.0f);
		return false;
	}
}

rContentError rOgreXMLModelLoader::ParseMaterials(){
	rString materialPath = rPath::Assemble(m_modelDir, m_modelName, ".material");

	std::ifstream materialFile(materialPath.c_str());
	rMaterialData* currentMaterial = NULL;

	if (materialFile){
		rString line, name;
		while (std::getline(materialFile, line)){
			if (line.find("material ") != rString::npos){
				std::istringstream str(line);
				str >> name >> name;
				currentMaterial = m_modelData->CreateMaterialData(name);
				currentMaterial->SetShader("default_textured", "");
				currentMaterial->SetParameter(rMATERIAL_PARAMETER_COLOR, "fragColor", "255 255 255 255");
			}
			else if (line.find("texture ") != rString::npos){
				std::istringstream str(line);
				str >> name >> name;
				rString texturePath = rPath::Combine(m_modelDir, name);
				if (currentMaterial){
					currentMaterial->SetParameter(rMATERIAL_PARAMETER_TEXTURE2D, "s_texture", name, name);
				}

				if (m_imageLoader){
					rTexture2DData* textureData = m_modelData->CreateTextureData(name);
					if (textureData)
						m_imageLoader->LoadTexture(texturePath, *textureData);
				}
			}
		}

		return rCONTENT_ERROR_NONE;
	}
	else{
		return rCONTENT_ERROR_FILE_NOT_FOUND;
	}
}

void rOgreXMLModelLoader::ParseGeometry(rXMLDocument& document){
	size_t vertexCount;
	rString positions, normals, texCoords;
	rXMLElement* sharedgeometry = document.GetRoot()->GetFirstChildNamed("sharedgeometry");
	rXMLElement* vertexbuffer = sharedgeometry->GetFirstChildNamed("vertexbuffer");


	//for now assume these are present and accounted for...
	sharedgeometry->GetAttribute<size_t>("vertexcount", vertexCount);
	vertexbuffer->GetAttribute<rString>("positions", positions);
	vertexbuffer->GetAttribute<rString>("normals", normals);
	vertexbuffer->GetAttribute<rString>("texture_coords", texCoords);
	
	rGeometryData& geometryData = m_modelData->GetGeometryData();
	geometryData.Allocate(3, vertexCount,true,true);

	rXMLElement* vertex = NULL;
	rVector3 position, normal;
	rVector2 texCoord;

	for (size_t i = 0; i < vertexCount; i++){
		vertex = vertexbuffer->GetChild(i);
		PositionElementToVector3(vertex->GetFirstChildNamed("position"), position);
		PositionElementToVector3(vertex->GetFirstChildNamed("normal"), normal);
		TexCoordElementToVector2(vertex->GetFirstChildNamed("texcoord"), texCoord);

		geometryData.SetVertex(i, position, normal, texCoord);
	}
	

	rXMLElement* submeshes = document.GetRoot()->GetFirstChildNamed("submeshes");
	unsigned short v1, v2, v3;

	for (size_t i = 0; i < submeshes->NumChildren(); i++){
		rXMLElement* submesh = submeshes->GetChild(i);
		rXMLElement* faces = submesh->GetFirstChildNamed("faces");
		rXMLElement* face = NULL;

		rString material;
		submesh->GetAttribute<rString>("material", material);
		rString meshName = GetNextMeshName();

		rMeshData* meshData = m_modelData->CreateMeshData(meshName);
		meshData->material = material;
		meshData->buffer = meshName;

		rElementBufferData* elementBuffer = geometryData.CreateElementBuffer(meshName);
		for (size_t f = 0; f < faces->NumChildren(); f++){
			face = faces->GetChild(f);

			face->GetAttribute<unsigned short>("v1", v1);
			face->GetAttribute<unsigned short>("v2", v2);
			face->GetAttribute<unsigned short>("v3", v3);

			elementBuffer->Push(v1, v2, v3);
		}
	}
}

rString rOgreXMLModelLoader::GetNextMeshName() const{
	std::ostringstream str;
	str << m_modelName << "_mesh_" << m_modelData->MeshCount();

	return str.str();
}

int rOgreXMLModelLoader::LoadModel(const rString& path, rModelData& modelData){
	m_modelData = &modelData;
	m_modelData->Clear();

	SetImportInfo(path);

	rXMLDocument document;
	rXMLReaderError error = document.LoadFromFile(path);

	if (!error){
		m_modelData->SetName(m_modelName);
		ParseMaterials();
		ParseGeometry(document);
	}

	Clear();

	return error;
}