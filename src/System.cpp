#include "System.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

// Variáveis estáticas para controle de entrada
static System* systemInstance = nullptr;
static bool tiroDisparado = false;
static bool fogTogglePressed = false;

System::System() : window(nullptr), 
                   camera(vec3(0.0f, 2.0f, 20.0f)),
                   deltaTime(0.0f),
                   lastFrame(0.0f),
                   firstMouse(true),
                   lastX(SCREEN_WIDTH  / 2.0f),
                   lastY(SCREEN_HEIGHT / 2.0f),
                   lightPos(0.0f, 10.0f, 5.0f),
                   lightColor(1.0f, 1.0f, 1.0f),
                   attConstant(1.0f),
                   attLinear(0.045f),
                   attQuadratic(0.0075f),
                   fogColor(0.9f, 0.9f, 0.9f), // cinza claro
                   fogDensity(0.05f),          // densidade do fog
                   fogStart(10.0f),
                   fogEnd(50.0f),
                   fogType(1),  // 0=linear, 1=exponencial, 2=exponencial²
                   fogEnabled(true) // fog inicialmente ligado
{
    systemInstance = this;

    // Inicializando o array de controle das teclas
    for (int i = 0; i < 1024; i++) { keys[i] = false; }
}


System::~System() { shutdown(); }


void System::shutdown() {
    sceneObjects.clear();
    projeteis.clear();
    
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();

    cout << "Desligamento do sistema concluido" << endl;
}


// Inicializa a GLFW (janela, contexto, callbacks)
bool System::initializeGLFW() {

    // GLFW: Inicialização e configurações de versão do OpenGL
    glfwInit(); // Inicialização da GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);  // Informa a versão do OpenGL a partir da qual o código funcionará
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);  // Exemplo para versão 3.3 - adaptar para a versão suportada por sua placa
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);  // Ativa MSAA 4x (Multisample Anti-Aliasing) - suaviza bordas serrilhadas
    
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GRAU B - Ian R. Boniatti e Eduardo Tropea", NULL, NULL);

    if (!window) {
        cerr << "Falha ao criar janela GLFW" << endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window); // Define a janela atual como contexto de renderização
    
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // Ajusta a viewport quando a janela é redimensionada
    glfwSetCursorPosCallback(window, mouse_callback);   // Captura a posição do mouse
    glfwSetScrollCallback(window, scroll_callback); // Captura o scroll do mouse
    glfwSetKeyCallback(window, key_callback);   // Captura eventos do teclado
    
    // Capturar mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    return true;
}


// Inicializa OpenGL (GLAD, Viewport, Depth Test)
bool System::initializeOpenGL() {

    // GLAD: Inicializa e carrega todos os ponteiros de funções da OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cerr << "Falha ao inicializar GLAD" << endl;
        return false;
    }
    
    // para desenhar apenas os fragmentos mais próximos da câmera
    glEnable(GL_DEPTH_TEST);        // Ativa o teste de profundidade (z-buffer)
    glClear(GL_DEPTH_BUFFER_BIT);   // Limpa o buffer de profundidade

    // Ativa multisampling para antialiasing (suaviza bordas serrilhadas) - Pesquisado na internet
    glEnable(GL_MULTISAMPLE);

    // Definindo as dimensões da viewport
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // Imprimir informações do OpenGL e Placa de Vídeo
    cout << "Versao OpenGL: " << glGetString(GL_VERSION) << endl;
    cout << "Renderer: "      << glGetString(GL_RENDERER) << endl;

    return true;
}


