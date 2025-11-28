#include "System.h"
#include "Texture.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

// Variáveis estáticas para controle de entrada
static System* systemInstance = nullptr;
static bool tiroDisparado = false;
static bool fogTogglePressed = false;

// Grau B - Carrega configurações do sistema (câmera, luz, fog) também a partir do arquivo
// "Configurador_Sistema.txt", assim como os objetos da cena, de forma que configurações
// de camera, iluminação e fog podem ser ajustadas neste arquivo sem a necessidade de recompilar o código
System::System() : window(nullptr), 
                   camera(vec3(0.0f, 2.0f, 20.0f)), // valores padrão, serão sobrescritos
                   deltaTime(0.0f),
                   lastFrame(0.0f),
                   firstMouse(true),
                   lastX(SCREEN_WIDTH  / 2.0f),
                   lastY(SCREEN_HEIGHT / 2.0f),
                   lightPos(0.0f, 10.0f, 5.0f),
                   lightIntensity(1.0f, 1.0f, 1.0f),
                   attConstant(1.0f),
                   attLinear(0.045f),
                   attQuadratic(0.0075f),
                   fogColor(0.9f, 0.9f, 0.9f),
                   fogDensity(0.05f),
                   fogStart(10.0f),
                   fogEnd(50.0f),
                   fogType(1),
                   fogEnabled(true)
{
    systemInstance = this;
}


// Destrutor padrão
System::~System() { }


// Função de limpeza e desligamento do sistema
void System::shutdown() {
    // Fluxo de limpeza:
    // 1. Limpar objetos da cena (libera VAO e VBO de cada objeto)
    // 2. Limpar projéteis (libera recursos gráficos dos projéteis)
    // 3. Limpar cache de texturas (chama glDeleteTextures para cada textura)
    // 4. Destruir janela GLFW (destrói contexto OpenGL)
    // 5. Terminar GLFW (libera recursos da biblioteca)
    
    sceneObjects.clear(); // remove todos os objetos da cena e chama os destrutores de cada objeto
    projeteis.clear(); // remove todos os projéteis da cena e chama os destrutores de cada objeto
    
    // Limpa o cache de texturas, liberando recursos da GPU
    Texture::clearCache();
    
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();

    cout << "Sistema encerrado: objetos, texturas e recursos liberados" << endl;    cout << endl;
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
    cout << endl;

    return true;
}


