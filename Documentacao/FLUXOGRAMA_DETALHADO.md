# Fluxograma Detalhado do Programa
## Visualizador 3D - Grau B - Expansão dos Blocos Principais

---

## 📋 Índice

1. [Fase 1: Inicialização do Sistema](#fase-1-inicialização-do-sistema)
2. [Fase 2: Loop Principal (Game Loop)](#fase-2-loop-principal-game-loop)
3. [Fase 3: Finalização](#fase-3-finalização)
4. [Fluxos Especializados](#fluxos-especializados)

---

## Fase 1: Inicialização do Sistema

### Visão Geral da Inicialização

```mermaid
graph TB
    Start[INICIALIZAÇÃO] --> Step1
    Step1[1. Inicializa GLFW<br/> - Cria janela 1024x768<br/> - Configura callbacks] --> Step2
    Step2[2. Inicializa OpenGL<br/> - Carrega funções GLAD<br/> - Ativa Depth Test, Blending] --> Step3
    Step3[3. Carrega Shaders<br/> - Vertex Shader<br/> - Fragment Shader] --> Step4
    Step4[4. Carrega Configurações<br/> - Câmera posição, FOV<br/> - Luz posição, intensidade<br/> - Fog cor, densidade] --> Step5
    Step5[5. Carrega Objetos 3D<br/> - Lê Configurador_Cena.txt<br/> - Carrega modelos .obj<br/> - Cria VBO, VAO, EBO] --> End[Sistema Pronto]
```

---

### 1️⃣ Inicialização GLFW (System::initializeGLFW)

```mermaid
graph TB
    Start[initializeGLFW] --> Init[glfwInit]
    Init --> Hints[Configura Window Hints]
    Hints --> Version[OpenGL 4.6 Core Profile]
    Version --> MSAA[Ativa MSAA 4x antialiasing]
    MSAA --> Create[glfwCreateWindow 1024x768]
    Create --> Check{Janela<br/> criada?}
    Check -->|Não| Error[Retorna false]
    Check -->|Sim| Context[glfwMakeContextCurrent]
    Context --> Callback1[framebuffer_size_callback]
    Callback1 --> Callback2[mouse_callback]
    Callback2 --> Callback3[scroll_callback]
    Callback3 --> Cursor[Desabilita cursor<br/> GLFW_CURSOR_DISABLED]
    Cursor --> Success[Retorna true]
```

**Detalhes:**
- **glfwInit()**: Inicializa biblioteca GLFW
- **Window Hints**: Define versão OpenGL 4.6, Core Profile
- **MSAA**: Ativa antialiasing 4x para suavizar bordas
- **Callbacks**: Registra funções para eventos (resize, mouse, scroll)
- **Cursor Mode**: Captura mouse para controle de câmera FPS

---

### 2️⃣ Inicialização OpenGL (System::initializeOpenGL)

```mermaid
graph TB
    Start[initializeOpenGL] --> GLAD[gladLoadGLLoader]
    GLAD --> Check{GLAD<br/> OK?}
    Check -->|Não| Error[Retorna false]
    Check -->|Sim| Viewport[glViewport 0,0,1024,768]
    Viewport --> Depth[glEnable GL_DEPTH_TEST]
    Depth --> Blend[glEnable GL_BLEND]
    Blend --> BlendFunc[glBlendFunc<br/> GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA]
    BlendFunc --> MSAA[glEnable GL_MULTISAMPLE]
    MSAA --> Cull[glEnable GL_CULL_FACE<br/> glCullFace GL_BACK]
    Cull --> Success[Retorna true]
```

**Detalhes:**
- **GLAD**: Carrega ponteiros de funções OpenGL
- **Viewport**: Define área de renderização
- **Depth Test**: Ativa teste de profundidade (Z-buffer)
- **Blending**: Transparência com alpha blending
- **MSAA**: Ativa multisample antialiasing
- **Culling**: Remove faces traseiras (otimização)

---

### 3️⃣ Carregamento de Shaders (System::loadShaders)

```mermaid
graph TB
    Start[loadShaders] --> ReadVS[Lê arquivo Vertex Shader]
    ReadVS --> ReadFS[Lê arquivo Fragment Shader]
    ReadFS --> CompileVS[Compila Vertex Shader]
    CompileVS --> CheckVS{Compilação<br/> OK?}
    CheckVS -->|Não| ErrorVS[Exibe log de erro<br/> Retorna false]
    CheckVS -->|Sim| CompileFS[Compila Fragment Shader]
    CompileFS --> CheckFS{Compilação<br/> OK?}
    CheckFS -->|Não| ErrorFS[Exibe log de erro<br/> Retorna false]
    CheckFS -->|Sim| Link[glLinkProgram]
    Link --> CheckLink{Link<br/> OK?}
    CheckLink -->|Não| ErrorLink[Exibe log de erro<br/> Retorna false]
    CheckLink -->|Sim| Delete[Deleta shaders individuais]
    Delete --> Success[Retorna true]
```

**Shaders do Sistema:**
- **Vertex Shader**: Transforma vértices (Model-View-Projection)
- **Fragment Shader**: Calcula cor final com Phong + Fog

---

### 4️⃣ Carregamento de Configurações (System::loadSystemConfiguration)

```mermaid
graph TB
    Start[loadSystemConfiguration] --> Open[Abre Configurador_Cena.txt]
    Open --> Check{Arquivo<br/> aberto?}
    Check -->|Não| Error[Retorna false]
    Check -->|Sim| Loop{Próxima<br/> linha}
    Loop -->|EOF| Success[Retorna true]
    Loop -->|Linha| Parse[Parse linha]
    
    Parse --> Cam{camera:?}
    Cam -->|Sim| CamData[Extrai posição x,y,z<br/> Extrai front x,y,z<br/> Extrai FOV]
    CamData --> Loop
    
    Parse --> Light{light:?}
    Light -->|Sim| LightData[Extrai posição x,y,z<br/> Extrai intensidade RGB<br/> Extrai atenuação k_c,k_l,k_q]
    LightData --> Loop
    
    Parse --> Fog{fog:?}
    Fog -->|Sim| FogData[Extrai cor RGB<br/> Extrai densidade<br/> Extrai start/end<br/> Extrai tipo 0-2<br/> Extrai enabled]
    FogData --> Loop
    
    Parse --> Other[Ignora linha]
    Other --> Loop
```

**Formato do Arquivo:**
```
camera: pos(0,2,20) front(0,0,-1) fov(45)
light: pos(0,10,5) intensity(1,1,1) att(1,0.045,0.0075)
fog: color(0.9,0.9,0.9) density(0.05) range(10,50) type(1) enabled(true)
```

---

### 5️⃣ Carregamento de Objetos 3D (System::loadSceneObjects)

```mermaid
graph TB
    Start[loadSceneObjects] --> Read[readObjectsInfos<br/> Lê Configurador_Cena.txt]
    Read --> Loop{Para cada<br/> ObjectInfo}
    Loop -->|Próximo| Create[Cria unique_ptr Object3D]
    Create --> Load[object->loadObject]
    Load --> Check{Carregamento<br/> OK?}
    Check -->|Não| Error[Exibe erro<br/> Retorna false]
    Check -->|Sim| Transform[Aplica transformações<br/> posição, rotação, escala]
    Transform --> Add[Adiciona a sceneObjects]
    Add --> Loop
    Loop -->|Fim| Success[Retorna true]
```

#### 5.1. Leitura do Arquivo de Configuração (readObjectsInfos)

```mermaid
graph TB
    Start[readObjectsInfos] --> Open[Abre Configurador_Cena.txt]
    Open --> Vector[Cria vector ObjectInfo]
    Vector --> Loop{Próxima<br/> linha}
    Loop -->|EOF| Return[Retorna vector]
    Loop -->|Linha| Check{Começa com<br/> object:?}
    Check -->|Não| Loop
    Check -->|Sim| Parse[Parse parâmetros]
    Parse --> Name[Extrai name]
    Name --> Model[Extrai modelPath]
    Model --> Pos[Extrai position x,y,z]
    Pos --> Rot[Extrai rotation x,y,z]
    Rot --> Scale[Extrai scale x,y,z]
    Scale --> Elim[Extrai eliminable true/false]
    Elim --> Create[Cria ObjectInfo]
    Create --> Add[Adiciona ao vector]
    Add --> Loop
```

**Formato do Arquivo:**
```
object: name(Pista) model(pista.obj) pos(0,-0.5,0) rot(0,0,0) scale(1,1,1) elim(false)
object: name(Carro) model(car.obj) pos(0,0,0) rot(0,90,0) scale(1,1,1) elim(true)
```

#### 5.2. Carregamento de Modelo OBJ (Object3D::loadObject → Mesh::readObjectModel)

```mermaid
graph TB
    Start[Object3D::loadObject] --> CreateMesh[Cria unique_ptr Mesh]
    CreateMesh --> ReadModel[mesh->readObjectModel]
    ReadModel --> OBJReader[OBJReader::readFileOBJ]
    
    OBJReader --> OpenOBJ[Abre arquivo .obj]
    OpenOBJ --> InitVectors[Inicializa vectors<br/> vertices, texCoords, normals, groups]
    InitVectors --> LoopLines{Próxima<br/> linha}
    
    LoopLines -->|mtllib| MTL[readFileMTL<br/> Carrega materiais]
    MTL --> LoopLines
    
    LoopLines -->|v| Vertex[parseVertice<br/> Adiciona vec3 vertices]
    Vertex --> LoopLines
    
    LoopLines -->|vt| TexCoord[parseTexCoord<br/> Adiciona vec2 texCoords]
    TexCoord --> LoopLines
    
    LoopLines -->|vn| Normal[parseNormal<br/> Adiciona vec3 normals]
    Normal --> LoopLines
    
    LoopLines -->|g/o| Group[Cria novo Group<br/> Atribui material atual]
    Group --> LoopLines
    
    LoopLines -->|usemtl| Material[Define material atual<br/> Atribui ao grupo]
    Material --> LoopLines
    
    LoopLines -->|f| Face[parseFace<br/> Adiciona Face ao grupo]
    Face --> LoopLines
    
    LoopLines -->|EOF| Setup[setupMesh<br/> Cria VAO, VBO, EBO]
    Setup --> Return[Retorna true]
```

#### 5.3. Leitura de Arquivo MTL (OBJReader::readFileMTL)

```mermaid
graph TB
    Start[readFileMTL] --> Open[Abre arquivo .mtl]
    Open --> Loop{Próxima<br/> linha}
    Loop -->|EOF| Return[Retorna]
    
    Loop -->|newmtl| NewMat[Cria novo Material<br/> Adiciona ao map]
    NewMat --> Loop
    
    Loop -->|Ka| Ambient[Parse Ka R G B<br/> material.ambient]
    Ambient --> Loop
    
    Loop -->|Kd| Diffuse[Parse Kd R G B<br/> material.diffuse]
    Diffuse --> Loop
    
    Loop -->|Ks| Specular[Parse Ks R G B<br/> material.specular]
    Specular --> Loop
    
    Loop -->|Ns| Shininess[Parse Ns valor<br/> material.shininess]
    Shininess --> Loop
    
    Loop -->|d| Alpha[Parse d valor<br/> material.alpha]
    Alpha --> Loop
    
    Loop -->|map_Kd| Texture[Parse caminho textura<br/> Texture::loadFromFile<br/> material.textureID]
    Texture --> Loop
    
    Loop -->|Outro| Ignore[Ignora linha]
    Ignore --> Loop
```

#### 5.4. Setup de Malha OpenGL (Mesh::setupMesh)

```mermaid
graph TB
    Start[setupMesh] --> Organize[Organiza dados por grupo<br/> vertices, normais, UVs]
    Organize --> GenVAO[glGenVertexArrays<br/> Cria VAO]
    GenVAO --> BindVAO[glBindVertexArray VAO]
    BindVAO --> GenVBO[glGenBuffers<br/> Cria VBO]
    GenVBO --> BindVBO[glBindBuffer GL_ARRAY_BUFFER]
    BindVBO --> BufferData[glBufferData<br/> Envia vértices para GPU]
    BufferData --> GenEBO[glGenBuffers<br/> Cria EBO]
    GenEBO --> BindEBO[glBindBuffer GL_ELEMENT_ARRAY_BUFFER]
    BindEBO --> BufferIndices[glBufferData<br/> Envia índices para GPU]
    
    BufferIndices --> Attr0[glVertexAttribPointer 0<br/> Posição xyz]
    Attr0 --> Enable0[glEnableVertexAttribArray 0]
    Enable0 --> Attr1[glVertexAttribPointer 1<br/> Normal xyz]
    Attr1 --> Enable1[glEnableVertexAttribArray 1]
    Enable1 --> Attr2[glVertexAttribPointer 2<br/> TexCoord uv]
    Attr2 --> Enable2[glEnableVertexAttribArray 2]
    
    Enable2 --> Unbind[glBindVertexArray 0<br/> Desvincula VAO]
    Unbind --> Return[Retorna]
```

**Layout dos Atributos de Vértice:**
```
layout(location = 0) in vec3 aPos;      // Posição
layout(location = 1) in vec3 aNormal;   // Normal
layout(location = 2) in vec2 aTexCoord; // Coordenada de textura
```

---

## Fase 2: Loop Principal (Game Loop)

### Visão Geral do Loop

```mermaid
graph TB
    Start([Loop Iniciado]) --> A[A. Calcula deltaTime]
    A --> B[B. Processa Input]
    B --> C[C. Atualiza Animações]
    C --> D[D. Atualiza Projéteis]
    D --> E[E. Verifica Colisões]
    E --> F[F. Renderiza Cena]
    F --> G[G. Swap Buffers & Poll Events]
    G --> H{Janela<br/> fechada?}
    H -->|Não| A
    H -->|Sim| End([Sai do Loop])
```

---

### A. Cálculo de DeltaTime

```mermaid
graph LR
    Start[Início do Frame] --> GetTime[currentFrame = glfwGetTime]
    GetTime --> Calc[deltaTime = currentFrame - lastFrame]
    Calc --> Update[lastFrame = currentFrame]
    Update --> Use[Usa deltaTime para<br/> movimentação suave]
```

**Propósito:** Garante movimentação independente de FPS
- **60 FPS**: deltaTime ≈ 0.0167s
- **30 FPS**: deltaTime ≈ 0.0333s

---

### B. Processamento de Input (System::processInput)

```mermaid
graph TB
    Start[processInput] --> ESC{ESC<br/> pressionado?}
    ESC -->|Sim| Close[glfwSetWindowShouldClose true]
    ESC -->|Não| W
    
    Close --> W
    
    W{W ou UP<br/> pressionado?} -->|Sim| MoveF[camera.ProcessKeyboard FORWARD]
    W -->|Não| S
    MoveF --> S
    
    S{S ou DOWN<br/> pressionado?} -->|Sim| MoveB[camera.ProcessKeyboard BACKWARD]
    S -->|Não| A
    MoveB --> A
    
    A{A ou LEFT<br/> pressionado?} -->|Sim| MoveL[camera.ProcessKeyboard LEFT]
    A -->|Não| D
    MoveL --> D
    
    D{D ou RIGHT<br/> pressionado?} -->|Sim| MoveR[camera.ProcessKeyboard RIGHT]
    D -->|Não| Space
    MoveR --> Space
    
    Space{SPACE<br/> pressionado?} -->|Sim| CheckShot{Tiro não<br/> disparado?}
    Space -->|Não| F
    CheckShot -->|Sim| Shoot[disparo<br/> tiroDisparado = true]
    CheckShot -->|Não| F
    Shoot --> F
    
    F{F<br/> pressionado?} -->|Sim| CheckToggle{fogToggle não<br/> pressionado?}
    F -->|Não| Released
    CheckToggle -->|Sim| Toggle[fogEnabled = !fogEnabled<br/> fogTogglePressed = true]
    CheckToggle -->|Não| Released
    Toggle --> Released
    
    Released{F<br/> liberado?} -->|Sim| Reset[fogTogglePressed = false]
    Released -->|Não| End
    Reset --> End[Fim]
```

**Callbacks de Mouse:**

#### Mouse Movement (mouse_callback)

```mermaid
graph TB
    Start[mouse_callback] --> First{firstMouse?}
    First -->|Sim| Init[lastX = xpos<br/> lastY = ypos<br/> firstMouse = false]
    First -->|Não| Calc
    Init --> Return
    
    Calc[xoffset = xpos - lastX<br/> yoffset = lastY - ypos]
    Calc --> Update[lastX = xpos<br/> lastY = ypos]
    Update --> Process[camera.ProcessMouseMovement<br/> xoffset, yoffset]
    Process --> Return[Retorna]
```

#### Scroll (scroll_callback)

```mermaid
graph TB
    Start[scroll_callback] --> Process[camera.ProcessMouseScroll<br/> yoffset]
    Process --> Return[Retorna]
```

---

### C. Atualização de Animações (System::updateAnimations)

```mermaid
graph TB
    Start[updateAnimations] --> Loop{Para cada<br/> sceneObject}
    Loop -->|Próximo| Check{Objeto tem<br/> animação?}
    Check -->|Não| Loop
    Check -->|Sim| Type{Tipo de<br/> animação}
    
    Type -->|Rotação| Rotate[Incrementa ângulo<br/> baseado em deltaTime]
    Rotate --> Apply1[Aplica transformação]
    Apply1 --> Loop
    
    Type -->|Translação| Translate[Atualiza posição<br/> baseado em velocidade]
    Translate --> Apply2[Aplica transformação]
    Apply2 --> Loop
    
    Type -->|Escala| Scale[Atualiza escala<br/> baseado em tempo]
    Scale --> Apply3[Aplica transformação]
    Apply3 --> Loop
    
    Loop -->|Fim| Return[Retorna]
```

---

### D. Atualização de Projéteis (System::updateProjeteis)

```mermaid
graph TB
    Start[updateProjeteis] --> Loop{Para cada<br/> projétil}
    Loop -->|Próximo| Update[posição += velocidade * deltaTime]
    Update --> CheckBounds{Fora dos<br/> limites?}
    CheckBounds -->|Sim| Remove[Remove projétil<br/> do vector]
    CheckBounds -->|Não| Loop
    Remove --> Loop
    Loop -->|Fim| Return[Retorna]
```

**Limites de Verificação:**
- Distância máxima da origem (ex: 100 unidades)
- Tempo de vida máximo (ex: 5 segundos)

---

### E. Verificação de Colisões (System::checkCollisions)

```mermaid
graph TB
    Start[checkCollisions] --> LoopP{Para cada<br/> projétil}
    LoopP -->|Próximo| LoopO{Para cada<br/> objeto}
    
    LoopO -->|Próximo| CheckElim{Objeto<br/> eliminável?}
    CheckElim -->|Não| LoopO
    CheckElim -->|Sim| CalcDist[Calcula distância<br/> projétil ↔ objeto]
    
    CalcDist --> CheckDist{Distância <<br/> raio colisão?}
    CheckDist -->|Não| LoopO
    CheckDist -->|Sim| Collision[COLISÃO DETECTADA]
    
    Collision --> RemoveObj[Remove objeto de sceneObjects]
    RemoveObj --> RemoveProj[Remove projétil de projeteis]
    RemoveProj --> LoopP
    
    LoopO -->|Fim| LoopP
    LoopP -->|Fim| Return[Retorna]
```

**Detecção de Colisão:**
- Tipo: Esfera-Esfera (bounding sphere)
- Fórmula: $|P_{proj} - P_{obj}| < (R_{proj} + R_{obj})$

---

### F. Renderização da Cena (System::render)

```mermaid
graph TB
    Start[render] --> Clear[glClear<br/> COLOR_BUFFER_BIT<br/> DEPTH_BUFFER_BIT]
    Clear --> UseShader[mainShader.use]
    UseShader --> CalcView[view = camera.GetViewMatrix]
    CalcView --> CalcProj[projection = perspective<br/> FOV, aspect, near, far]
    
    CalcProj --> SendView[uniform mat4 view]
    SendView --> SendProj[uniform mat4 projection]
    SendProj --> SendCamPos[uniform vec3 viewPos]
    
    SendCamPos --> SendLight[uniform vec3 lightPos<br/> uniform vec3 lightIntensity<br/> uniform float attConstants]
    SendLight --> SendFog[uniform vec3 fogColor<br/> uniform float fogDensity<br/> uniform int fogType<br/> uniform bool fogEnabled]
    
    SendFog --> LoopObj{Para cada<br/> sceneObject}
    LoopObj -->|Próximo| CalcModel[model = translate * rotate * scale]
    CalcModel --> SendModel[uniform mat4 model]
    SendModel --> Render[object->render mainShader]
    Render --> LoopObj
    
    LoopObj -->|Fim| LoopProj{Para cada<br/> projétil}
    LoopProj -->|Próximo| CalcModelP[model = translate * scale]
    CalcModelP --> SendModelP[uniform mat4 model]
    SendModelP --> RenderP[projetil->render mainShader]
    RenderP --> LoopProj
    
    LoopProj -->|Fim| Return[Retorna]
```

#### F.1. Renderização de Objeto Individual (Object3D::render)

```mermaid
graph TB
    Start[Object3D::render] --> Loop{Para cada<br/> grupo da malha}
    Loop -->|Próximo| GetMat[Obtém material do grupo]
    GetMat --> SendMat[Envia uniforms do material<br/> Ka, Kd, Ks, Ns, alpha]
    SendMat --> CheckTex{Tem<br/> textura?}
    CheckTex -->|Sim| BindTex[glBindTexture<br/> uniform sampler2D texture1<br/> uniform bool hasTexture true]
    CheckTex -->|Não| NoTex[uniform bool hasTexture false]
    BindTex --> Draw
    NoTex --> Draw[glBindVertexArray VAO<br/> glDrawElements<br/> GL_TRIANGLES, count, indices]
    Draw --> Loop
    Loop -->|Fim| Unbind[glBindVertexArray 0<br/> glBindTexture 0]
    Unbind --> Return[Retorna]
```

---

### G. Swap Buffers & Poll Events

```mermaid
graph TB
    Start[Fim do Frame] --> Swap[glfwSwapBuffers<br/> Exibe frame renderizado]
    Swap --> Poll[glfwPollEvents<br/> Processa eventos pendentes]
    Poll --> Return[Retorna ao início do loop]
```

**Double Buffering:**
- **Front Buffer**: Exibido na tela
- **Back Buffer**: Sendo renderizado
- Troca evita flickering (tremor visual)

---

## Fase 3: Finalização

### System::shutdown

```mermaid
graph TB
    Start[shutdown] --> ClearObj[sceneObjects.clear<br/> Chama destrutores Object3D]
    ClearObj --> ClearProj[projeteis.clear<br/> Chama destrutores Projetil]
    ClearProj --> ClearTex[Texture::clearCache<br/> glDeleteTextures]
    ClearTex --> CheckWin{window<br/> != nullptr?}
    CheckWin -->|Sim| DestroyWin[glfwDestroyWindow]
    CheckWin -->|Não| Terminate
    DestroyWin --> SetNull[window = nullptr]
    SetNull --> Terminate[glfwTerminate]
    Terminate --> Log[cout mensagem de encerramento]
    Log --> End[Retorna]
```

### Destrutor Object3D::~Object3D

```mermaid
graph TB
    Start[~Object3D] --> CheckMesh{mesh<br/> != nullptr?}
    CheckMesh -->|Sim| DestroyMesh[~Mesh<br/> Deleta VAO, VBO, EBO]
    CheckMesh -->|Não| End
    DestroyMesh --> End[Retorna]
```

### Destrutor Mesh::~Mesh

```mermaid
graph TB
    Start[~Mesh] --> CheckVAO{VAO != 0?}
    CheckVAO -->|Sim| DeleteVAO[glDeleteVertexArrays VAO]
    CheckVAO -->|Não| CheckVBO
    DeleteVAO --> CheckVBO{VBO != 0?}
    CheckVBO -->|Sim| DeleteVBO[glDeleteBuffers VBO]
    CheckVBO -->|Não| CheckEBO
    DeleteVBO --> CheckEBO{EBO != 0?}
    CheckEBO -->|Sim| DeleteEBO[glDeleteBuffers EBO]
    CheckEBO -->|Não| End
    DeleteEBO --> End[Retorna]
```

---

## Fluxos Especializados

### Disparo de Projétil (System::disparo)

```mermaid
graph TB
    Start[disparo] --> Create[Cria unique_ptr Projetil]
    Create --> SetPos[posição = camera.Position]
    SetPos --> SetDir[direção = camera.Front]
    SetDir --> SetVel[velocidade = direção * speed]
    SetVel --> Load[projetil->loadObject<br/> Carrega modelo .obj]
    Load --> Check{Carregamento<br/> OK?}
    Check -->|Não| Error[cout erro<br/> Retorna]
    Check -->|Sim| Add[Adiciona a projeteis vector]
    Add --> Return[Retorna]
```

---

### Carregamento de Textura (Texture::loadFromFile)

```mermaid
graph TB
    Start[loadFromFile] --> CheckCache{Textura já<br/> em cache?}
    CheckCache -->|Sim| ReturnCached[Retorna textureID do cache]
    CheckCache -->|Não| Load[stbi_load<br/> Carrega imagem]
    
    Load --> CheckLoad{Imagem<br/> carregada?}
    CheckLoad -->|Não| Error[cout erro<br/> Retorna 0]
    CheckLoad -->|Sim| GenTex[glGenTextures]
    
    GenTex --> Bind[glBindTexture GL_TEXTURE_2D]
    Bind --> TexImage[glTexImage2D<br/> Envia dados para GPU]
    TexImage --> Mipmap[glGenerateMipmap]
    
    Mipmap --> Params[glTexParameteri<br/> WRAP_S, WRAP_T<br/> MIN_FILTER, MAG_FILTER]
    Params --> Free[stbi_image_free]
    Free --> Cache[Adiciona ao cache]
    Cache --> ReturnNew[Retorna textureID]
```

**Parâmetros de Textura:**
- **Wrap**: GL_REPEAT (azulejos)
- **Filter**: GL_LINEAR_MIPMAP_LINEAR (trilinear)

---

### Pipeline de Renderização OpenGL

```mermaid
graph TB
    Start[Vértices na CPU] --> VBO[Upload para VBO GPU]
    VBO --> VS[Vertex Shader<br/> Transformação MVP]
    VS --> Clip[Clipping Frustum]
    Clip --> Raster[Rasterização<br/> Primitivas → Fragmentos]
    Raster --> FS[Fragment Shader<br/> Iluminação Phong + Fog]
    FS --> DepthTest[Depth Test<br/> Z-buffer]
    DepthTest --> BlendTest[Blend Test<br/> Alpha blending]
    BlendTest --> FB[Frame Buffer]
    FB --> Screen[Tela]
```

---

### Cálculo de Iluminação Phong (Fragment Shader)

```mermaid
graph TB
    Start[Para cada fragmento] --> Ambient[Ambiente<br/> I_a = Ka * lightIntensity]
    
    Start --> Diffuse[Difusa Lambert<br/> N · L = max dot normal, lightDir, 0<br/> I_d = Kd * lightIntensity * N·L]
    
    Start --> Specular[Especular Phong<br/> R = reflect -lightDir, normal<br/> R · V = max dot R, viewDir, 0<br/> I_s = Ks * lightIntensity * pow R·V, Ns]
    
    Ambient --> Sum[I_phong = I_a + I_d + I_s]
    Diffuse --> Sum
    Specular --> Sum
    
    Sum --> Att[Atenuação<br/> d = distance lightPos, fragPos<br/> att = 1 / k_c + k_l*d + k_q*d²]
    Att --> Mult[I_phong *= att]
    
    Mult --> CheckTex{hasTexture?}
    CheckTex -->|Sim| ApplyTex[I_final = I_phong * texture color]
    CheckTex -->|Não| NoTex[I_final = I_phong]
    
    ApplyTex --> CheckFog{fogEnabled?}
    NoTex --> CheckFog
    
    CheckFog -->|Sim| Fog[Calcula fator de fog<br/> fogFactor baseado em tipo<br/> I_final = mix I_final, fogColor, fogFactor]
    CheckFog -->|Não| Output
    
    Fog --> Output[FragColor = vec4 I_final, alpha]
    Output --> End[Retorna cor final]
```

**Tipos de Fog:**
- **Linear**: $f = \frac{end - d}{end - start}$
- **Exponencial**: $f = e^{-density \cdot d}$
- **Exponencial²**: $f = e^{-(density \cdot d)^2}$

---

## Arquitetura do Sistema

### Hierarquia de Classes

```
System
├── Camera
├── Shader
├── vector<unique_ptr<Object3D>>
│   └── Object3D
│       ├── unique_ptr<Mesh>
│       │   └── Mesh
│       │       ├── vector<Group>
│       │       │   └── Group
│       │       │       ├── vector<Face>
│       │       │       │   └── Face
│       │       │       └── Material
│       │       │           └── Texture (via ID)
│       │       ├── GLuint VAO, VBO, EBO
│       │       └── vector<vec3> vertices, normals
│       └── mat4 modelMatrix
└── vector<unique_ptr<Projetil>>
    └── Projetil : Object3D
        └── vec3 velocity
```

### Fluxo de Dados Completo

```mermaid
sequenceDiagram
    participant User
    participant GLFW
    participant System
    participant Camera
    participant Shader
    participant Object3D
    participant Mesh
    participant OpenGL
    participant GPU

    User->>GLFW: Input (teclado/mouse)
    GLFW->>System: Callbacks
    System->>Camera: ProcessInput
    Camera-->>System: Atualiza posição/direção
    
    System->>System: updateAnimations
    System->>System: updateProjeteis
    System->>System: checkCollisions
    
    System->>OpenGL: glClear
    System->>Shader: use()
    System->>Shader: setUniforms (view, proj, light, fog)
    
    loop Para cada objeto
        System->>Object3D: render(shader)
        Object3D->>Shader: setUniforms (model, material)
        Object3D->>Mesh: draw()
        Mesh->>OpenGL: glBindVertexArray(VAO)
        Mesh->>OpenGL: glDrawElements
        OpenGL->>GPU: Processa vértices
        GPU->>GPU: Vertex Shader
        GPU->>GPU: Rasterização
        GPU->>GPU: Fragment Shader
        GPU->>GPU: Testes (depth, blend)
        GPU-->>OpenGL: Frame buffer
    end
    
    System->>GLFW: glfwSwapBuffers
    GLFW->>GPU: Exibe na tela
    GPU-->>User: Imagem renderizada
```

---

## Resumo de Performance

### Otimizações Implementadas

| Técnica | Descrição | Benefício |
|---------|-----------|-----------|
| **Face Culling** | Remove faces traseiras | -50% fragmentos processados |
| **Depth Test** | Descarta fragmentos ocultos | Menos overdraw |
| **Batch por Material** | Agrupa objetos por textura | Reduz trocas de estado |
| **Cache de Texturas** | Reutiliza texturas carregadas | Economiza VRAM |
| **DeltaTime** | Movimentação independente de FPS | Consistência entre máquinas |
| **MSAA 4x** | Antialiasing por hardware | Suavização eficiente |

### Estatísticas Típicas

- **Objetos na cena**: 10-50
- **Triângulos por objeto**: 100-10,000
- **Texturas**: 5-20 (cache compartilhado)
- **FPS alvo**: 60 FPS (16.67ms/frame)
- **Resolução**: 1024x768

---

**Autores:** Ian Rossetti Boniatti e Eduardo Tropea  
**Curso:** Jogos Digitais - Unisinos  
**Disciplina:** Computação Gráfica em Tempo Real  
**Data:** Novembro 2025