// Alteramos para o Grau B - Iluminação de Phong e texturas
// Carrega os shaders
bool System::loadShaders() {
    // Código fonte do Vertex Shader com iluminação de Phong
    string vertexShaderSource = R"(
        #version 400 core
        layout (location = 0) in vec3 coordenadasDaGeometria;
        layout (location = 1) in vec2 coordenadasDaTextura;
        layout (location = 2) in vec3 coordenadasDaNormal;
        
        out vec2 textureCoord;    // Coordenadas de textura do vértice
        out vec3 elementPosition; // No VS representa a posição do vértice no world space   // antes era fragPos
        out vec3 normal;          // Vetor normal no world space

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform bool isProjectile; // flag para diferenciar projéteis de objetos da cena
        
        void main() {

            vec4 worldPos = model * vec4(coordenadasDaGeometria, 1.0);  // Posição dos vértices antes da projeção (world space)
            elementPosition = worldPos.xyz;                             // Converte vec4 para vec3

            gl_Position = projection * view * worldPos;                 // Posição final do vértice após todas as transformações
            
            normal = mat3(transpose(inverse(model))) * coordenadasDaNormal; // transforma a normal para o world space
                                                                            // usando transposta da inversa da matriz de modelo


            // Passa coordenadas de textura
            if (!isProjectile) { textureCoord = coordenadasDaTextura; }
            else { textureCoord = vec2(0.0, 0.0); } // valor padrão para projéteis
        }
    )";
    // Inputs globais (uniforms) do pipeline:
        // "model"        matriz de transformações a serem aplicadas ao objeto (translação, rotação, escala)
        // "view"         matriz de visualização da câmera (posição, direção, etc.)
        // "projection"   matriz de projeção escolhida (perspectiva ou ortográfica)
        // "isProjectile" flag para diferenciar projéteis de objetos da cena
    // Inputs do Vertex Shader:
	    // "coordenadasDaGeometria" recebe as informações que estão no local 0 -> definidas em glVertexAttribPointer(0, xxxxxxxx);
		// "coordenadasDaTextura"   recebe as informações que estão no local 1 -> definidas em glVertexAttribPointer(1, xxxxxxxx);
        // "coordenadasDaNormal"    recebe as informações que estão no local 2 -> definidas em glVertexAttribPointer(2, xxxxxxxx);
    // Outputs do Vertex Shader:
        // "elementPosition"     enviará ao pipeline a posição do vértice no "world space"
        // "normal"       enviará ao pipeline a normal do vértice no "world space"
		// "textureCoord" enviará ao pipeline a textura de uma posição específica
		// "gl_Position"  é uma variável específica do GLSL que recebe a posição final do vertice processado após todas as transformações


	// Código fonte do Fragment Shader com modelo de Phong completo
    string fragmentShaderSource = R"(
        #version 400 core
        out vec4 FragColor;     // Cor final do fragmento - output do Fragment Shader
        
        // Inputs do Fragment Shader - outputs do Vertex Shader
        in vec2 textureCoord;     // Coordenadas de textura
        in vec3 elementPosition;  // No FS representa a posição do fragmento // antes era fragPos
        in vec3 normal;           // NORMAL INTERPOLADA pelo pipeline - // normal do fragmento
        
        // Propriedades do material
        uniform vec3 Ka;   // Coeficiente ambiente
        uniform vec3 Kd;   // Coeficiente difuso
        uniform vec3 Ks;   // Coeficiente especular
        uniform float Ns;  // Expoente especular (shininess)  
        
        // Propriedades da luz
        uniform vec3 lightPos;      // Posição da luz
        uniform vec3 lightColor;    // Cor da luz
        uniform vec3 viewPos;       // Posição da câmera
        
        // Coeficientes de atenuação da luz
        uniform float attConstant;  // Atenuação constante
        uniform float attLinear;    // Atenuação linear
        uniform float attQuadratic; // Atenuação quadrática
        
        // Parâmetros do fog
        uniform bool fogEnabled;    // Flag para ligar/desligar o fog
        uniform vec3  fogColor;     // Cor do fog
        uniform float fogDensity;   // Densidade do fog (para fog exponencial)
        uniform float fogStart;     // Início do fog (para fog linear)
        uniform float fogEnd;       // Fim do fog (para fog linear)
        uniform int fogType;        // 0=linear, 1=exponencial, 2=exponencial²
        
        // Texturas
        uniform sampler2D diffuseMap;   // Mapa de textura difusa
        uniform bool hasDiffuseMap;     // Indica se o objeto possui mapa de textura difusa
        uniform bool isProjectile;      // Indica se o objeto é um projétil
        uniform vec3 objectColor;       // Cor sólida do objeto (se não usar textura)
        
        void main() { // processamento de cada fragmento
            
            vec3 norm = normalize(normal); // normaliza a normal (interpolada) para cálculos de iluminação
            
            // Cor base do material (textura ou cor sólida)
            vec3 baseColor = objectColor;
            if (!isProjectile && hasDiffuseMap) { baseColor = texture(diffuseMap, textureCoord).rgb; }
            
            // CÁLCULO DA ATENUAÇÃO DA LUZ
            float distance = length(lightPos - elementPosition);
            float attenuation = 1.0 / (attConstant + attLinear * distance + 
                                      attQuadratic * (distance * distance));

            // CÁLCULO DA COMPONENTE AMBIENTE
            vec3 ambient = Ka * lightColor * baseColor;

            // CÁLCULO DA COMPONENTE DIFUSA
            vec3 lightDir = normalize(lightPos - elementPosition);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = Kd * diff * attenuation * lightColor * baseColor;
            
            // CÁLCULO DA COMPONENTE ESPECULAR (reflexo brilhante - não usa baseColor)
            vec3 viewDir = normalize(viewPos - elementPosition);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), Ns);
            vec3 specular = Ks * spec * attenuation * lightColor;
            
            // COR FINAL DO FRAGMENTO (SEM FOG) - Phong: Ambient + Diffuse + Specular
            vec3 finalFragmentColor = ambient + diffuse + specular;
            
            // CÁLCULO DO FOG (apenas se estiver habilitado)
            if (fogEnabled) {
                float fogFactor = 0.0;
                float fogDistance = length(viewPos - elementPosition);
                
                if (fogType == 1) {
                    // Fog linear
                    //fogFactor = (fogEnd - fogDistance) / (fogEnd - fogStart);
                    fogFactor = 1 / fogDistance; // alternativa para evitar divisão por zero

                } else if (fogType == 0) {
                    // Fog exponencial
                    fogFactor = exp(-fogDensity * fogDistance);
                } else if (fogType == 2) {
                    // Fog exponencial ao quadrado
                    fogFactor = exp(-pow(fogDensity * fogDistance, 2.0));
                }
                
                fogFactor = clamp(fogFactor, 0.0, 1.0);  // garante que o fator fique entre 0 e 1
                
                // Interpola entre a cor do objeto e a cor do fog
                finalFragmentColor = mix(fogColor, finalFragmentColor, fogFactor);
            }
            
            FragColor = vec4(finalFragmentColor, 1.0); // envia a cor final do fragmento para o pipeline
        }
    )";
    
    // Compila e linka os shaders
    if (!mainShader.loadFromStrings(vertexShaderSource, fragmentShaderSource)) {
        return false;
    }
    
    return true;
}


