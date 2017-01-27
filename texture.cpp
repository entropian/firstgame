#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(std::string file_name)
{
    data = stbi_load(file_name.c_str(), &width, &height, &comp, STBI_rgb);
}
