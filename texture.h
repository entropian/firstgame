#pragma once
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class Texture
{
public:
    Texture()
        :width(0), height(0), comp(0), data(nullptr)
    {}
    
    Texture(const char *file_name)
    {
        data = stbi_load(file_name, &width, &height, &comp, STBI_rgb);
    }

    void loadTexFile(const char *file_name)
    {
        if(data)
        {
            free(data);
        }
        data = stbi_load(file_name, &width, &height, &comp, STBI_rgb);
    }

    ~Texture()
    {
        if(data)
        {
            free(data);
        }
        width = height = comp = 0;
    }

    int width;
    int height;
    int comp;
    unsigned char *data;
};