// Carrega os objetos na cena
bool System::loadSceneObjects() {
                                                     // na apresentação: ver readFileConfiguration() logo abaixo
    auto sceneObjectsInfo = readFileConfiguration(); // lê as configurações gerais dos objetos da cena, a partir do arquivo de configuração,
                                                     // e retorna um vetor (sceneObjectsInfo) de estruturas ObjectInfo

    for (auto& sceneObject : sceneObjectsInfo) { // loop para processar cada configuração de objeto lida do arquivo
        auto object = make_unique<Object3D>(sceneObject.name);  // cria um novo objeto 3D com o nome
                                                                // especificado no arquivo de configuração lido acima

        // Tenta carregar o modelo (arquivo .obj), se falhar não adiciona o objeto à cena
        // As texturas agora são carregadas automaticamente através dos materiais MTL
        if (object->loadObject(sceneObject.modelPath)) {
            object->setPosition(sceneObject.position);
            object->setRotation(sceneObject.rotation);
            object->setScale(sceneObject.scale);
            object->setEliminable(sceneObject.eliminable);

            sceneObjects.push_back(move(object));   // adiciona o objeto 3D criado à lista de objetos 3D da cena
                                                    // o vetor sceneObjects é um atributo da classe System, gerado no arquivo System.h
            cout << "Objeto carregado: " << sceneObject.name << endl;
        }
        else {
            cout << "Falha ao carregar objeto: " << sceneObject.name
                 << " de " << sceneObject.modelPath << endl;
        }
    }

    return true;
}


