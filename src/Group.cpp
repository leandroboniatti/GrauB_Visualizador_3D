#include "Group.h"
#include "Shader.h"
#include "Texture.h"
#include <glad/glad.h>
#include <iostream>


Group::Group()
    : name(""), VAO(0), VBO(0), vertexCount(0), textureID(0) {}


Group::Group(const string& groupName) 
    : name(groupName), VAO(0), VBO(0), vertexCount(0), textureID(0) {}


Group::~Group() { cleanup(); }


// Adiciona uma face ao grupo (vetor de faces), triangulando se necessário
void Group::addFace(const Face& face) {

    // Antes de adicionar a face do objeto ao grupo, "triangula" a face
    // dividindo ela em triângulos, usando "fan triangulation - ver Face.cpp"
    auto triangles = face.triangulate();

    // Adiciona cada um dos triângulos resultantes ao grupo (que é um vetor de faces)
    for (const auto& triangle : triangles) { faces.push_back(triangle); }
}


// Configura os buffers de OpenGL (VBO e VAO) para o grupo em processamento
// optamos por usar um único VBO para posições, texturas e normais
void Group::setupBuffers(const vector<vec3>& objVertices,      // recebe referência dos vetores que guardam a posição,
                         const vector<vec2>& objTexCoords,     // textura e normais do objeto/Grupo em processamento,
                         const vector<vec3>& objNormals   ) {  // que serão acessados através dos índices das faces do grupo

    // Primeiro gera os dados sequenciais dos vértices do grupo, dentro do vetor "vertices"
    // "vertices" armazenará posição, coordenadas de textura e normal de cada vértice sequencialmente
    // "vertices" é atributo da classe Group - vector<float> vertices;
    vertices.clear();   // limpa dados anteriores, se houver, do vetor que guardará as informações
                        // dos vértices a serem enviados para renderização. Inseridos sequencialmente.
                        // posição<3> + texCoord<2> + normal<3> = 8 floats por vértice.
    
    for (const auto& face : faces) { // para cada face do grupo faz uma iteração e guarda informações em "vertices"

        for (size_t i = 0; i < face.vertexIndices.size(); i++) { // para cada posição de "vertexIndices" faz uma iteração
                                                                 // isto é, para cada vértice da face atual
            // Posição do vértice
            if (face.vertexIndices[i] - 1 < objVertices.size()) {               // ajuste de índice (OBJ inicia em 1 e vector em 0)
                const auto& vertex = objVertices[face.vertexIndices[i] - 1];    // acessa a informação da posição do vértice indiretamente, via índice
                vertices.push_back(vertex.x);
                vertices.push_back(vertex.y);
                vertices.push_back(vertex.z);
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }
            
            // Coordenadas de textura do vértice
            if (!face.textureIndices.empty() && i < face.textureIndices.size() && 
                face.textureIndices[i] - 1 < objTexCoords.size()) {
                const auto& texCoord = objTexCoords[face.textureIndices[i] - 1];  // acessa a informação da coordenada de textura do vértice indiretamente, via índice
                vertices.push_back(texCoord.x);
                vertices.push_back(texCoord.y);
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }
            
            // Normal do vértice
            if (!face.normalIndices.empty() && i < face.normalIndices.size() && 
                face.normalIndices[i] - 1 < objNormals.size()) {
                const auto& normal = objNormals[face.normalIndices[i] - 1];  // acessa a informação da normal do vértice indiretamente, via índice
                vertices.push_back(normal.x);
                vertices.push_back(normal.y);
                vertices.push_back(normal.z);
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(1.0f);
                vertices.push_back(0.0f);
            }
        }
    }
    
    // Calcular número de vértices do grupo
    vertexCount = vertices.size() / 8; // 8 floats por vértice (posição<3> + texCoord<2> + normal<3>)    

    // "vertices" é o vetor de dados (floats) dos vértices (posições, normais, coordenadas de textura)
    // para envio à OpenGL. Armazena sequencialmente os atributos de cada vértice.
    // Exemplo: v1.x, v1.y, v1.z, v1.u, v1.v, v1.nx, v1.ny, v1.nz, v2.x, v2.y, ...
    // Cada grupo de 8 floats representa um vértice (posição<3> + texCoord<2> + normal<3>)

    // Optamos por usar um único VBO para agrupar posições, normais e texturas

    // Configuração do VBO (Vertex Buffer Object) para o grupo
    glGenBuffers(1, &VBO); // Geração do identificador do VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Vincula (bind) o VBO do grupo em processamento
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW); // Envia os dados dos vértices para o buffer OpenGL

    // Configuração do VAO (Vertex Array Object) para o grupo
    glGenVertexArrays(1, &VAO); // Geração do identificador do VAO
    glBindVertexArray(VAO); // Vincula (bind) o VAO do grupo em processamento

    // Agora precisamos configurar os atributos de vértices (vertex attributes) para que a GPU
    // interprete corretamente os dados armazenados no buffer VAO atualmente vinculado

    // Configura Atributo coordenada de posição - coord x, y, z - 3 valores
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // location 0, offset 0, posição do vértice
    glEnableVertexAttribArray(0);   // Habilita o "location 0" do VAO - no vertex shader teremos layout(location = 0) para posição
    
    // Configura Atributo coordenada de textura - coord s, t - 2 valores
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // location 1, offset 3 floats, texCoord do vértice
    glEnableVertexAttribArray(1);   // Habilita o "location 1" do VAO - no vertex shader teremos layout(location = 1) para texCoord
    
    // Configura Atributo normal - coord x, y, z - 3 valores
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float))); // location 2, offset 5 floats, normal do vértice
    glEnableVertexAttribArray(2);   // Habilita o "location 2" do VAO - no vertex shader teremos layout(location = 2) para normal   
    
    // Desvincula o VBO e o VAO do grupo (boa prática)
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Desvincula o VBO do grupo
    glBindVertexArray(0); // Desvincula o VAO do grupo

    cout << "Grupo \"" << name << "\" configurado com VBO = " << VBO << " e VAO = " << VAO << endl;
    cout << endl;
}


