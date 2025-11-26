/***         GRAU B - Visualizador 3D com Iluminação Phong        ***/
/*** Computação Gráfica em Tempo Real - Jogos Digitais - Unisinos ***/
/***          Visualizador de Modelos 3D com OpenGL 4+            ***/
/***        Alunos: Ian Rossetti Boniatti e Eduardo Tropea        ***/

/*** Fontes:
                * GRAU A de Fundamentos de CGR e GRAU B de Fundamentos de CG
                * Exemplo SaberThoot
                * UML dos slides "Modelagem 3D"
                * pseudos códigos dos slides de CGR
                * learnopengl.com
                * Exemplos da internet
***/

#include <iostream>
#include "System.h"

using namespace std;

int main() {
    cout << endl;
    cout << "    Visualizador de Modelos 3D - CGR    " << endl;
    cout << endl;

    System system;  // Instancia o sistema (onde teremos janela, OpenGL, Shaders, cena, etc)

    // inicializa a GLFW (janela, contexto, callbacks, etc - na apresentação ver System.cpp)
    if (!system.initializeGLFW()) {
        cerr << "Falha ao inicializar GLFW" << endl;
        return EXIT_FAILURE; }

    // Inicializa OpenGL (GLAD, Viewport, Depth Test, etc - ver System.cpp)
    if (!system.initializeOpenGL()) {
        cerr << "Falha ao inicializar OpenGL" << endl;
        return EXIT_FAILURE; }

    // Carrega os shaders (ver System.cpp)
    if (!system.loadShaders()) {
        cerr << "Falha ao carregar shaders" << endl;
        return EXIT_FAILURE;
    }

    // Carrega configurações da cena (câmera, luz, fog) do arquivo de configuração
    if (!system.loadSystemConfiguration()) {
        cerr << "Falha ao carregar configuracoes da cena (camera, luz, fog)" << endl;
        return EXIT_FAILURE;
    }

    // Carrega os objetos da cena (ver System.cpp)
    if (!system.loadSceneObjects()) {
        cerr << "Falha ao carregar objetos da cena" << endl;
        return EXIT_FAILURE;
    }

    cout << endl;
    cout << "Sistema inicializado com sucesso" << endl;
    cout << endl;

    cout << "Controles:" << endl;
    cout << "  WASD/Setas: Mover camera" << endl;
    cout << "  Mouse: Olhar ao redor" << endl;
    cout << "  Scroll: Zoom" << endl;
    cout << "  ESPAÇO: Atirar" << endl;
    cout << "  ESC: Sair" << endl;
    cout << endl;

    // Main loop - game loop
    while (!glfwWindowShouldClose(system.window)) {
        
        float currentFrame = glfwGetTime(); // Tempo atual em segundos desde que a GLFW foi inicializada 
        system.deltaTime = currentFrame - system.lastFrame; // Tempo entre frames para movimentação dos
        system.lastFrame = currentFrame;                    // projéteis e demais objetos animados

        system.processInput();  // Processa entrada do usuário (teclado, mouse, etc - ver System.cpp)

        system.updateAnimations();  // Atualiza animações dos objetos (ver System.cpp)

        system.updateProjeteis();   // Atualiza posição dos projéteis (ver System.cpp)

        system.checkCollisions();   // Verifica colisões entre projéteis e objetos da cena (ver System.cpp)

        system.render();        // Renderiza a cena (ver System.cpp)

        glfwSwapBuffers(system.window); // Troca os buffers da janela (ver System.cpp)

        glfwPollEvents();   // Processa eventos da janela (teclado, mouse, etc) (ver System.cpp)
    }

    system.shutdown(); // Limpa e finaliza o sistema

    return 0;
}