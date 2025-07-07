#pragma once
#include <GL/glut.h>
#include "stb_image.h"
#include <iostream>

GLuint loadTexture(const char* filename) {
    std::cout << "Loading texture: " << filename << std::endl;
    int width, height, channels;
    // Force load 4 channels (RGBA) for compatibility
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load image: " << filename << "\n"
                  << "stb_image error: " << stbi_failure_reason() << std::endl;
        return 0;
    }
    std::cout << "Loaded: " << width << "x" << height << " pixels, channels forced to RGBA" << std::endl;

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Always use RGBA format
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);

    std::cout << "Texture uploaded, ID=" << textureID << std::endl;
    return textureID;
}