// Alteramos para o Grau B - inserção do envio das propriedades do material para os shaders
// Renderiza o grupo de faces, enviando propriedades do material do grupo para os shaders
void Group::render(const Shader& shader) const {    // No Grau A era void Group::render() const { // alterado para receber referência do shader

    if (VAO == 0) return;   // Se não houver VAO configurado para o grupo, sai da função
    
    // Envia as propriedades do material para os shaders - acrescentado para o GRAU B
    glUniform3fv(glGetUniformLocation(shader.ID,"Ka"), 1, value_ptr(material.Ka)); // Ambiente
    glUniform3fv(glGetUniformLocation(shader.ID,"Kd"), 1, value_ptr(material.Kd)); // Difusa
    glUniform3fv(glGetUniformLocation(shader.ID,"Ks"), 1, value_ptr(material.Ks)); // Especular
    glUniform1f (glGetUniformLocation(shader.ID,"Ns"), material.Ns);               // Brilho (Shininess)    
    
    // Configura a textura se o material tiver uma
    if (textureID != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID); // Vincula a textura do material do grupo
        glUniform1i(glGetUniformLocation(shader.ID, "hasDiffuseMap"), true);
        glUniform1i(glGetUniformLocation(shader.ID, "diffuseMap"), 0);
    } else {
        glUniform1i(glGetUniformLocation(shader.ID, "hasDiffuseMap"), false);
    }
    
    glBindVertexArray(VAO); // Conectando ao buffer VAO do grupo
    glDrawArrays(GL_TRIANGLES, 0, vertexCount); // Desenha os triângulos do grupo
    glBindVertexArray(0); // Desvincula o VAO do grupo
    glBindTexture(GL_TEXTURE_2D, 0); // Desvincula a textura
}


// Limpa os buffers OpenGL do grupo - VBO, VAO
// Nota: textureID não é deletado aqui pois pode estar em cache e ser usado por outros grupos
void Group::cleanup() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    // textureID é gerenciado pelo cache em Texture::clearCache()
}


// Carrega a textura do material do arquivo MTL
void Group::loadMaterialTexture(const string& mtlDirectory) {
    if (material.hasTexture() && !material.map_Kd.empty()) {
        // Constroi o caminho completo da textura
        string texturePath = mtlDirectory + "/" + material.map_Kd;
        
        // Carrega a textura usando a classe Texture
        textureID = Texture::loadTexture(texturePath);
        
        if (textureID != 0) {
            cout << "Textura vinculada ao grupo \"" << name << "\" (ID: " << textureID << ")" << endl;
        } else {
            cerr << "Falha ao carregar textura do arquivo MTL para o grupo \"" << name << "\": " << texturePath << endl;
        }
    }
}