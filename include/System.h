#ifndef SYSTEM_H
#define SYSTEM_H

#include <vector>
#include <memory>
#include <string>
#include <glad/glad.h>  // biblioteca de funções baseada nas definições/especificações OPENGL
                        // Incluir antes de outros que requerem OpenGL (como GLFW)
#include <GLFW/glfw3.h> // biblioteca de funções para criação da janela no Windows
                        // e gerenciar entrada de teclado/mouse
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "Shader.h"
#include "Object3D.h"
#include "Projetil.h"

using namespace std;	// Para não precisar digitar std:: na frente de comandos da biblioteca
using namespace glm;	// Para não precisar digitar  na frente de comandos da biblioteca


// Estrutura da Classe System para armazenar informações de cada objeto da cena 3D,
// que serão carregados a partir do arquivo de configuração "Configurador_Cena.txt"
struct ObjectInfo {
    string name;        // Nome do objeto
    string modelPath;   // Caminho para o arquivo .obj
    vec3 position;      // Posição atual do objeto na cena
    vec3 rotation;      // Rotação aplicada ao objeto
    vec3 scale;         // Escala/Dimensões do objeto
    bool eliminable;    // Se o objeto pode ser eliminado por um tiro

    //string texturePath; // Caminho para a textura do objeto - No Grau B não é mais necessário
                          // pois foi deslocada para ser elemento da estrutura Material e 
                          // será associada a seu respectivo grupo da malha do Objecto 3D
                          // Mtl -> Textura -> id da textura -> grupo do objeto -> malha do objeto  
};

class System {
public:
    GLFWwindow* window; // Janela principal do sistema OpenGL
    Object3D* sceneObject; // Objeto atualmente selecionado (para manipulação)

    // Configurações da janela
    static const unsigned int SCREEN_WIDTH = 1024;
    static const unsigned int SCREEN_HEIGHT = 768;

    // Temporização
    float deltaTime;
    float lastFrame;    

    System();   // Construtor padrão

    ~System();  // Destrutor padrão
    
    bool initializeGLFW();
    bool initializeOpenGL();
    bool loadShaders();
    bool loadSceneObjects();
    void processInput();
    void render();
    void shutdown();
    
    Camera camera;      // câmera do sistema
    Shader mainShader;  // shader unificado para objetos da cena e projéteis
    
    // Propriedades de iluminação
    vec3 lightPos;      // Posição da luz na cena
    vec3 lightColor;    // Cor da luz
    
    // Coeficientes de atenuação da luz
    float attConstant;       // Atenuação constante
    float attLinear;         // Atenuação linear
    float attQuadratic;      // Atenuação quadrática
    
    // Propriedades do fog
    vec3 fogColor;      // Cor do fog
    float fogDensity;        // Densidade do fog
    float fogStart;          // Início do fog (linear)
    float fogEnd;            // Fim do fog (linear)
    int fogType;             // 0=linear, 1=exponencial, 2=exponencial²
    bool fogEnabled;         // Flag para ligar/desligar o fog
    

    // Cria um vetor para armazenar a coleção dos objetos 3D da cena
    vector<unique_ptr<Object3D>> sceneObjects;

    // Cria um vetor para armazenar a coleção dos projéteis disparados
    vector<unique_ptr<Projetil>> projeteis;
    
    // Entrada
    bool keys[1024];
    bool firstMouse;
    float lastX, lastY;

    void disparo();
    void updateProjeteis();
    void checkCollisions();
    
    // Callbacks
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
    
    // Funções auxiliares
    vector<ObjectInfo> readFileConfiguration();
};

#endif
