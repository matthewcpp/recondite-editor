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
		v.y = 1.0f - v.y; // convert ogre tex coords to opengl tex coords
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
					currentMaterial->SetParameter(rMATERIAL_PARAMETER_TEXTURE2D, "s_texture", name, name + ".rtex");
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
	rAlignedBox3& modelBoundingBox = m_modelData->GetBoundingBox();

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
		modelBoundingBox.AddPoint(position);
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

			geometryData.GetVertexPosition(v1, position);
			meshData->boundingBox.AddPoint(position);

			geometryData.GetVertexPosition(v2, position);
			meshData->boundingBox.AddPoint(position);

			geometryData.GetVertexPosition(v3, position);
			meshData->boundingBox.AddPoint(position);
		}
	}

	rXMLElement* boneassignments = document.GetRoot()->GetFirstChildNamed("boneassignments");

	unsigned short vertexindex, boneindex;
	float weight;
	for (size_t i = 0; i < boneassignments->NumChildren(); i++){
		rXMLElement* assignment = boneassignments->GetChild(i);
		assignment->GetAttribute<unsigned short>("vertexindex", vertexindex);
		assignment->GetAttribute<unsigned short>("boneindex", boneindex);
		assignment->GetAttribute<float>("weight", weight);

		geometryData.CreateVertexBoneLink(vertexindex,boneindex,weight);
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

		rXMLElement* skeletonLink = document.GetRoot()->GetFirstChildNamed("skeletonlink");

		if (skeletonLink){
			rString skeletonFileName;
			skeletonLink->GetAttribute<rString>("name", skeletonFileName);
			ParseSkeleton(skeletonFileName);
		}
	}

	Clear();

	return error;
}

void rOgreXMLModelLoader::ParseSkeleton(const rString& skeletonFileName){
	rString skeletonPath = rPath::Assemble(m_modelDir, skeletonFileName, ".xml");

	rXMLDocument document;
	rXMLReaderError error = document.LoadFromFile(skeletonPath);

	if (!error){
		m_modelData->CreateSkeleton();

		ParseBones(document);
		ParseAnimations(document);
	}
}

void rOgreXMLModelLoader::ParseBones(rXMLDocument& document){
	rXMLElement* bones = document.GetRoot()->GetFirstChildNamed("bones");
	rXMLElement* bonehierarchy = document.GetRoot()->GetFirstChildNamed("bonehierarchy");
	rSkeleton* skeleton = m_modelData->GetSkeleton();

	rXMLElement* boneNode = NULL;
	rString name, parent;
	rVector3 pos;

	for (size_t i = 0; i < bones->NumChildren(); i++){
		boneNode = bones->GetChild(i);
		boneNode->GetAttribute<rString>("name", name);
		PositionElementToVector3(boneNode->GetFirstChildNamed("position"), pos);

		rBone* bone = skeleton->CreateBone(name);

		bone->position = pos;
	}

	for (size_t i =0; i < bonehierarchy->NumChildren(); i++){
		rXMLElement* boneparent = bonehierarchy->GetChild(i);
		
		boneparent->GetAttribute<rString>("bone", name);
		boneparent->GetAttribute<rString>("parent", parent);

		rBone* childBone = skeleton->GetBone(name);
		rBone* parentBone = skeleton->GetBone(parent);

		if (childBone && parentBone){
			parentBone->AddChild(childBone);
		}
	}
}

void rOgreXMLModelLoader::ParseAnimations(rXMLDocument& document){
	rXMLElement* animations = document.GetRoot()->GetFirstChildNamed("animations");
	rSkeleton* skeleton = m_modelData->GetSkeleton();

	rString name;
	float time, angle;
	unsigned short boneId;
	rVector3 axis;

	for (size_t i = 0; i < animations->NumChildren(); i++){
		rXMLElement* animationNode = animations->GetChild(i);{
			animationNode->GetAttribute<rString>("name", name);
			animationNode->GetAttribute<float>("length", time);

			rAnimation* animation = skeleton->CreateAnimation(name);
			animation->SetDuration(time);

			rXMLElement* tracks = animationNode->GetFirstChildNamed("tracks");
			for (size_t t = 0; t < tracks->NumChildren(); t++){
				rXMLElement* trackNode = tracks->GetChild(t);
				trackNode->GetAttribute<unsigned short>("bone", boneId);

				rAnimationTrack* animationTrack = animation->CreateTrack(boneId);

				rXMLElement* keyframes = trackNode->GetFirstChildNamed("keyframes");
				animationTrack->AllocateFrames(keyframes->NumChildren());

				for (size_t k = 0; k < keyframes->NumChildren(); k++){
					rXMLElement* keyframeNode = keyframes->GetChild(k);
					rAnimationKeyframe* keyframe = animationTrack->GetKeyframe(k);

					rXMLElement* translate = keyframeNode->GetFirstChildNamed("translate");

					if (translate){
						translate->GetAttribute<float>("x", keyframe->translation.x);
						translate->GetAttribute<float>("y", keyframe->translation.y);
						translate->GetAttribute<float>("z", keyframe->translation.z);
					}

					rXMLElement* rotate = keyframeNode->GetFirstChildNamed("rotate");

					if (rotate){
						rotate->GetAttribute<float>("angle", angle);

						rXMLElement* axisNode = rotate->GetFirstChildNamed("axis");
						axisNode->GetAttribute<float>("x", axis.x);
						axisNode->GetAttribute<float>("y", axis.y);
						axisNode->GetAttribute<float>("z", axis.z);

						angle = rMath::RadToDeg(angle);

						keyframe->rotation.SetFromAxisAngle(axis, angle);
					}
				}
			}
		}
	}
}