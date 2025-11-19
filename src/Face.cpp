#include "Face.h"

Face::Face() {}

// Construtor recebe por referência os índices dos vértices, texturas e normais
// armazenando cópias locais nos parametros da classe (vIndices, tIndices, nIndices)
Face::Face(const vector<unsigned int>& vIndices, 
           const vector<unsigned int>& tIndices,
           const vector<unsigned int>& nIndices) :  vertexIndices (vIndices),  // lista de inicializadores
                                                         textureIndices(tIndices),  // dos parâmetros
                                                         normalIndices (nIndices) { }

// Converte face com 4 ou mais vértices em triângulos usando "fan triangulation" - mais simples - 
vector<Face> Face::triangulate() const {

    vector<Face> faces_triangulares = {};   // Vetor para armazenar as faces triangulares
    
    // se a face tem menos de 3 vértices, não é possível formar um triângulo
    if (vertexIndices.size()  < 3) { return faces_triangulares; }

    // Se a face já é um triângulo, retorna ela mesma
    if (vertexIndices.size() == 3) {
        faces_triangulares.push_back(*this);
        return faces_triangulares;
    }
    
    // Triangulação usando fan triangulation
    vector<unsigned int> triangleVertices, triangleTextures, triangleNormals;

    for (size_t i = 1; i < vertexIndices.size() - 1; i++) {

        triangleVertices = {vertexIndices[0], vertexIndices[i], vertexIndices[i + 1]};  // o primeiro vértice (fixo = 0) + os dois próximos

        if (!textureIndices.empty()) {
            triangleTextures = {textureIndices[0], textureIndices[i], textureIndices[i + 1]};
        }
        
        if (!normalIndices.empty()) {
            triangleNormals = {normalIndices[0], normalIndices[i], normalIndices[i + 1]};
        }

        faces_triangulares.emplace_back(triangleVertices, triangleTextures, triangleNormals);
    }

    return faces_triangulares;
}
