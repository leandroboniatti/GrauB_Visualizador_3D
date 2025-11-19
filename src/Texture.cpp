#include "Texture.h"
#include <iostream>
#include <stb_image.h>

unsigned int Texture::loadTexture(const string& path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;

    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    
    if (data) {
        GLenum format = GL_RGB; // Formato padrão
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        stbi_image_free(data);
        cout << "Textura carregada: " << path << endl;
        return textureID;
    } else {
        cout << "Falha ao carregar textura: " << path << endl;
        glDeleteTextures(1, &textureID);
        return 0;
    }
}
