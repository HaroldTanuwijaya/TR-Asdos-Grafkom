#pragma once
#include <GL/glut.h>
#include "stb_image.h"
#include <iostream>
#include <fstream>
#include <windows.h>

// Define constants if not available
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#ifndef GL_GENERATE_MIPMAP
#define GL_GENERATE_MIPMAP 0x8191
#endif

// Function to check if file exists
bool fileExists(const char* filename) {
    std::ifstream file(filename);
    return file.good();
}

// Function to check if size is power of 2
bool isPowerOfTwo(int n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}

// Function to get next power of 2
int nextPowerOfTwo(int n) {
    int power = 1;
    while (power < n) {
        power *= 2;
    }
    return power;
}

// Manual mipmap generation for compatibility
void generateMipmapsManual(int width, int height, unsigned char* data) {
    int currentWidth = width;
    int currentHeight = height;
    int level = 0;
    
    unsigned char* currentData = new unsigned char[width * height * 4];
    memcpy(currentData, data, width * height * 4);
    
    while (currentWidth > 1 || currentHeight > 1) {
        int newWidth = std::max(1, currentWidth / 2);
        int newHeight = std::max(1, currentHeight / 2);
        
        unsigned char* newData = new unsigned char[newWidth * newHeight * 4];
        
        // Simple box filter downsampling
        for (int y = 0; y < newHeight; y++) {
            for (int x = 0; x < newWidth; x++) {
                int r = 0, g = 0, b = 0, a = 0;
                int count = 0;
                
                // Sample 2x2 area
                for (int sy = 0; sy < 2 && (y * 2 + sy) < currentHeight; sy++) {
                    for (int sx = 0; sx < 2 && (x * 2 + sx) < currentWidth; sx++) {
                        int srcIndex = ((y * 2 + sy) * currentWidth + (x * 2 + sx)) * 4;
                        r += currentData[srcIndex + 0];
                        g += currentData[srcIndex + 1];
                        b += currentData[srcIndex + 2];
                        a += currentData[srcIndex + 3];
                        count++;
                    }
                }
                
                int dstIndex = (y * newWidth + x) * 4;
                newData[dstIndex + 0] = r / count;
                newData[dstIndex + 1] = g / count;
                newData[dstIndex + 2] = b / count;
                newData[dstIndex + 3] = a / count;
            }
        }
        
        level++;
        glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, newWidth, newHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, newData);
        
        delete[] currentData;
        currentData = newData;
        currentWidth = newWidth;
        currentHeight = newHeight;
    }
    
    delete[] currentData;
}

GLuint loadTexture(const char* filename, bool forceResize = false) {
    std::cout << "Loading texture: " << filename << std::endl;
    
    // Check if file exists first
    if (!fileExists(filename)) {
        std::cerr << "File does not exist: " << filename << std::endl;
        
        // Print current working directory
        char cwd[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, cwd);
        std::cerr << "Current working directory: " << cwd << std::endl;
        
        // Try alternative paths
        std::string alternatives[] = {
            std::string("gambar/") + filename,
            std::string("../") + filename,
            std::string("texture/") + filename,
            std::string("textures/") + filename
        };
        
        bool found = false;
        for (int i = 0; i < 4; i++) {
            if (fileExists(alternatives[i].c_str())) {
                std::cout << "Found file at: " << alternatives[i] << std::endl;
                filename = alternatives[i].c_str();
                found = true;
                break;
            }
        }
        
        if (!found) {
            std::cerr << "Could not find texture file in any location!" << std::endl;
            return 0;
        }
    }
    
    // Check OpenGL limits
    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    std::cout << "Maximum texture size supported: " << maxTextureSize << "x" << maxTextureSize << std::endl;
    
    int width, height, channels;
    // Load image data
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load image: " << filename << "\n"
                  << "stb_image error: " << stbi_failure_reason() << std::endl;
        return 0;
    }
    
    std::cout << "Original image size: " << width << "x" << height << " pixels" << std::endl;
    std::cout << "Original channels: " << channels << " (forced to RGBA)" << std::endl;
    
    // Check if image is too large
    if (width > maxTextureSize || height > maxTextureSize) {
        std::cerr << "Image size (" << width << "x" << height << ") exceeds maximum texture size (" 
                  << maxTextureSize << "x" << maxTextureSize << ")" << std::endl;
        stbi_image_free(data);
        return 0;
    }
    
    // For older OpenGL implementations, you might need power-of-2 textures
    int newWidth = width;
    int newHeight = height;
    unsigned char* finalData = data;
    bool needsResize = false;
    
    // Check if we need to resize to power of 2 (for older OpenGL)
    if (forceResize || !isPowerOfTwo(width) || !isPowerOfTwo(height)) {
        newWidth = nextPowerOfTwo(width);
        newHeight = nextPowerOfTwo(height);
        
        // Make sure it doesn't exceed max texture size
        if (newWidth > maxTextureSize) newWidth = maxTextureSize;
        if (newHeight > maxTextureSize) newHeight = maxTextureSize;
        
        std::cout << "Resizing to power of 2: " << newWidth << "x" << newHeight << std::endl;
        
        // Resize the image data
        unsigned char* resizedData = new unsigned char[newWidth * newHeight * 4];
        
        // Simple nearest neighbor resize
        for (int y = 0; y < newHeight; y++) {
            for (int x = 0; x < newWidth; x++) {
                int srcX = (x * width) / newWidth;
                int srcY = (y * height) / newHeight;
                
                // Clamp to image bounds
                srcX = std::min(srcX, width - 1);
                srcY = std::min(srcY, height - 1);
                
                int srcIndex = (srcY * width + srcX) * 4;
                int dstIndex = (y * newWidth + x) * 4;
                
                resizedData[dstIndex + 0] = data[srcIndex + 0]; // R
                resizedData[dstIndex + 1] = data[srcIndex + 1]; // G
                resizedData[dstIndex + 2] = data[srcIndex + 2]; // B
                resizedData[dstIndex + 3] = data[srcIndex + 3]; // A
            }
        }
        
        finalData = resizedData;
        needsResize = true;
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Set pixel storage parameters
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newWidth, newHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, finalData);
    
    // Set texture parameters - use GL_CLAMP instead of GL_CLAMP_TO_EDGE for compatibility
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    
    // Generate mipmaps manually for compatibility
    generateMipmapsManual(newWidth, newHeight, finalData);
    
    // Clean up
    stbi_image_free(data);
    if (needsResize) {
        delete[] finalData;
    }
    
    std::cout << "Texture uploaded successfully, ID=" << textureID << std::endl;
    std::cout << "Final texture size: " << newWidth << "x" << newHeight << std::endl;
    
    return textureID;
}