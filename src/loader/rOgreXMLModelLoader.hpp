#ifndef R_OGREXMLMODELLOADER_HPP
#define R_OGREXMLMODELLOADER_HPP

#include <fstream>
#include <sstream>

#include "rTypes.hpp"
#include "rDefs.hpp"

#include "xml/rXMLDocument.hpp"
#include "data/rModelData.hpp"
#include "loader/rImageLoader.hpp"

#include "rPath.hpp"

class rOgreXMLModelLoader {
public:
	rOgreXMLModelLoader();
	rOgreXMLModelLoader(rImageLoader* imageLoader);

public:

	int LoadModel(const rString& path, rModelData& modelData);
	void SetImageLoader(rImageLoader* imageLoader);

private:

	void ParseGeometry(rXMLDocument& document);
	static bool PositionElementToVector3(rXMLElement* element, rVector3& v);
	static bool TexCoordElementToVector2(rXMLElement* element, rVector2& v);

	void ParseSkeleton(const rString& skeletonFileName);
	void ParseBones(rXMLDocument& document);
	void ParseAnimations(rXMLDocument& document);

	rContentError ParseMaterials();
	void SetImportInfo(const rString& path);
	rString GetNextMeshName() const;

	void Clear();

	rModelData* m_modelData;
	rImageLoader* m_imageLoader;
	rString m_modelName;
	rString m_modelDir;
};

#endif