// Alteramos para o Grau B - inclusão da Iluminação de Phong e mapeamento de texturas obtidas a partir do arquivo MTL
bool System::loadShaders() {
    // Código fonte do Vertex Shader com iluminação de Phong
    string vertexShaderSource = R"(
        #version 400 core
        layout (location = 0) in vec3 coordenadasDaGeometria;
        layout (location = 1) in vec2 coordenadasDaTextura;
        layout (location = 2) in vec3 coordenadasDaNormal;
        
        out vec2 textureCoord;    // Coordenadas de textura do vértice
        out vec3 elementPosition; // No VS representa a posição do vértice no world space   // antes era fragPos
        out vec3 worldNormal;     // Vetor normal no world space

        uniform mat4 model;        // Matriz que aplica as transformações ao objeto (translação, rotação, escala)
        uniform mat4 view;         // Matriz de visualização da câmera (posição, direção, etc.)
        uniform mat4 projection;   // Matriz de projeção escolhida (perspectiva ou ortográfica)
        uniform bool isProjectile; // flag para diferenciar projéteis de objetos da cena
        
        void main() {

            vec4 worldPos = model * vec4(coordenadasDaGeometria, 1.0);  // Posição dos vértices antes de view e da projeção (world space)
            elementPosition = worldPos.xyz;                             // Converte vec4 para vec3

            gl_Position = projection * view * worldPos;                 // Posição final do vértice após todas as transformações
            
            worldNormal = mat3(transpose(inverse(model))) * coordenadasDaNormal; // transforma a normal para o world space
                                                                            // usando transposta da inversa da matriz de modelo
                                                                            // fonte: LearnOpenGL.com
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
        // "elementPosition" enviará ao pipeline a posição do vértice no "world space"
        // "worldNormal"     enviará ao pipeline a normal do vértice no "world space"
		// "textureCoord"    enviará ao pipeline a textura de uma posição específica
		// "gl_Position"     é uma variável específica do GLSL que recebe a posição final do vertice processado após todas as transformações


	// Código fonte do Fragment Shader com modelo de Phong completo
    // Iluminação de Phong realiza o cálculo das componentes ambiente, difusa e especular da luz
    // utilizando as propriedades do material e da luz definidas no System e enviadas via uniforms
    // para processamento de cada fragmento no fragment shader
    string fragmentShaderSource = R"(
        #version 400 core
        out vec4 FragColor;     // Cor final do fragmento - output do Fragment Shader
        
        // Inputs do Fragment Shader - outputs do Vertex Shader
        in vec2 textureCoord;     // Coordenadas de textura
        in vec3 elementPosition;  // No FS representa a posição do fragmento // antes era fragPos
        in vec3 worldNormal;      // NORMAL INTERPOLADA pelo pipeline - // normal do fragmento
        
        // Propriedades do material
        uniform vec3 Ka;   // Coeficiente ambiente
        uniform vec3 Kd;   // Coeficiente difuso
        uniform vec3 Ks;   // Coeficiente especular
        uniform float Ns;  // Expoente especular (shininess)  
        
        // Propriedades da luz
        uniform vec3 lightPos;      // Posição da luz
        uniform vec3 lightIntensity;    // Intensidade/Cor da luz
        uniform vec3 viewPos;       // Posição da câmera
        
        // Constantes de atenuação da fonte de luz
        uniform float attConstante;  // Atenuação constante  (c1 nos slides de iluminação)
        uniform float attLinear;     // Atenuação linear     (c2 nos slides de iluminação)
        uniform float attQuadratica; // Atenuação quadrática (c3 nos slides de iluminação)
        
        // Parâmetros do fog
        uniform bool  fogEnabled;   // Flag para ligar/desligar o fog
        uniform vec3  fogColor;     // Cor do fog
        uniform float fogDensity;   // Densidade do fog (para fog exponencial)
        uniform float fogStart;     // Início do fog (para fog linear)
        uniform float fogEnd;       // Fim do fog (para fog linear)
        uniform int   fogType;      // 0=linear, 1=exponencial, 2=exponencial²
        
        // Texturas
        uniform sampler2D diffuseMap;   // Mapa de textura difusa
        uniform bool hasDiffuseMap;     // Indica se o objeto possui mapa de textura difusa
        uniform bool isProjectile;      // Indica se o objeto é um projétil
        uniform vec3 objectColor;       // Cor sólida do objeto (se não usar textura)
        
        void main() { // processamento de cada fragmento
            
            vec3 norm = normalize(worldNormal); // normaliza a WorldNormal (interpolada) para cálculos de iluminação
            
            // Cor base do material (textura ou cor sólida)
            vec3 baseColor;
            if (!isProjectile && hasDiffuseMap) { baseColor = texture(diffuseMap, textureCoord).rgb; }
            else { baseColor = objectColor; }
            
            // CÁLCULO DA ATENUAÇÃO DA LUZ -> fatt = min { 1/(c1 + c2*d + c3*d²) } de acordo com os slides
            float distance = length(lightPos - elementPosition);
            float attenuation = 1.0 / (attConstante + attLinear * distance + attQuadratica * (distance * distance));
            attenuation = min(attenuation, 1.0);    // garante que a atenuação não ultrapasse 1.0

            // CÁLCULO DA COMPONENTE AMBIENTE de acordo com os slides
            vec3 ambient = Ka * lightIntensity * baseColor;

            // CÁLCULO DA COMPONENTE DIFUSA
            vec3 lightDir = normalize(lightPos - elementPosition);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = Kd * diff * attenuation * lightIntensity * baseColor;
            
            // CÁLCULO DA COMPONENTE ESPECULAR (reflexo brilhante - não usa baseColor)
            vec3 viewDir = normalize(viewPos - elementPosition);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), Ns);
            vec3 specular = Ks * spec * attenuation * lightIntensity;
            
            // COR FINAL DO FRAGMENTO (SEM FOG) - Phong: Ambient + Diffuse + Specular
            vec3 finalFragmentColor = ambient + diffuse + specular;
            
            // CÁLCULO DO FOG (apenas se estiver habilitado)
            if (fogEnabled) {
                float fogFactor = 0.0;

                float fogDistance = length(viewPos - elementPosition);
                
                if      (fogType == 0) { fogFactor = 1 / fogDistance; } // Fog linear - antes era fogFactor = (fogEnd - fogDistance) / (fogEnd - fogStart);
                
                else if (fogType == 1) { fogFactor = exp(-fogDensity * fogDistance); } // Fog exponencial
                
                else if (fogType == 2) { fogFactor = exp(-pow(fogDensity * fogDistance, 2.0)); } // Fog exponencial ao quadrado
                
                fogFactor = clamp(fogFactor, 0.0, 1.0);  // garante que o fator fique entre 0 e 1
                
                finalFragmentColor = mix(fogColor, finalFragmentColor, fogFactor); // Interpola entre a cor do objeto e a cor do fog
            }
            
            FragColor = vec4(finalFragmentColor, 1.0); // envia a cor final do fragmento para o pipeline ()
        }
    )";
    
    // Compila e linka os shaders
    if (!mainShader.loadShaders(vertexShaderSource, fragmentShaderSource)) {
        return false;
    }
    
    return true;
}


