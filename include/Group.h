#ifndef GROUP_H
#define GROUP_H

#include <vector>
#include <string>
#include "Face.h"
#include "Material.h"

using namespace std;
using namespace glm;

class Group {
public:
    string name;
    vector<Face> faces;
    Material material;  // Material associado ao grupo
    
    // OpenGL objects
    unsigned int VAO;
    unsigned int VBO;
    unsigned int textureID;  // ID da textura carregada do material MTL

    // Vetor de dados (floats) dos vértices (posições, normais, coordenadas de textura)
    // para envio à OpenGL. Armazena sequencialmente os atributos de cada vértice.
    // Exemplo: v1.x, v1.y, v1.z, v1.nx, v1.ny, v1.nz, v1.u, v1.v, v2.x, v2.y, ...
    // Cada grupo de 8 floats representa um vértice (posição<3> + texCoord<2> + normal<3>)
    vector<float> vertices;

    int vertexCount; // Número de vértices do grupo para envio à glDrawArrays
                     // cada vértice tem 8 floats (posição<3> + texCoord<2> + normal<3>)
                     // logo, vertexCount = vertices.size() / 8
    
    Group();

    Group(const string& groupName); // Construtor com nome do grupo

    ~Group();

    void addFace(const Face& face);

    // Configura os buffers de OpenGL (VBO e VAO) para o grupo em processamento
    // recebe referência dos vetores que guardam a posição, textura e normais do
    // objeto/Grupo em processamento,acessados através dos índices das faces do grupo
    void setupBuffers(const vector<vec3>& objVertices,
                      const vector<vec2>& objTexCoords,
                      const vector<vec3>& objNormals);


    // Alteramos para o Grau B
    // Renderiza o grupo de faces, enviando propriedades do material do grupo para os shaders                  
    void render(const class Shader& shader) const; // No Grau A era void Group::render() const;  // Alterado para receber referência do shader

    // Carrega a textura do material MTL
    void loadMaterialTexture(const string& modelDirectory);

    void cleanup();
};

#endif
