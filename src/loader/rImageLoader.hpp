#ifndef R_IMAGELOADER_HPP
#define R_IMAGELOADER_HPP

#include "rTypes.hpp"
#include "rDefs.hpp"

#include "data/rTexture2DData.hpp"

class rImageLoader{
public:
	virtual int LoadTexture(const rString& path, rTexture2DData& textureData) = 0;
};

#endif