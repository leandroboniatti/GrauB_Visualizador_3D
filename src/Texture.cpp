#include "Texture.h"
#include <iostream>
#include <stb_image.h>

// Inicialização do cache estático
map<string, unsigned int> Texture::textureCache;


// Função auxiliar privada para carregar textura do arquivo
static unsigned int loadTextureFromFile(const string& path) {

    unsigned int textureID;

    glGenTextures(1, &textureID);
    
    // Inverte a imagem verticalmente ao carregar (stb_image carrega de cima para baixo, OpenGL espera de baixo para cima)
    stbi_set_flip_vertically_on_load(true);
    
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
        
        return textureID;
    } else {
        cout << "Falha ao carregar textura: " << path << endl;
        glDeleteTextures(1, &textureID);
        return 0;
    }
}


// Carrega uma textura a partir do arquivo, utilizando o cache
unsigned int Texture::loadTexture(const string& path) {
    // Verifica se a textura já está no cache
    auto it = textureCache.find(path);
    if (it != textureCache.end()) {
        // Textura encontrada! Reutilizar a textura existente
        cout << "Textura recuperada do cache: " << path << " (ID: " << it->second << ")" << endl;
        return it->second;  // retorna o ID da textura
    } else {
        // Textura não encontrada - carregar do disco e adicionar ao cache
        unsigned int newTextureID = loadTextureFromFile(path);
        if (newTextureID != 0) {
            textureCache[path] = newTextureID;
            cout << "Textura carregada do arquivo: " << path << " (ID: " << newTextureID << ")" << endl;
        }
        return newTextureID;
    }
}


// Limpa o cache de texturas
void Texture::clearCache() {
    for (auto& pair : textureCache) {
        glDeleteTextures(1, &pair.second);
    }
    textureCache.clear();
}