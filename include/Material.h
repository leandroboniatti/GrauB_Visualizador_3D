#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

// Estrutura para representar um material MTL
struct Material {
    string name;      // Nome do material (identificador)
    vec3 Ka;          // Coeficiente de reflexão ambiente (Ambient)
    vec3 Kd;          // Coeficiente de reflexão difusa (Diffuse)
    vec3 Ks;          // Coeficiente de reflexão especular (Specular)
    float Ns;         // Expoente especular (Shininess) - brilho
    string map_Kd;    // Nome do arquivo da textura difusa (se houver)
    
    // Construtor padrão com valores iniciais
    Material() 
        : name("default"),
          Ka(0.2f, 0.2f, 0.2f),     // Ambiente padrão (cinza escuro)
          Kd(0.8f, 0.8f, 0.8f),     // Difusa padrão (cinza claro)
          Ks(1.0f, 1.0f, 1.0f),     // Especular padrão (branco)
          Ns(32.0f),                // Brilho médio
          map_Kd("")                // Sem textura
    {}
    
    // Construtor com parâmetros
    Material(const string& materialName, 
             const vec3& ambient,
             const vec3& diffuse,
             const vec3& specular,
             float shininess,
             const string& texture = "")
        : name(materialName),
          Ka(ambient),
          Kd(diffuse),
          Ks(specular),
          Ns(shininess),
          map_Kd(texture)
    {}
    
    // Verifica se o material tem textura
    bool hasTexture() const {
        return !map_Kd.empty();
    }
};

#endif
