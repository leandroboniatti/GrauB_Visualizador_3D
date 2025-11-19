#include "OBJReader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

// Realiza a leitura de um arquivo OBJ, preenchendo os vetores passados por referência
// Uso: System::LoadSceneObjects -> OBJ3D:loadObject -> Mesh::readObjectModel -> OBJReader::readFileOBJ
// As classes anteriores "chamam" este método para carregar o modelo OBJ
// Os vetores de armazenamento de vértices, textura, normais e grupos
// são passados como referência para este método, que os preenche
bool OBJReader::readFileOBJ(const string& objFilePath,
                            vector<vec3>& vertices,
                            vector<vec2>& texCoords,
                            vector<vec3>& normals,
                            vector<Group>& groups,
                            map<string, Material>& materials)          {

    ifstream objFile(objFilePath);    // Abre o arquivo OBJ para leitura

    if (!objFile.is_open()) {  // Debug
        cerr << "Falha ao abrir arquivo OBJ: " << objFilePath << endl;
        return false;
    }
    
    vertices.clear();
    texCoords.clear();
    normals.clear();
    groups.clear();
    materials.clear();
    
    string objFileLine;  // variável temporária para armazenar cada linha lida do arquivo de configuração .obj
    Group* currentGroup = nullptr;  // Ponteiro para o grupo em processamento
    string currentMaterialName = ""; // Nome do material atual

    string objDirectory = getDirectory(objFilePath); // Obtém o diretório do arquivo .obj para localizar arquivos MTL e texturas

    while (getline(objFile, objFileLine)) { // Lê o arquivo linha por linha e armazena cada linha em 'objFileLine'
        objFileLine = trim(objFileLine);  // Remove espaços em branco no início e no final da linha

        if (objFileLine.empty() || objFileLine[0] == '#') { continue; } // Ignora linhas vazias e de comentários "#"

        istringstream sline(objFileLine); // Cria um stream de string para processar a linha
        string prefix;
        sline >> prefix;  // Lê o prefixo da linha (v, vt, vn, f, etc.)

        if (prefix == "mtllib") { // Carrega o arquivo MTL
            string mtlFileName;
            sline >> mtlFileName;
            string mtlFilePath = objDirectory + "/" + mtlFileName;
            readFileMTL(mtlFilePath, materials, objDirectory); // le o arquivo MTL e preenche o mapa de materiais
        }
        else if (prefix == "v") {           // adiciona as coord. do vértice (vec3), presentes na linha, ao vetor de
            parseVertice(objFileLine, vertices);   // coordenadas dos vértices definido na classe Mesh (vector<vec3> vertices;)
        }
        else if (prefix == "vt") {          // adiciona as coord. de textura (vec2), presentes na linha, ao vetor de
            parseTexCoord(objFileLine, texCoords); // coordenadas de textura definido na classe Mesh (vector<vec2> texCoords);
        }
        else if (prefix == "vn") {          // adiciona as normais (vec3), presentes na linha, ao vetor de    
            parseNormal(objFileLine, normals);     // normais definido na classe Mesh (vector<vec3> normals);
        }
        else if (prefix == "g" || prefix == "o") {  // Verifica o nome do grupo e inicia um novo grupo
            string groupName;
            sline >> groupName;
            if (groupName.empty()) groupName = "default";
            groups.emplace_back(groupName); // adiciona o grupo em processamento ao vetor de grupos
            currentGroup = &groups.back();  // ponteiro para o último elemento do vetor de groups (grupo atual)
            
            // Atribui o material atual ao grupo (se houver)
            if (!currentMaterialName.empty() && materials.find(currentMaterialName) != materials.end()) {
                currentGroup->material = materials[currentMaterialName];
            }
        }
        else if (prefix == "usemtl") { // Define o material atual
            sline >> currentMaterialName;
            
            // Se já existe um grupo, atribui o material a ele
            if (currentGroup && materials.find(currentMaterialName) != materials.end()) {
                currentGroup->material = materials[currentMaterialName];
            }
        }
        else if (prefix == "f") {   // Adiciona os indices que referenciam os vértices de uma face ao grupo atual.
                                    // No arquivo .obj cada vertice é representado por índices no formato:
                                    // vertice/texCoord/normal -> ex.: f 1/1/1 2/2/1 3/3/1 4/4/1
            if (!currentGroup) {
                groups.emplace_back("default"); // adiciona o grupo em processamento ao vetor de grupos
                currentGroup = &groups.back();  // ponteiro para o último elemento do vetor de groups (grupo atual)
                
                // Atribui o material atual ao grupo (se houver)
                if (!currentMaterialName.empty() && materials.find(currentMaterialName) != materials.end()) {
                    currentGroup->material = materials[currentMaterialName];
                }
            }
            
            Face face;  // gera/instancia uma nova face
            parseFace(objFileLine, face);  // popula a face com os INDICES lidos na linha
            currentGroup->addFace(face);   // adiciona a face processada ao grupo atual, que já está no vetor de grupos
        }
    }

    objFile.close();    // Fecha o arquivo .obj

    return true;
}


