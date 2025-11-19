#ifndef FACE_H
#define FACE_H

#include <vector>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

class Face {
public:
    vector<unsigned int> vertexIndices;
    vector<unsigned int> textureIndices;
    vector<unsigned int> normalIndices;
    
    Face();

    // Construtor com parâmetros. Os parâmetros são referências para os vetores de índices:
    Face(const vector<unsigned int>& vIndices,       // índices da posição dos vértices
         const vector<unsigned int>& tIndices = {},  // índices de texturas (opcional)
         const vector<unsigned int>& nIndices = {}); // índices de normais  (opcional)

    // Converte face com 4 ou mais vértices em triângulos usando "fan triangulation"
    vector<Face> triangulate() const;
};

#endif
