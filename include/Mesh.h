#ifndef MESH_H
#define MESH_H

#include <vector>
#include <map>
#include <cfloat>
#include <glm/glm.hpp>
#include "Group.h"
#include "Material.h"

using namespace std;
using namespace glm;

// Estrutura para Axis-Aligned Bounding Box (AABB)
struct BoundingBox {
    vec3 pontoMinimo;   // Ponto mínimo da bounding box
    vec3 pontoMaximo;   // Ponto máximo da bounding box
    
    BoundingBox() : pontoMinimo(vec3(FLT_MAX)), pontoMaximo(vec3(-FLT_MAX)) {}
    
    void expand(const vec3& point) {    // Expande ou contrai a bounding box para incluir o ponto fornecido
        pontoMinimo = min(pontoMinimo, point);  // min é método da glm -> encontra o menor valor das componentes x, y, z
        pontoMaximo = max(pontoMaximo, point);  // max é método da glm -> encontra o maior valor das componentes x, y, z
    }

    vec3 center() const { return (pontoMinimo + pontoMaximo) * 0.5f; }

    vec3 size() const { return pontoMaximo - pontoMinimo; }

    float radius() const { return length(size()) * 0.5f; }
};


class Mesh {
public:
    vector<vec3> vertices;  // Vetor que armazena os vértices da malha (objeto 3D)
    vector<vec2> texCoords; // Vetor que armazena as coordenadas de textura da malha (objeto 3D)
    vector<vec3> normals;   // Vetor que armazena as normais da malha (objeto 3D)
    vector<Group> groups;   // Vetor que armazena os grupos que compõem a malha (objeto 3D)
    map<string, Material> materials;  // Mapa de materiais que podem ser usados em cada grupo da malha (objeto 3D)
    
    BoundingBox boundingBox;    // estrutura da bounding box do objeto 3D
    
    Mesh();  // Construtor padrão
    ~Mesh(); // Destrutor

    
    // Carrega dados do OBJ chamando OBJReader::readFileOBJ, método da classe OBJReader.
    // Este, por sua vez, preenche os vetores e mapas passados por referência.
    bool readObjectModel(string& path);

    // Configura os buffers OpenGL (VBOs, VAOs) para cada grupo da malha
    void setupBuffers();
    
    // Renderiza a malha chamando render() de cada grupo
    void render(const class Shader& shader) const;

    // Limpa os dados da malha e libera recursos OpenGL
    void cleanup();
    
    // Calcula a bounding box do modelo/objeto
    void calculateBoundingBox();

    // Testa interseção do segmento (ray) com a bounding box (retorna true se houver interseção)
    bool rayIntersect(const vec3& rayOrigin, const vec3& rayDirection,
                     float& distance) const;

    // Calcula a normal de uma face dada pelos três vértices (v0, v1, v2)
    vec3 calculateFaceNormal(const vec3& v0, const vec3& v1, const vec3& v2) const;
};

#endif
