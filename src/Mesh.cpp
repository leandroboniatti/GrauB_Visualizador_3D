#include "Mesh.h"
#include "OBJReader.h"
#include "Shader.h"
#include <iostream>
#include <algorithm>
#include <cfloat>

Mesh::Mesh() {}

Mesh::~Mesh() {
    cleanup();
}

bool Mesh::readObjectModel(string& path) {
    
    // Carrega dados do OBJ chamando OBJReader::readFileOBJ, método da classe OBJReader.
    // Este, por sua vez, preenche os vetores e mapas passados por referência.
    // path - caminho do arquivo, recebido como parâmetro
    // vertices - vetor com os vértices do modelo, no formato VEC3, definido aqui na classe Mesh
    // texCoords - vetor com as coordenadas de textura, no formato VEC2, definido aqui na classe Mesh
    // normals - vetor com as normais de cada face no formato VEC3, definido aqui na classe Mesh
    // groups - grupo de grupos - vetor com os grupos, definido aqui na classe Mesh
    // materials - mapa de materiais do arquivo MTL, definido aqui na classe Mesh

    if (!OBJReader::readFileOBJ(path, vertices, texCoords, normals, groups, materials)) {
        return false;
    }

    // Extrai o diretório do modelo para carregar texturas
    size_t pos = path.find_last_of("/\\\\");
    string modelDirectory = (pos != string::npos) ? path.substr(0, pos) : ".";

    // Calcula a bounding box do modelo/objeto
    calculateBoundingBox();

    setupBuffers(); // Configura os buffers OpenGL (VBOs, VAOs) para cada grupo da malha

    // Carrega as texturas dos materiais MTL para cada grupo
    for (auto& group : groups) {
        group.loadMaterialTexture(modelDirectory);
    }

    return true;
}


// Configura buffers OpenGL (VBOs, VAOs) para cada grupo da malha
void Mesh::setupBuffers() {
    for (auto& group : groups) { group.setupBuffers(vertices, texCoords, normals);}

    cout << "Buffers OpenGL configurados" << endl;
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
    boundingBox = BoundingBox();
    
    for (const auto& vertex : vertices) {
        boundingBox.expand(vertex);
    }
    
    //cout << "Bounding box calculated: min(" << boundingBox.min.x << ", " 
    //          << boundingBox.min.y << ", " << boundingBox.min.z << "), max(" 
    //         << boundingBox.max.x << ", " << boundingBox.max.y << ", " 
    //          << boundingBox.max.z << ")" << endl;
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

// Calcula a normal de uma face dada pelos três vértices (v0, v1, v2)
vec3 Mesh::calculateFaceNormal(const vec3& v0, const vec3& v1, const vec3& v2) const {
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    return normalize(cross(edge1, edge2));
}
