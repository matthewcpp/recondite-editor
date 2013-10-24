#include "rSimpleImageLoader.hpp"

int rSimpleImageLoader::LoadTexture(const rString& path, rTexture2DData& textureData){
	textureData.Clear();

	int width, height, bpp;
	unsigned char *data = stbi_load(path.c_str(), &width, &height, &bpp, 0);

	if (data){

		textureData.SetImageData(width, height, bpp, data);

		stbi_image_free(data);
		return 0;
	}
	else{
		return 1;
	}
}