// Carrega configurações da cena (câmera, luz, fog) do arquivo de configuração
// de cena - "Configurador_Cena.txt" - evita a necessidade de recompilar o código
// para alterar parâmetros como posição da câmera, luz e fog
bool System::loadSystemConfiguration() {

    ifstream configFile("Configurador_Cena.txt");

    if (!configFile.is_open()) {
        cerr << "Aviso: Nao foi possivel abrir Configurador_Cena.txt para configuracoes do sistema" << endl;
        return false;
    }
    
    string line;

    while (getline(configFile, line)) {

        if (line.empty() || line[0] == '#') continue;
        
        istringstream sline(line);
        string keyword;
        sline >> keyword;
        
        if (keyword == "CAMERA") {
            vec3 cameraPos;
            sline >> cameraPos.x >> cameraPos.y >> cameraPos.z;
            camera.Position = cameraPos;
            cout << "Camera configurada para a posicao: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << endl;
        }
        else if (keyword == "LIGHT") {
            sline >> lightPos.x >> lightPos.y >> lightPos.z
                  >> lightIntensity.x >> lightIntensity.y >> lightIntensity.z;
            cout << "Luz configurada para a posicao: (" << lightPos.x << ", " << lightPos.y << ", " << lightPos.z << ")";
            cout << " e Intensidade(Cor): (" << lightIntensity.x << ", " << lightIntensity.y << ", " << lightIntensity.z << ")" << endl;
        }
        else if (keyword == "ATTENUATION") {
            sline >> attConstant >> attLinear >> attQuadratic;
            cout << "Atenuacao configurada para (c1,c2,c3): (" << attConstant << ", " << attLinear << ", " << attQuadratic << ")" << endl;
        }
        else if (keyword == "FOG") {
            int enabled;
            sline >> enabled >> fogColor.x >> fogColor.y >> fogColor.z
                  >> fogDensity >> fogStart >> fogEnd >> fogType;
            fogEnabled = (enabled == 1);
            cout << "Fog configurado => Habilitado: " << (fogEnabled ? "Sim" : "Nao")
                 << " Tipo: " << fogType << " Densidade: " << fogDensity << endl;
        }
    }
    
    configFile.close();

    cout << endl;

    return true;
}


// Carrega os objetos na cena
bool System::loadSceneObjects() {
                                                  // na apresentação: ver readObjectsInfos() (está logo abaixo)
    auto sceneObjectsInfos = readObjectsInfos();  // a partir do arquivo de configuração, lê as configurações gerais
                                                  // dos objetos da cena (nome, path do modelo, posição, rotação, escala, eliminável)
                                                  // e retorna um vetor (sceneObjectsInfo) de estruturas ObjectInfo

    for (auto& sceneObject : sceneObjectsInfos) { // loop para processar cada configuração de objeto lida do arquivo "configurador_cena.txt"

        auto object = make_unique<Object3D>(sceneObject.name);  // cria um novo objeto 3D com o nome especificado
                                                                // no arquivo de configuração lido acima e armazenado na estrutura sceneObject

        // Tenta carregar o modelo (arquivo .obj), se falhar não adiciona o objeto à cena
        // Grau B: As texturas agora são carregadas através dos arquivos de materiais MTL

        if (object->loadObject(sceneObject.modelPath)) {   // tenta carregar o modelo 3D do arquivo .obj através da sequencia de métodos:
                                                           // Object3D::loadObject -> Mesh::readObjectModel -> OBJReader::readFileOBJ
                                                           // que por sua vez chama Mesh::(string& path)
                                                           // se carregar com sucesso, prossegue para configurar o objeto com os demais parâmetros
            object->setPosition(sceneObject.position);     // posiciona o objeto na cena
            object->setRotation(sceneObject.rotation);     // rotaciona o objeto na cena
            object->baseRotation = sceneObject.rotation;   // guarda rotação inicial para animação
            object->setScale(sceneObject.scale);           // escala o objeto na cena
            object->setEliminable(sceneObject.eliminable); // define se o objeto pode ser eliminado ou não

            // Se o objeto é o veículo, carrega a curva de animação 
            if (sceneObject.name == "Veiculo" || sceneObject.name == "Conversivel") {
                // Busca os parâmetros da pista para aplicar à curva
                vec3 trackPos(0.0f), trackRot(0.0f), trackScale(1.0f);
                for (const auto& obj : sceneObjectsInfos) {
                    if (obj.name == "Pista") {
                        trackPos = obj.position;    // posição da pista será aplicada à curva do Veículo
                        trackRot = obj.rotation;    // rotação da pista será aplicada à curva do Veículo
                        trackScale = obj.scale;     // escala da pista será aplicada à curva do Veículo
                        break;
                    }
                }
                
                // Carrega a curva de animação do veículo aplicando os parâmetros da pista (posição, rotação e escala)
                if (object->loadAnimationCurve("models/curva_BSpline.txt", trackPos, trackRot, trackScale)) {
                    object->setAnimationSpeed(4.0f); // Velocidade da animação
                    //cout << "Animacao carregada para o " << sceneObject.name << endl;
                }
            }

            sceneObjects.push_back(move(object));   // adiciona o objeto 3D criado à lista de objetos 3D da cena
                                                    // o vetor sceneObjects é um atributo da classe System, gerado no arquivo System.h
        }
        else {
            cout << "Falha ao carregar objeto " << sceneObject.name
                 << " de " << sceneObject.modelPath << endl;
        }

        cout << endl;
    }

    return true;
}


