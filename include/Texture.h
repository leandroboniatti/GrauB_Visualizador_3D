#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <glad/glad.h>

using namespace std;

class Texture {
public:
    // Carrega uma textura a partir de um arquivo e retorna o ID da textura OpenGL
    static unsigned int loadTexture(const string& path);
};

#endif
