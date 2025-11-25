#ifndef OBJREADER_H
#define OBJREADER_H

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "Group.h"
#include "Material.h"

using namespace std;
using namespace glm;

class OBJReader {
public:

    // Lê um arquivo OBJ e preenche os vetores e grupos fornecidos por referência
    static bool readFileOBJ(const string& path,
                        vector<vec3>& vertices,
                        vector<vec2>& texCoords,
                        vector<vec3>& normals,
                        vector<Group>& groups,
                        map<string, Material>& materials);
    
    // Lê um arquivo MTL e preenche o mapa de materiais fornecido por referência
    static bool readFileMTL(const string& path, map<string, Material>& materials);
    
    // Divide uma string em substrings com base em um delimitador
    static vector<string> split(const string& str, char delimiter);

    // Remove espaços em branco do início e fim de uma string
    static string trim(const string& str);

    // Extrai o diretório de um caminho de arquivo
    static string getDirectory(const string& filepath);

    // Analisa uma linha do arquivo OBJ e preenche os indices da face
    // nos vetores de índices da face (vertexIndices, textureIndices, normalIndices)
    static void parseFace(const string& faceStr, Face& face);

    // Analisa uma linha do arquivo OBJ ("v") e preenche os dados de posição do vértice
    // no vetor de vértices (vec3)
    static void parseVertice(const string& line, vector<vec3>& vertices);

    // Analisa uma linha do arquivo OBJ ("vt") e preenche os dados de coordenadas de textura
    // no vetor de coordenadas de textura (vec2)
    static void parseTexCoord(const string& line, vector<vec2>& texCoords);

    // Analisa uma linha do arquivo OBJ ("vn") e preenche os dados de normais
    // no vetor de normais (vec3)
    static void parseNormal(const string& line, vector<vec3>& normals);
};

#endif
