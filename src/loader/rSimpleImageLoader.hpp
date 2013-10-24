#ifndef R_SIMPLEIMAGELOADER_HPP
#define R_SIMPLEIMAGELOADER_HPP

// uses public domain image loader: http://nothings.org/stb_image.c 

#include "rTypes.hpp"
#include "rDefs.hpp"

#include "rImageLoader.hpp"
#include "data/rTexture2DData.hpp"

#include"stb_image.h"

class rSimpleImageLoader : public rImageLoader{
public:
	virtual int LoadTexture(const rString& path, rTexture2DData& textureData);
};

#endif