// Realiza a Leitura de um arquivo MTL e preenche o mapa de materiais
bool OBJReader::readFileMTL(const string& mtlFilePath,
                            map<string, Material>& materials,
                            const string& objDirectory) {
    
    ifstream mtlFile(mtlFilePath); // Abre o arquivo MTL para leitura
    
    if (!mtlFile.is_open()) {   // Debug
        cerr << "Aviso: Falha ao abrir arquivo MTL: " << mtlFilePath << endl;
        return false;
    }
    
    Material currentMaterial;   // instancia um material temporário
    bool hasMaterial = false;
    string mtlFileLine; // variável temporária para armazenar cada linha lida do arquivo de configuração .mtl
    
    while (getline(mtlFile, mtlFileLine)) { // Lê o arquivo linha por linha e armazena cada linha em 'mtlFileLine'
        mtlFileLine = trim(mtlFileLine);    // Remove espaços em branco no início e no final da linha
        
        if (mtlFileLine.empty() || mtlFileLine[0] == '#') { continue; } // Ignora linhas vazias e de comentários "#"
        
        istringstream sline(mtlFileLine);   // Cria um stream de string para processar a linha
        string prefix;
        sline >> prefix;    // Lê o prefixo da linha (newmtl, Ka, Kd, Ks, Ns, map_Kd, etc.)
        
        if (prefix == "newmtl") {
                
            if (hasMaterial) { materials[currentMaterial.name] = currentMaterial; } // Se já havia um material sendo processado, salva no mapa
            
            currentMaterial = Material();   // instancia um novo material padrão
            sline >> currentMaterial.name;
            hasMaterial = true;
        }
        else if (prefix == "Ka") {  // Coeficiente de reflexão ambiente
            float r, g, b;
            sline >> r >> g >> b;
            currentMaterial.Ka = vec3(r, g, b);
        }
        else if (prefix == "Kd") {  // Coeficiente de reflexão difusa
            float r, g, b;
            sline >> r >> g >> b;
            currentMaterial.Kd = vec3(r, g, b);
        }
        else if (prefix == "Ks") {  // Coeficiente de reflexão especular
            float r, g, b;
            sline >> r >> g >> b;
            currentMaterial.Ks = vec3(r, g, b);
        }
        else if (prefix == "Ns") {  // Expoente especular (brilho/shininess)
            float ns;
            sline >> ns;
            currentMaterial.Ns = ns;
        }
        else if (prefix == "map_Kd") {  // Textura
            //string texturePath;
            //sline >> texturePath;
            // Mantém apenas o nome do arquivo, o caminho completo será resolvido depois
            //currentMaterial.map_Kd = texturePath;
            sline >> currentMaterial.map_Kd;    // caminho do arquivo de textura
        }
    }
    
    // Salva o último material processado
    if (hasMaterial) { materials[currentMaterial.name] = currentMaterial; }
    
    mtlFile.close();
    
    cout << "Carregado(s) " << materials.size() << " material(is) do arquivo " << mtlFilePath << endl;
    
    return true;
}




// Analisa uma linha do arquivo OBJ e preenche os indices da face
// nos vetores de índices da face (vertexIndices, textureIndices, normalIndices)
void OBJReader::parseFace(const string& faceStr, Face& face) {
    
    istringstream sline(faceStr); // Cria um stream de string para processar a linha

    string indicesStr;  // string temporária para processamento

    sline >> indicesStr; // ignora o caracter "f"

    while (sline >> indicesStr) {  // para cada sequencia de indices presentes na linha:

        vector<string> indices = split(indicesStr, '/');       // retira o caracter "/"

        if (!indices.empty() && !indices[0].empty()) {         // Adiciona o índice que referencia as coordenadas do vértice (vec3)
            face.vertexIndices.push_back(stoi(indices[0]));    // presentes no vetor de vértices definido na classe Mesh
        }                                                      // (vector<vec3> vertices;)

        if (indices.size() > 1 && !indices[1].empty()) {       // Adiciona o índice que referencia as coordenadas de textura (vec2)
            face.textureIndices.push_back(stoi(indices[1]));   // presentes no vetor de coordenadas de textura definido na classe Mesh
        }                                                      // (vector<vec2> texCoords;)

        if (indices.size() > 2 && !indices[2].empty()) {       // Adiciona o índice que referencia as coordenadas da normal (vec3)
            face.normalIndices.push_back(stoi(indices[2]));    // presentes no vetor de normais definido na classe Mesh
        }                                                      // (vector<vec3> normals;)
    }
}


void OBJReader::parseVertice(const string& line, vector<vec3>& vertices) {
    
    istringstream sline(line);
    string prefix;
    float x, y, z;
    sline >> prefix >> x >> y >> z;
    vertices.emplace_back(x, y, z);
}


void OBJReader::parseTexCoord(const string& line, vector<vec2>& texCoords) {

    istringstream sline(line);
    string prefix;
    float u, v;
    sline >> prefix >> u >> v;
    texCoords.emplace_back(u, v);
}


void OBJReader::parseNormal(const string& line, vector<vec3>& normals) {

    istringstream sline(line);
    string prefix;
    float x, y, z;
    sline >> prefix >> x >> y >> z;
    normals.emplace_back(x, y, z);
}


vector<string> OBJReader::split(const string& str, char delimiter) {

    vector<string> tokens;
    string token;
    istringstream tokenStream(str);

    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}


string OBJReader::trim(const string& str) {
    size_t first = str.find_first_not_of(' ');
    if (string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}


string OBJReader::getDirectory(const string& filepath) {
    size_t pos = filepath.find_last_of("/\\");
    if (pos != string::npos) {
        return filepath.substr(0, pos);
    }
    return ".";
}
