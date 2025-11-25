#include "Mesh.h"
#include "OBJReader.h"
#include "Shader.h"
#include <iostream>
#include <algorithm>
#include <cfloat>


Mesh::Mesh() {}


Mesh::~Mesh() { cleanup(); }


// Carrega dados do OBJ chamando OBJReader::readFileOBJ, método da classe OBJReader.
// lembrando a sequência de chamadas:
// System::loadSceneObjects -> Object3D::loadObject -> Mesh::readObjectModel -> OBJReader::readFileOBJ
bool Mesh::readObjectModel(string& objFilePath) {
    
    // "OBJReader::readFileOBJ" preenche os vetores e mapas da malha (Mesh), passados por referência:
    // vertices  - vetor com os vértices do modelo, no formato VEC3, definido aqui na classe Mesh
    // texCoords - vetor com as coordenadas de textura, no formato VEC2, definido aqui na classe Mesh
    // normals   - vetor com as normais de cada face no formato VEC3, definido aqui na classe Mesh
    // groups    - grupo de grupos - vetor com os grupos, definido aqui na classe Mesh
    // materials - mapa de materiais originado do arquivo MTL, definido aqui na classe Mesh

    // objFilePath - caminho do arquivo do modelo (OBJ), recebido como parâmetro

    bool leuArquivoOBJ = OBJReader::readFileOBJ(objFilePath, vertices, texCoords, normals, groups, materials);

    if (!leuArquivoOBJ) { return false; } // se não leu o arquivo OBJ, retorna falso

    // Extrai o diretório do modelo para carregar texturas
    size_t pos = objFilePath.find_last_of("/\\\\");
    string modelDirectory = (pos != string::npos) ? objFilePath.substr(0, pos) : ".";

    // Carrega as texturas e configura os buffers OpenGL para cada grupo
    for (auto& group : groups) {
        group.loadMaterialTexture(modelDirectory);  // Carrega as texturas dos materiais MTL para cada grupo
        group.setupBuffers(vertices, texCoords, normals); // Configura os buffers OpenGL (VBOs, VAOs) para cada grupo da malha
    }

    calculateBoundingBox(); // Calcula a bounding box do objeto

    return true;
}


// Renderiza a malha chamando render() de cada grupo
void Mesh::render(const Shader& shader) const {
    for (const auto& group : groups) {
        group.render(shader);
    }
}


// Limpa os dados da malha e libera recursos OpenGL
void Mesh::cleanup() {
    for (auto& group : groups) {
        group.cleanup();
    }
    groups.clear();
    vertices.clear();
    texCoords.clear();
    normals.clear();
}


// Calcula a bounding box do modelo/objeto
void Mesh::calculateBoundingBox() {

    boundingBox = BoundingBox();    // inicializa a bounding box com valores extremos FLT_MAX e -FLT_MAX
    
    for (const auto& vertice : vertices) {
        boundingBox.expand(vertice); // chama método expand da BoundingBox que atualiza os pontos mínimo e máximo
    }                                // testando se o vertice informado é menor ou maior que os atuais
}


// Testa interseção do segmento (ray) com a bounding box (retorna true se houver interseção)
bool Mesh::rayIntersect(const vec3& rayOrigin, const vec3& rayDirection, float& distance) const {
    // Ray-AABB teste de interseção usando o método
    vec3 invDir = 1.0f / rayDirection;
    vec3 t1 = (boundingBox.pontoMinimo - rayOrigin) * invDir;
    vec3 t2 = (boundingBox.pontoMaximo - rayOrigin) * invDir;
    
    vec3 tMin = min(t1, t2);
    vec3 tMax = max(t1, t2);
    
    float tNear = std::max(std::max(tMin.x, tMin.y), tMin.z);
    float tFar = std::min(std::min(tMax.x, tMax.y), tMax.z);
    
    if (tNear > tFar || tFar < 0.0f) {
        return false;
    }
    
    distance = tNear > 0.0f ? tNear : tFar;
    return true;
}