// Carrega as informações gerais dos objetos da cena a partir do arquivo de configuração da cena - "Configurador_Cena.txt"
// No Grau A era: (Nome Path posX posY posZ rotX rotY rotZ scaleX scaleY scaleZ Eliminável(S/N) TexturePath)
// Agora no Grau B ficou: (Nome Path posX posY posZ rotX rotY rotZ scaleX scaleY scaleZ Eliminável(S/N))
// uma vez que as texturas agora são carregadas automaticamente através da referência do arquivo .mtl
vector<ObjectInfo> System::readFileConfiguration() {

    vector<ObjectInfo> sceneObjectsInfo;  // ObjectInfo é uma estrutura para armazenar informações sobre um determinado objeto 3D
                                          // sceneObjectsInfo é um vetor que armazena várias dessas estruturas (qtd = nº de objetos da cena)

    ifstream configFile("Configurador_Cena.txt");   // abre o arquivo de configuração para leitura

    string line;  // variável temporária para armazenar cada linha lida do arquivo de configuração

    while (getline(configFile, line)) { // loop para processar cada linha do arquivo de configuração
        if (line.empty() || line[0] == '#') continue; // Ignora linhas vazias ou comentários

        // Nome Path posX posY posZ rotX rotY rotZ scaleX scaleY scaleZ eliminável
        istringstream sline(line);  // Cria um stream (sline) a partir da linha lida
        ObjectInfo objectInfo;      // instancia estrutura para armazenar informações do objeto descrito na linha processada

        // carrega os dados da linha para o respectivo campo da estrutura objectInfo
        sline >> objectInfo.name
              >> objectInfo.modelPath
              >> objectInfo.position.x
              >> objectInfo.position.y
              >> objectInfo.position.z
              >> objectInfo.rotation.x
              >> objectInfo.rotation.y
              >> objectInfo.rotation.z
              >> objectInfo.scale.x
              >> objectInfo.scale.y
              >> objectInfo.scale.z
              >> objectInfo.eliminable;
              //>> objectInfo.texturePath;

        sceneObjectsInfo.push_back(objectInfo); // adiciona a estrutura recém preenchida ao vetor de configurações
    }                                           // vetor de estruturas ObjectInfo
    
    configFile.close();

    // debug: informa se objetos foram carregados
    if (sceneObjectsInfo.empty()) { cerr << "Nenhum objeto carregado: verifique o arquivo Configurador_Cena.txt" << endl; }
    else { cout << "Informações gerais de " << sceneObjectsInfo.size() << " objetos encontradas no arquivo de configuração de cena." << endl; }

    return sceneObjectsInfo;
}


// Processa a entrada do usuário
void System::processInput() {

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // movimentação da câmera
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    
    // Disparo
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !tiroDisparado) {
        disparo();
        tiroDisparado = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        tiroDisparado = false;
    }
    
    // Toggle Fog com tecla F
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fogTogglePressed) {
        fogEnabled = !fogEnabled;
        fogTogglePressed = true;
        cout << "Fog " << (fogEnabled ? "ligado" : "desligado") << endl;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
        fogTogglePressed = false;
    }
}


// Renderiza a cena
void System::render() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);   // cor de fundo (branco)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calcula a matriz de projeção - perspective(FOV, razão de aspecto, Near, Far) - razão de aspecto = largura/altura
    mat4 projection = perspective(radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

    // Calcula a matriz de visualização - lookAt(posição da câmera, ponto para onde a câmera está olhando, vetor up da câmera)
    mat4 view = camera.GetViewMatrix(); // lookAt(Position, Position + Front, Up)
    
    // Configura 
    mainShader.use();
    glUniformMatrix4fv(glGetUniformLocation(mainShader.ID, "projection"), 1, GL_FALSE, value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(mainShader.ID, "view"), 1, GL_FALSE, value_ptr(view));
    
    // Configura uniforms de iluminação (modelo de Phong completo)
    glUniform3fv(glGetUniformLocation(mainShader.ID, "lightPos"), 1, value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(mainShader.ID, "lightColor"), 1, value_ptr(lightColor));
    glUniform3fv(glGetUniformLocation(mainShader.ID, "viewPos"), 1, value_ptr(camera.Position));
    
    // Coeficientes de atenuação atmosférica
    glUniform1f(glGetUniformLocation(mainShader.ID, "attConstant"), attConstant);
    glUniform1f(glGetUniformLocation(mainShader.ID, "attLinear"), attLinear);
    glUniform1f(glGetUniformLocation(mainShader.ID, "attQuadratic"), attQuadratic);
    
    // Parâmetros do fog
    glUniform1i(glGetUniformLocation(mainShader.ID, "fogEnabled"), fogEnabled);
    glUniform3fv(glGetUniformLocation(mainShader.ID, "fogColor"), 1, value_ptr(fogColor));
    glUniform1f(glGetUniformLocation(mainShader.ID, "fogDensity"), fogDensity);
    glUniform1f(glGetUniformLocation(mainShader.ID, "fogStart"), fogStart);
    glUniform1f(glGetUniformLocation(mainShader.ID, "fogEnd"), fogEnd);
    glUniform1i(glGetUniformLocation(mainShader.ID, "fogType"), fogType);
    
    glUniform1i(glGetUniformLocation(mainShader.ID, "isProjectile"), false); // objetos da cena não são projéteis
    glUniform3f(glGetUniformLocation(mainShader.ID, "objectColor"), 1.0f, 1.0f, 1.0f);
    
    for (const auto& obj : sceneObjects) { // renderiza cada objeto da cena
        obj->render(mainShader);
    }
    
    // Render projeteis
    glUniform1i(glGetUniformLocation(mainShader.ID, "isProjectile"), true); // agora renderizando projéteis
    glUniform1i(glGetUniformLocation(mainShader.ID, "hasDiffuseMap"), false); // projéteis não usam texturas
    
    for (const auto& projetil : projeteis) {
        if (projetil->isActive()) {
            projetil->draw(mainShader);
        }
    }
}


