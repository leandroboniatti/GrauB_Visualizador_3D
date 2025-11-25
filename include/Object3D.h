#ifndef OBJECT3D_H
#define OBJECT3D_H

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

using namespace std;
using namespace glm;

class Object3D {
public:
    Mesh mesh;         // malha do objeto 3D
    mat4 transform;    // matriz de transformação do objeto (model matrix)
    vec3 position;     // posição do objeto
    vec3 rotation;     // ângulos de rotação do objeto (em radianos)
    vec3 scale;        // escala do objeto

    bool eliminable;
    string name;
    //, modelPath, texturePath;
    
    // Animation support
    vector<vec3> animationPoints;   // Pontos da curva de animação
    int currentCurveIndex;          // Índice atual na curva
    float curveProgress;            // Progresso fracionário entre pontos
    bool isAnimated;          // Flag para indicar se o objeto está animado
    float animationSpeed;     // Velocidade da animação (pontos por update)
    
    // Texture support
    //unsigned int textureID;
    //bool hasTexture;
    
    Object3D();

    Object3D(string& objName);

    ~Object3D();

    // Carrega um objeto 3D a partir de um arquivo
    bool loadObject(string& objFilePath);

    // Renderiza o objeto 3D usando o shader fornecido
    void render(const Shader& shader) const;
    
    // Define a posição, rotação e escala do objeto e atualiza a matriz de transformação
    void setPosition(const vec3& pos);
    void setRotation(const vec3& rot);
    void setScale   (const vec3& scl);
    void setEliminable(bool canEliminate);
   // void setTexture(const string& texturePath);

    bool isEliminable() const { return eliminable; }

    // Animation methods
    bool loadAnimationCurve(const string& curveFilePath, 
                           const vec3& trackPosition = vec3(0.0f),
                           const vec3& trackRotation = vec3(0.0f),
                           const vec3& trackScale = vec3(1.0f));
    void updateAnimation(float deltaTime);
    void setAnimationSpeed(float speed);

    BoundingBox getTransformedBoundingBox() const;

    // Testa interseção do segmento (ray) com a bounding box (retorna true se houver interseção)
    bool rayIntersect(const vec3& rayOrigin, const vec3& rayDirection, float& distance) const;
    
    // Atualiza a matriz de transformação (model matrix) com base na posição, rotação e escala
    void updateTransform();
};

#endif
