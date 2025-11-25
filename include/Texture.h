#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <map>
#include <glad/glad.h>

using namespace std;

class Texture {
private:
    // Cache de texturas já carregadas (path -> textureID)
    static map<string, unsigned int> textureCache;

public:
    // Carrega uma textura a partir de um arquivo e retorna o ID da textura OpenGL
    // Se a textura já foi carregada, retorna o ID do cache de texturas
    static unsigned int loadTexture(const string& path);
    
    // Limpa o cache de texturas (opcional, para liberar memória)
    static void clearCache();
};

#endif