// realiza o disparo de um projétil a partir da posição e direção da câmera
void System::disparo() {
    vec3 projetilPos = camera.Position + camera.Front * 0.5f;  // posição inicial do projétil ligeiramente à frente da câmera
                                                                    // para evitar colisão imediata com a própria câmera
    vec3 projetilDir = camera.Front; // Retorna a direção da câmera para disparo

    auto projetil = make_unique<Projetil>(projetilPos, projetilDir, 10.0f, 5.0f); // cria um novo projétil
    projeteis.push_back(move(projetil));   // adiciona o projétil à lista de projéteis ativos
}


// Atualiza a posição dos projéteis e remove os inativos
void System::updateProjeteis() {
    for (auto& projetil : projeteis) {
        if (projetil->isActive()) {
            projetil->update(deltaTime);
        }
    }
    
    // Remove projeteis inativos
    projeteis.erase(remove_if(projeteis.begin(), projeteis.end(),
                                [](const unique_ptr<Projetil>& projetil) {
                                    return !projetil->isActive();
                                }), projeteis.end());
}


// funções de callback estáticas
void System::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}


// mouse_callback para movimentação da câmera
void System::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!systemInstance) return;
    
    if (systemInstance->firstMouse) {
        systemInstance->lastX = xpos;
        systemInstance->lastY = ypos;
        systemInstance->firstMouse = false;
    }
    
    float xoffset = xpos - systemInstance->lastX;
    float yoffset = systemInstance->lastY - ypos; // coordenadas invertidas já que y-coordinates vão de baixo para cima
    
    systemInstance->lastX = xpos;
    systemInstance->lastY = ypos;
    
    systemInstance->camera.ProcessMouseMovement(xoffset, yoffset, true);
}


// scroll_callback para zoom da câmera
void System::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (systemInstance) {
        systemInstance->camera.ProcessMouseScroll(yoffset);
    }
}


// key_callback para controle de teclas
void System::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (!systemInstance) return;
    
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            systemInstance->keys[key] = true;
        else if (action == GLFW_RELEASE)
            systemInstance->keys[key] = false;
    }
}


// Verifica colisões entre projéteis e objetos da cena - tem problema na reflexão!
void System::checkCollisions() {
    const float MIN_DISTANCE = 0.1f; // Distância mínima segura antes de verificar colisões

    for (auto& projetil : projeteis) {
        if (!projetil->isActive()) continue;
        
        // Só verifica colisões se o projétil já percorreu distância mínima
        if (projetil->lifetime < MIN_DISTANCE / projetil->speed) {
            continue;
        }

        for (auto sceneObject = sceneObjects.begin(); sceneObject != sceneObjects.end();) {
            float distance;
            
            // Calcular próxima posição do projétil para verificação de colisão
            vec3 nextPosition = projetil->position + projetil->direction * projetil->speed * deltaTime;
            
            if ((*sceneObject)->rayIntersect(projetil->position, projetil->direction, distance)) {
                // Verificar se a colisão acontecerá no próximo frame (não imediatamente)
                if (distance <= projetil->speed * deltaTime * 1.1f && distance > 0.0f) {
                    if ((*sceneObject)->isEliminable()) {
                        cout << "Objeto \"" << (*sceneObject)->name << "\" eliminado!" << endl;
                        sceneObject = sceneObjects.erase(sceneObject);
                        projetil->desativar();
                    } else {
                        // Calcular ponto de impacto mais preciso
                        vec3 hitPoint = projetil->position + projetil->direction * distance;
                        BoundingBox bbox = (*sceneObject)->getTransformedBoundingBox();
                        vec3 center = bbox.center();
                        vec3 normal = normalize(hitPoint - center);
                        
                        // Mover projétil para posição de colisão antes de refletir
                        projetil->position = hitPoint + normal * 0.01f; // Pequeno offset para evitar re-colisão
                        projetil->reflect(normal);
                        cout << "Tiro refletiu em \"" << (*sceneObject)->name << "\"!" << endl;
                        ++sceneObject;
                    }
                    break;
                } else {
                    ++sceneObject; // Colisão muito distante, continua verificando
                }
            } else {
                ++sceneObject; // Nenhuma colisão, verifica o próximo objeto
            }
        }
    }
}