// Carrega em um vetor as informações gerais (nome, path do modelo, posição, rotação, escala, eliminável)
// dos objetos da cena a partir do arquivo de configuração da cena - "Configurador_Cena.txt".
// No Grau A era: (Nome Path posX posY posZ rotX rotY rotZ scaleX scaleY scaleZ Eliminável(S/N) TexturePath)
// Agora no Grau B ficou: (Nome Path posX posY posZ rotX rotY rotZ scaleX scaleY scaleZ Eliminável(S/N))
// uma vez que as texturas agora são carregadas automaticamente através da referência do arquivo .mtl
vector<ObjectInfo> System::readObjectsInfos() {

    vector<ObjectInfo> sceneObjectsInfos;  // ObjectInfo é uma estrutura para armazenar informações sobre um determinado objeto 3D
                                           // sceneObjectsInfo é um vetor que armazena várias dessas estruturas (qtd = nº de objetos da cena)

    ifstream configFile("Configurador_Cena.txt");   // abre o arquivo de configuração para leitura

    string line;  // variável temporária para armazenar cada linha lida do arquivo de configuração

    while (getline(configFile, line)) { // loop para processar cada linha do arquivo de configuração
        
        if (line.empty() || line[0] == '#') continue; // Ignora linhas vazias ou comentários

        istringstream sline(line);  // Cria um stream (sline) a partir da linha lida
        
        string firstWord;
        sline >> firstWord; // Lê a primeira palavra da linha para verificar se é uma configuração do sistema

        if (firstWord == "CAMERA" || firstWord == "LIGHT" || 
            firstWord == "ATTENUATION" || firstWord == "FOG") {
            continue;       // Ignora linhas de configuração do sistema
        }

        ObjectInfo objectInfo;  // instancia estrutura para armazenar informações do objeto descrito na linha processada
                                // Nome Path posX posY posZ rotX rotY rotZ scaleX scaleY scaleZ eliminável

        // carrega os dados da linha para o respectivo campo da estrutura objectInfo
        objectInfo.name = firstWord;    // nome que o objeto terá na cena, já lido acima
        sline >> objectInfo.modelPath   // caminho do modelo 3D (.obj)
              >> objectInfo.position.x  // posição X, inicial, do objeto na cena
              >> objectInfo.position.y  // posição Y, inicial, do objeto na cena
              >> objectInfo.position.z  // posição Z, inicial, do objeto na cena
              >> objectInfo.rotation.x  // rotação no eixo X, inicial, do objeto na cena (em graus)
              >> objectInfo.rotation.y  // rotação no eixo Y, inicial, do objeto na cena (em graus)
              >> objectInfo.rotation.z  // rotação no eixo Z, inicial, do objeto na cena (em graus)
              >> objectInfo.scale.x     // escala no eixo X, inicial, do objeto na cena
              >> objectInfo.scale.y     // escala no eixo Y, inicial, do objeto na cena
              >> objectInfo.scale.z     // escala no eixo Z, inicial, do objeto na cena
              >> objectInfo.eliminable; // se o objeto pode ser eliminado (1 = sim, 0 = não)
        
        // Converte rotações de graus para radianos
        objectInfo.rotation.x = glm::radians(objectInfo.rotation.x);
        objectInfo.rotation.y = glm::radians(objectInfo.rotation.y);
        objectInfo.rotation.z = glm::radians(objectInfo.rotation.z);
              //>> objectInfo.texturePath;  // caminho da textura do objeto (Grau A - removido no Grau B)
                                            // agora as texturas são carregadas automaticamente através do arquivo .mtl associado ao .obj

        sceneObjectsInfos.push_back(objectInfo); // adiciona a estrutura recém preenchida ao vetor de configurações
    }                                           // vetor de estruturas ObjectInfo
    
    configFile.close();

    // debug: informa se objetos foram carregados
    if (sceneObjectsInfos.empty()) { cerr << "Nenhum objeto carregado: verifique o arquivo Configurador_Cena.txt" << endl; }
    else { cout << sceneObjectsInfos.size() << " objetos encontradas no arquivo de configuracao de cena." << endl; }
    cout << endl;

    return sceneObjectsInfos;
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
    
    vec3 bgColor = fogEnabled ? fogColor : vec3(0.85f, 1.0f, 0.85f); // Usa a cor do fog como cor de fundo quando fog estiver ativo

    // Limpa o buffer de cor e o buffer de profundidade
    glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f); // define a cor de fundo
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // limpa os buffers

    // Calcula a matriz de projeção - perspective(FOV, razão de aspecto, Near, Far) - razão de aspecto = largura/altura
    mat4 projection = perspective(radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

    // Calcula a matriz de visualização - lookAt(posição da câmera, ponto para onde a câmera está olhando, vetor up da câmera)
    mat4 view = camera.GetViewMatrix(); // lookAt(Position, Position + Front, Up)
    
    // Ativa o programa de shader
    if (mainShader.ID != 0) { glUseProgram(mainShader.ID); }
    
    // Configura uniforms de transformação
    glUniformMatrix4fv(glGetUniformLocation(mainShader.ID, "projection"), 1, GL_FALSE, value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(mainShader.ID, "view"), 1, GL_FALSE, value_ptr(view));
    
    // Configura uniforms de iluminação (modelo de Phong completo)
    glUniform3fv(glGetUniformLocation(mainShader.ID, "lightPos"), 1, value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(mainShader.ID, "lightIntensity"), 1, value_ptr(lightIntensity));
    glUniform3fv(glGetUniformLocation(mainShader.ID, "viewPos"), 1, value_ptr(camera.Position));
    
    // Configura coeficientes de atenuação atmosférica
    glUniform1f(glGetUniformLocation(mainShader.ID, "attConstant"), attConstant);
    glUniform1f(glGetUniformLocation(mainShader.ID, "attLinear"), attLinear);
    glUniform1f(glGetUniformLocation(mainShader.ID, "attQuadratic"), attQuadratic);
    
    // Configura parâmetros do fog
    glUniform1i(glGetUniformLocation(mainShader.ID, "fogEnabled"), fogEnabled);
    glUniform3fv(glGetUniformLocation(mainShader.ID, "fogColor"), 1, value_ptr(fogColor));
    glUniform1f(glGetUniformLocation(mainShader.ID, "fogDensity"), fogDensity);
    glUniform1f(glGetUniformLocation(mainShader.ID, "fogStart"), fogStart);
    glUniform1f(glGetUniformLocation(mainShader.ID, "fogEnd"), fogEnd);
    glUniform1i(glGetUniformLocation(mainShader.ID, "fogType"), fogType);
    
    // Configura propriedades do material (valores padrão, podem ser alterados por objeto)
    glUniform3f(glGetUniformLocation(mainShader.ID, "Ka"), 0.1f, 0.1f, 0.1f); // coeficiente ambiente
    glUniform3f(glGetUniformLocation(mainShader.ID, "Kd"), 0.8f, 0.8f, 0.8f); // coeficiente difuso
    glUniform3f(glGetUniformLocation(mainShader.ID, "Ks"), 1.0f, 1.0f, 1.0f); // coeficiente especular
    glUniform1f(glGetUniformLocation(mainShader.ID, "Ns"), 32.0f);            // expoente especular (shininess)
    glUniform1i(glGetUniformLocation(mainShader.ID, "isProjectile"), false);  // objetos da cena não são projéteis
    glUniform3f(glGetUniformLocation(mainShader.ID, "objectColor"), 1.0f, 1.0f, 1.0f);
    
    for (const auto& sceneObject : sceneObjects) { // renderiza cada objeto da cena
        sceneObject->render(mainShader);
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


// Atualiza as animações dos objetos
void System::updateAnimations() {
    for (auto& obj : sceneObjects) {
        obj->updateAnimation(deltaTime);
    }
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
            // Ignora colisão com a pista
            if ((*sceneObject)->name == "Pista") {
                ++sceneObject;
                continue;
            }
            
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