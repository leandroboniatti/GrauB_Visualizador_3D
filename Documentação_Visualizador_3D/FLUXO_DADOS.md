# 🔄 Fluxo de Dados - Visualizador 3D

## Detalhamento Completo do Fluxo de Informações

> 📘 **Documentação Técnica:** Este documento detalha como os dados fluem através do sistema, desde o disco até a tela, incluindo todas as transformações e transferências entre CPU e GPU.

---

## 📋 Índice

1. [🌐 Visão Geral do Fluxo de Dados](#-visão-geral-do-fluxo-de-dados)
2. [⚙️ Fluxo de Dados: Inicialização](#️-fluxo-de-dados-inicialização)
3. [🔁 Fluxo de Dados: Loop de Renderização](#fluxo-de-dados-loop-de-renderização)
4. [🎨 Fluxo de Dados: Pipeline OpenGL](#fluxo-de-dados-pipeline-opengl)
5. [📦 Estruturas de Dados](#estruturas-de-dados)
6. [💾 Mapeamento de Memória](#mapeamento-de-memória)

---

## 🌐 Visão Geral do Fluxo de Dados

> **Objetivo:** Compreender como os dados transitam entre Disco → RAM → VRAM → Tela

### Arquitetura de Dados do Sistema

```mermaid
graph TB
    subgraph "DISCO"
        A[Configurador_Cena.txt]
        B[Arquivos .obj]
        C[Arquivos .mtl]
        D[Imagens Texturas .png/.jpg]
    end
    
    subgraph "CPU - Memória RAM"
        E[System]
        F[OBJReader]
        G[Texture::loadFromFile]
        H[Mesh]
        I[Cache de Texturas]
        J[Groups]
        K[Faces]
        L[Material]
    end
    
    subgraph "GPU - VRAM"
        M[VAO - Vertex Array Object]
        N[VBO - Vertex Buffer Object]
        P[Texture Objects]
        Q[Shader Program]
    end
    
    subgraph "Pipeline GPU"
        R[Vertex Shader]
        S[Rasterização]
        T[Fragment Shader]
        U[Frame Buffer]
        V[Tela]
    end
    
    A -->|Leitura IO| E
    B -->|Leitura IO| F
    C -->|Leitura IO| F
    D -->|Leitura IO| G
    
    E --> F
    F --> H
    G --> I
    
    H --> J
    J --> K
    J --> L
    L --> I
    
    H -->|glBufferData| M
    H -->|glBufferData| N
    I -->|glTexImage2D| P
    
    Q --> R
    M --> R
    N --> R
    R --> S
    S --> T
    P --> T
    T --> U
    U --> V
```

---

## ⚙️ Fluxo de Dados: Inicialização

> **Fase:** Carregamento inicial de todos os recursos antes do loop principal

### 📄 1. Leitura de Configuração da Cena

```mermaid
sequenceDiagram
    participant File as Configurador_Cena.txt
    participant System
    participant Camera
    participant Light
    participant Fog

    System->>File: Abre arquivo
    
    loop Para cada linha
        File->>System: Linha de texto
        
        alt CAMERA linha
            System->>Camera: position(x,y,z)
            Camera-->>System: Configurado
        else LIGHT linha
            System->>Light: position(x,y,z)
            System->>Light: intensity(R,G,B)
            Light-->>System: Configurado
        else ATTENUATION linha
            System->>Light: constant, linear, quadratic
            Light-->>System: Configurado
        else FOG linha
            System->>Fog: enabled, color, density, type
            Fog-->>System: Configurado
        else Objeto linha
            System->>System: Adiciona a objectsInfo[]
        end
    end
    
    System->>File: Fecha arquivo
```

**Formato dos Dados no Arquivo:**

```ini
# ========================================
# Configuração do Sistema
# ========================================
CAMERA 0.0 2.0 20.0                    # Posição inicial (x, y, z)
LIGHT 0.0 10.0 5.0 1.0 1.0 1.0         # Posição + Intensidade RGB
ATTENUATION 1.0 0.045 0.0075           # Constantes k_c, k_l, k_q
FOG 1 0.9 0.9 0.9 0.05 10.0 50.0 1    # Enabled + Color + Densidade + Tipo

# ========================================
# Objetos da Cena
# ========================================
# Nome  Modelo              Pos(x,y,z)      Rot(x,y,z)    Escala       Elim
Pista   models/pista.obj    0.0 -0.5 0.0    0.0 0.0 0.0   1.0 1.0 1.0  0
Carro   models/car.obj      0.0 0.0 0.0     0.0 90.0 0.0  1.0 1.0 1.0  1
```

**Estrutura ObjectInfo (CPU):**

```cpp
struct ObjectInfo {
    string name;        // "Carro"
    string modelPath;   // "models/car.obj"
    vec3 position;      // (0.0, 0.0, 0.0)
    vec3 rotation;      // (0.0, 90.0, 0.0)
    vec3 scale;         // (1.0, 1.0, 1.0)
    bool eliminable;    // true
};
```

---

### 2. Carregamento de Modelo OBJ → GPU

```mermaid
graph TB
    subgraph "DISCO"
        A[arquivo.obj]
        B[arquivo.mtl]
        C[textura.png]
    end
    
    subgraph "CPU - Parsing"
        D[OBJReader::readFileOBJ]
        E[vector&lt;vec3&gt; vertices]
        F[vector&lt;vec2&gt; texCoords]
        G[vector&lt;vec3&gt; normals]
        H[vector&lt;Group&gt; groups]
        I[map&lt;string,Material&gt; materials]
    end
    
    subgraph "CPU - Organização"
        J[Group::setupBuffers]
        K[Organiza dados por grupo]
        L[Calcula índices]
        M[vector&lt;float&gt; vertexData]
        N[vector&lt;unsigned int&gt; indices]
    end
    
    subgraph "GPU - VRAM"
        O[glGenVertexArrays → VAO]
        P[glGenBuffers → VBO]
        Q[glBufferData VBO]
        R[glVertexAttribPointer]
        S[glEnableVertexAttribArray]
    end
    
    A --> D
    B --> D
    D --> E
    D --> F
    D --> G
    D --> H
    D --> I
    
    E --> J
    F --> J
    G --> J
    H --> J
    
    J --> K
    K --> L
    L --> M
    L --> N
    
    M --> Q
    O --> R
    P --> Q
    R --> S
    
    C --> V[stbi_load]
    V --> W[glGenTextures]
    W --> X[glTexImage2D]
    X --> Y[Texture ID em VRAM]
```

---

### 3. Estrutura de Dados: Arquivo OBJ → Mesh

```mermaid
graph LR
    subgraph "Arquivo OBJ"
        A[v 1.0 0.0 0.0<br/>v 0.0 1.0 0.0<br/>v 0.0 0.0 1.0]
        B[vt 0.0 0.0<br/>vt 1.0 0.0<br/>vt 0.5 1.0]
        C[vn 0.0 1.0 0.0<br/>vn 0.0 1.0 0.0<br/>vn 0.0 1.0 0.0]
        D[mtllib arquivo.mtl]
        E[usemtl Material1]
        F[f 1/1/1 2/2/2 3/3/3]
    end
    
    subgraph "Estruturas CPU"
        G["vertices[0] = vec3 1,0,0<br/>vertices[1] = vec3 0,1,0<br/>vertices[2] = vec3 0,0,1"]
        H["texCoords[0] = vec2 0,0<br/>texCoords[1] = vec2 1,0<br/>texCoords[2] = vec2 0.5,1"]
        I["normals[0] = vec3 0,1,0<br/>normals[1] = vec3 0,1,0<br/>normals[2] = vec3 0,1,0"]
        J["materials[Material1]<br/>Ka, Kd, Ks, Ns, textureID"]
        K["groups[0].faces[0]<br/>vIdx=0,1,2<br/>vtIdx=0,1,2<br/>vnIdx=0,1,2"]
    end
    
    subgraph "Buffers GPU"
        L[VBO: pos0,norm0,uv0<br/>pos1,norm1,uv1<br/>pos2,norm2,uv2<br/>...vértices sequenciais]
        M[VAO: layout config]
    end
    
    A --> G
    B --> H
    C --> I
    D --> J
    F --> K
    
    G --> L
    H --> L
    I --> L
    L --> M
```

---

### 4. Layout de Vértice Completo (VBO)

```text
┌─────────────────────────────────────────────────────────┐
│                    VBO (GPU VRAM)                       │
│              FORMATO INTERLEAVED (Entrelaçado)          │
├─────────────────────────────────────────────────────────┤
│ Vértice 0:                                              │
│   layout(location=0): vec3 position    (12 bytes)      │
│   layout(location=1): vec2 texCoord    (8 bytes)       │
│   layout(location=2): vec3 normal      (12 bytes)      │
│   TOTAL: 32 bytes (8 floats)                           │
├─────────────────────────────────────────────────────────┤
│ Vértice 1:                                              │
│   layout(location=0): vec3 position    (12 bytes)      │
│   layout(location=1): vec2 texCoord    (8 bytes)       │
│   layout(location=2): vec3 normal      (12 bytes)      │
│   TOTAL: 32 bytes (8 floats)                           │
├─────────────────────────────────────────────────────────┤
│ ...                                                     │
│ Vértice N:                                              │
│   (mesmo layout, dados sequenciais)                    │
└─────────────────────────────────────────────────────────┘

⚠️ NOTA: Não usa EBO - vértices são armazenados sequencialmente
         Triângulos compartilham vértices duplicados no VBO
         Usa glDrawArrays ao invés de glDrawElements
```

**Código de Setup (Group::setupBuffers):**

```cpp
// Stride: 32 bytes (3 floats pos + 2 floats texCoord + 3 floats normal)
GLsizei stride = 8 * sizeof(float);

// Location 0: Posição (offset 0)
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
glEnableVertexAttribArray(0);

// Location 1: TexCoord (offset 12 bytes = 3 floats)
glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(1);

// Location 2: Normal (offset 20 bytes = 5 floats)
glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
glEnableVertexAttribArray(2);

// Renderização usa glDrawArrays, não glDrawElements
glDrawArrays(GL_TRIANGLES, 0, vertexCount);
```

---

## Fluxo de Dados: Loop de Renderização

### 1. Ciclo de Atualização e Renderização

```mermaid
graph TB
    Start[Início do Frame] --> DeltaTime[Calcula deltaTime<br/>CPU: 1 float]
    
    DeltaTime --> Input[Processa Input GLFW]
    Input --> KeyState{Teclas<br/>pressionadas?}
    KeyState -->|W/A/S/D| CameraUpdate[Atualiza Camera.Position<br/>CPU: vec3]
    KeyState -->|Mouse| CameraRot[Atualiza Camera.Front, Yaw, Pitch<br/>CPU: vec3, 2 floats]
    KeyState -->|Space| Shoot[Cria Projetil<br/>CPU: adiciona ao vector]
    
    CameraUpdate --> AnimUpdate[Atualiza Animações]
    CameraRot --> AnimUpdate
    Shoot --> AnimUpdate
    
    AnimUpdate --> ObjLoop{Para cada<br/>objeto}
    ObjLoop --> UpdateTransform[Atualiza transform matrix<br/>CPU: mat4 16 floats]
    UpdateTransform --> ObjLoop
    
    ObjLoop --> ProjUpdate[Atualiza Projéteis]
    ProjUpdate --> ProjLoop{Para cada<br/>projétil}
    ProjLoop --> ProjMove[position += velocity * deltaTime<br/>CPU: vec3]
    ProjMove --> ProjLoop
    
    ProjLoop --> Collision[Verifica Colisões]
    Collision --> CollLoop{Para cada par<br/>proj x obj}
    CollLoop --> DistCalc[Calcula distância<br/>CPU: float]
    DistCalc --> CollCheck{d < raio?}
    CollCheck -->|Sim| Remove[Remove obj e proj<br/>CPU: erase de vectors]
    CollCheck -->|Não| CollLoop
    Remove --> CollLoop
    
    CollLoop --> Render[System::render]
```

---

### 2. Fluxo Detalhado de Renderização

```mermaid
sequenceDiagram
    participant CPU as CPU (System)
    participant Shader as Shader Program
    participant GPU as GPU
    participant VRAM as VRAM

    CPU->>GPU: glClear(COLOR | DEPTH)
    GPU->>VRAM: Limpa frame buffer
    
    CPU->>Shader: glUseProgram(shaderID)
    GPU->>GPU: Ativa shader program
    
    Note over CPU: Envia Uniforms Globais
    CPU->>Shader: glUniformMatrix4fv(projection)
    CPU->>Shader: glUniformMatrix4fv(view)
    CPU->>Shader: glUniform3fv(lightPos)
    CPU->>Shader: glUniform3fv(lightIntensity)
    CPU->>Shader: glUniform3fv(viewPos)
    CPU->>Shader: glUniform1f(attConstant)
    CPU->>Shader: glUniform1f(attLinear)
    CPU->>Shader: glUniform1f(attQuadratic)
    CPU->>Shader: glUniform1i(fogEnabled)
    CPU->>Shader: glUniform3fv(fogColor)
    CPU->>Shader: glUniform1f(fogDensity)
    CPU->>Shader: glUniform1i(fogType)
    
    loop Para cada objeto da cena
        Note over CPU: Uniforms por Objeto
        CPU->>Shader: glUniformMatrix4fv(model)
        CPU->>Shader: glUniform3fv(Ka, Kd, Ks)
        CPU->>Shader: glUniform1f(Ns)
        CPU->>Shader: glUniform1i(hasDiffuseMap)
        CPU->>Shader: glUniform1i(isProjectile)
        
        alt Objeto tem textura
            CPU->>GPU: glBindTexture(textureID)
            VRAM->>GPU: Carrega textura
        end
        
        CPU->>GPU: glBindVertexArray(VAO)
        VRAM->>GPU: Configura atributos
        
        CPU->>GPU: glDrawArrays(GL_TRIANGLES, 0, count)
        GPU->>GPU: Vertex Shader (count vértices)
        GPU->>GPU: Rasterização
        GPU->>GPU: Fragment Shader (M fragmentos)
        GPU->>VRAM: Escreve no frame buffer
    end
    
    loop Para cada projétil
        CPU->>Shader: glUniform1i(isProjectile, true)
        CPU->>Shader: glUniform3f(objectColor)
        Note over CPU,GPU: Mesmo processo de renderização
    end
    
    CPU->>GPU: glfwSwapBuffers
    VRAM->>GPU: Front ↔ Back buffer swap
```

---

### 3. Dados Enviados como Uniforms (CPU → GPU)

```mermaid
graph TB
    subgraph "CPU - System"
        A[Camera<br/>Position: vec3<br/>Front: vec3<br/>Zoom: float]
        B[Light<br/>Position: vec3<br/>Intensity: vec3<br/>Attenuation: 3 floats]
        C[Fog<br/>Color: vec3<br/>Density: float<br/>Type: int<br/>Enabled: bool]
        D[Object3D<br/>Transform: mat4<br/>Material: Ka,Kd,Ks,Ns]
    end
    
    subgraph "GPU - Uniforms"
        E[Matrizes de Transformação<br/>projection: mat4 64bytes<br/>view: mat4 64bytes<br/>model: mat4 64bytes]
        F[Iluminação<br/>lightPos: vec3 12bytes<br/>lightIntensity: vec3 12bytes<br/>viewPos: vec3 12bytes<br/>attenuation: 3 floats 12bytes]
        G[Material<br/>Ka: vec3 12bytes<br/>Kd: vec3 12bytes<br/>Ks: vec3 12bytes<br/>Ns: float 4bytes]
        H[Fog<br/>fogColor: vec3 12bytes<br/>fogDensity: float 4bytes<br/>fogType: int 4bytes<br/>fogEnabled: bool 4bytes]
        I[Flags<br/>hasDiffuseMap: bool 4bytes<br/>isProjectile: bool 4bytes]
    end
    
    A --> E
    A --> F
    B --> F
    C --> H
    D --> E
    D --> G
    D --> I
```

**Tamanho Total de Uniforms por Frame:**

| Categoria | Dados | Tamanho |
|-----------|-------|---------|
| **Matrizes** | projection, view | 128 bytes (enviado 1x/frame) |
| **Iluminação Global** | light, attenuation, viewPos | 48 bytes (enviado 1x/frame) |
| **Fog** | color, density, type, enabled | 32 bytes (enviado 1x/frame) |
| **Por Objeto** | model, material | 112 bytes (enviado N vezes) |
| **Por Objeto** | flags, color | 16 bytes (enviado N vezes) |

Para uma cena com 20 objetos:

- Uniforms globais: ~200 bytes
- Uniforms por objeto: 128 × 20 = 2,560 bytes
- **Total: ~2.8 KB enviados CPU→GPU por frame**

---

## Fluxo de Dados: Pipeline OpenGL

### 1. Vertex Shader - Transformação de Vértices

```mermaid
graph TB
    subgraph "Entrada Vertex Shader"
        A[layout 0: vec3 aPos<br/>layout 1: vec3 aNormal<br/>layout 2: vec2 aTexCoord]
        B[uniform mat4 model<br/>uniform mat4 view<br/>uniform mat4 projection]
    end
    
    subgraph "Processamento GPU"
        C[worldPos = model * vec4 aPos, 1]
        D[worldNormal = mat3 model * aNormal]
        E[viewPos = view * worldPos]
        F[gl_Position = projection * viewPos]
    end
    
    subgraph "Saída para Rasterizador"
        G[out vec3 elementPosition<br/>out vec3 worldNormal<br/>out vec2 textureCoord]
        H[gl_Position → Clip Space]
    end
    
    A --> C
    B --> C
    C --> D
    D --> E
    E --> F
    F --> H
    C --> G
    D --> G
    A --> G
```

**Exemplo de Transformação:**

```text
Vértice Original (Object Space):
  aPos = (1.0, 0.0, 0.0)

Aplicando Transformações:
  1. Model:      (1,0,0) → (2,0,0)     [scale 2x]
  2. View:       (2,0,0) → (0,0,-18)   [câmera em z=20]
  3. Projection: (0,0,-18) → NDC       [perspectiva]
  
gl_Position = (-0.5, 0.0, 0.9, 1.0)  [Normalized Device Coords]
```

---

### 2. Rasterização - Vértices → Fragmentos

```mermaid
graph TB
    subgraph "Espaço de Tela"
        A[Vértice 0<br/>x=100, y=200<br/>normal=0,1,0<br/>uv=0,0]
        B[Vértice 1<br/>x=500, y=200<br/>normal=0,1,0<br/>uv=1,0]
        C[Vértice 2<br/>x=300, y=500<br/>normal=0,1,0<br/>uv=0.5,1]
    end
    
    D[Rasterizador<br/>Gera Fragmentos]
    
    subgraph "Fragmentos Interpolados"
        E[Fragmento x=250,y=300<br/>normal=0,1,0<br/>uv=0.375,0.33]
        F[Fragmento x=300,y=350<br/>normal=0,1,0<br/>uv=0.5,0.5]
        G[...]
        H[~10,000 fragmentos<br/>para triângulo 400x300px]
    end
    
    A --> D
    B --> D
    C --> D
    D --> E
    D --> F
    D --> G
    D --> H
```

**Interpolação Linear:**

```text
Para fragmento em (x, y):
  - Calcula coordenadas baricêntricas (u, v, w)
  - Interpola atributos:
    normal_frag = u*normal0 + v*normal1 + w*normal2
    uv_frag = u*uv0 + v*uv1 + w*uv2
```

---

### 3. Fragment Shader - Cálculo de Iluminação Phong

```mermaid
graph TB
    subgraph "Entrada Fragment Shader"
        A[in vec3 elementPosition<br/>in vec3 worldNormal<br/>in vec2 textureCoord]
        B[uniform Material Ka,Kd,Ks,Ns<br/>uniform Light pos,intensity<br/>uniform sampler2D diffuseMap]
    end
    
    subgraph "Cálculo Cor Base"
        C{hasDiffuseMap?}
        C -->|Sim| D[baseColor = texture<br/>diffuseMap, textureCoord.rgb]
        C -->|Não| E[baseColor = objectColor]
    end
    
    subgraph "Cálculo Iluminação"
        F[Ambiente<br/>ambient = Ka * lightIntensity * baseColor]
        
        G[Difusa<br/>lightDir = normalize lightPos - pos<br/>diff = max dot normal, lightDir, 0<br/>diffuse = Kd * diff * baseColor * light]
        
        H[Especular<br/>viewDir = normalize viewPos - pos<br/>reflectDir = reflect -lightDir, normal<br/>spec = pow max dot viewDir, reflectDir, 0, Ns<br/>specular = Ks * spec * lightIntensity]
        
        I[Atenuação<br/>distance = length lightPos - pos<br/>att = 1 / c1+c2*d+c3*d²<br/>diffuse *= att<br/>specular *= att]
    end
    
    subgraph "Fog"
        J{fogEnabled?}
        J -->|Sim| K[fogDistance = length viewPos - pos<br/>fogFactor = calc based on fogType<br/>finalColor = mix fogColor, phongColor, fogFactor]
        J -->|Não| L[finalColor = phongColor]
    end
    
    subgraph "Saída"
        M[FragColor = vec4 finalColor, 1.0]
    end
    
    A --> C
    B --> C
    D --> F
    E --> F
    F --> G
    G --> H
    H --> I
    I --> J
    K --> M
    L --> M
```

**Exemplo de Cálculo por Fragmento:**

```text
Entrada:
  elementPosition = (5.0, 2.0, -10.0)
  worldNormal = (0.0, 1.0, 0.0)
  textureCoord = (0.5, 0.5)
  
Textura:
  baseColor = texture(diffuseMap, uv) = (0.8, 0.2, 0.1)
  
Ambiente:
  ambient = (0.1,0.1,0.1) * (1,1,1) * (0.8,0.2,0.1) = (0.08,0.02,0.01)
  
Difusa:
  lightDir = normalize((0,10,5) - (5,2,-10)) = normalize(-5,8,15) = (-0.28,0.45,0.85)
  diff = max(dot((0,1,0), (-0.28,0.45,0.85)), 0) = 0.45
  diffuse = (0.8,0.8,0.8) * 0.45 * (1,1,1) * (0.8,0.2,0.1) = (0.29,0.07,0.04)
  
Especular:
  viewDir = normalize((0,2,20) - (5,2,-10)) = normalize(-5,0,30) = (-0.16,0,0.99)
  reflectDir = reflect((0.28,-0.45,-0.85), (0,1,0)) = (0.28,0.45,-0.85)
  spec = pow(max(dot((-0.16,0,0.99), (0.28,0.45,-0.85)), 0), 32) = pow(0, 32) = 0
  specular = (0, 0, 0)
  
Atenuação:
  distance = length((0,10,5) - (5,2,-10)) = 17.35
  att = 1 / (1 + 0.045*17.35 + 0.0075*17.35²) = 0.38
  diffuse *= 0.38 = (0.11, 0.03, 0.02)
  
Phong Total:
  phongColor = (0.08,0.02,0.01) + (0.11,0.03,0.02) + (0,0,0) = (0.19, 0.05, 0.03)
  
Fog (se enabled):
  fogDistance = 30.4
  fogFactor = exp(-0.05 * 30.4) = 0.22
  finalColor = mix((0.9,0.9,0.9), (0.19,0.05,0.03), 0.22) = (0.74, 0.72, 0.71)
  
Saída:
  FragColor = (0.74, 0.72, 0.71, 1.0)
```

---

### 4. Depth Test e Blending

```mermaid
graph TB
    A[Fragment Shader Output<br/>color + depth] --> B[Depth Test]
    
    B --> C{depth < depthBuffer?}
    C -->|Não| D[Descarta fragmento]
    C -->|Sim| E[Continua]
    
    E --> F{Alpha < 1.0?}
    F -->|Não| G[Escreve cor diretamente]
    F -->|Sim| H[Alpha Blending]
    
    H --> I[finalColor = srcColor*srcAlpha<br/>+ dstColor*1-srcAlpha]
    
    G --> J[Escreve no Frame Buffer]
    I --> J
    J --> K[Atualiza Depth Buffer]
```

**Exemplo de Blending:**

```text
Fragmento Atual (vidro transparente):
  srcColor = (0.2, 0.3, 0.9, 0.3)  [30% opaco]
  
Frame Buffer (já renderizado):
  dstColor = (0.8, 0.1, 0.1, 1.0)  [vermelho]
  
Blending:
  finalColor.rgb = (0.2,0.3,0.9)*0.3 + (0.8,0.1,0.1)*(1-0.3)
                 = (0.06,0.09,0.27) + (0.56,0.07,0.07)
                 = (0.62, 0.16, 0.34)  [azulado sobre vermelho]
```

---

## Estruturas de Dados

### 1. Hierarquia de Classes e Dados

```text
System
├── Camera
│   ├── vec3 Position         [12 bytes]
│   ├── vec3 Front            [12 bytes]
│   ├── vec3 Up               [12 bytes]
│   ├── vec3 Right            [12 bytes]
│   ├── float Yaw, Pitch      [8 bytes]
│   └── float Zoom            [4 bytes]
│   TOTAL: ~60 bytes
│
├── Shader
│   ├── GLuint ID             [4 bytes]
│   └── string vertexSource   [variável]
│   └── string fragmentSource [variável]
│
├── vector<unique_ptr<Object3D>>
│   └── Object3D
│       ├── string name               [~50 bytes]
│       ├── vec3 position             [12 bytes]
│       ├── vec3 rotation             [12 bytes]
│       ├── vec3 scale                [12 bytes]
│       ├── mat4 transform            [64 bytes]
│       ├── bool eliminable           [1 byte]
│       └── Mesh
│           ├── vector<vec3> vertices     [N * 12 bytes]
│           ├── vector<vec2> texCoords    [N * 8 bytes]
│           ├── vector<vec3> normals      [N * 12 bytes]
│           ├── vector<Group> groups
│           │   └── Group
│           │       ├── string name           [~30 bytes]
│           │       ├── vector<Face> faces    [M faces]
│           │       ├── Material material
│           │       │   ├── vec3 Ka, Kd, Ks   [36 bytes]
│           │       │   ├── float Ns, alpha   [8 bytes]
│           │       │   └── GLuint textureID  [4 bytes]
│           │       ├── GLuint VAO            [4 bytes]
│           │       ├── GLuint VBO            [4 bytes]
│           │       ├── int vertexCount       [4 bytes]
│           │       └── vector<float> vertices [N * 32 bytes]
│           └── BoundingBox
│               ├── vec3 pontoMinimo      [12 bytes]
│               └── vec3 pontoMaximo      [12 bytes]
│
└── vector<unique_ptr<Projetil>>
    └── Projetil : Object3D
        ├── vec3 velocity         [12 bytes]
        ├── float speed           [4 bytes]
        └── bool active           [1 byte]
```

---

### 2. Exemplo de Ocupação de Memória

**Objeto Simples (Cubo):**

```text
Cubo com 36 vértices (6 faces × 2 triângulos × 3 vértices):
  vertices:   36 × 12 bytes = 432 bytes
  texCoords:  36 × 8 bytes  = 288 bytes
  normals:    36 × 12 bytes = 432 bytes
  faces:      12 × ~40 bytes = 480 bytes  [6 faces × 2 triângulos]
  Material:   ~100 bytes
  Buffers:    8 bytes (VAO, VBO IDs)
  
  TOTAL CPU: ~1.8 KB
  
  GPU (VBO): 36 × 32 bytes = 1,152 bytes (dados interleaved)
  GPU (Textura 512×512 RGB): 786 KB
  
  TOTAL GPU: ~787 KB
  
⚠️ NOTA: Vértices duplicados (sem EBO) = maior uso de VRAM
         Vantagem: Melhor cache locality e simplicidade
```

**Cena Completa (20 objetos):**

```text
CPU:
  System overhead:        ~1 KB
  Camera:                 ~100 bytes
  20 objetos × 5 KB:      100 KB
  Cache de 10 texturas:   ~1 KB (apenas IDs)
  
  TOTAL CPU: ~100 KB

GPU (VRAM):
  20 objetos × 1 MB:      20 MB (geometria)
  10 texturas × 1 MB:     10 MB
  Frame buffers:          6 MB (1024×768×4 bytes × 2 buffers)
  Depth buffer:           3 MB
  Shader programs:        ~100 KB
  
  TOTAL GPU: ~39 MB
```

---

## Mapeamento de Memória

### 1. Fluxo Disco → RAM → VRAM

```mermaid
graph LR
    subgraph "DISCO"
        A1[arquivo.obj<br/>~500 KB]
        A2[textura.png<br/>~2 MB]
    end
    
    subgraph "RAM CPU"
        B1[vector vertices<br/>~50 KB]
        B2[stbi_image buffer<br/>~2 MB]
    end
    
    subgraph "VRAM GPU"
        C1[VBO + EBO<br/>~60 KB]
        C2[Texture Object<br/>~1 MB compressed]
    end
    
    A1 -->|Leitura IO| B1
    A2 -->|Leitura IO| B2
    B1 -->|glBufferData| C1
    B2 -->|glTexImage2D| C2
```

---

### 2. Timeline de Transferência de Dados

```mermaid
gantt
    title Carregamento de Objeto 3D
    dateFormat X
    axisFormat %L ms
    
    section Disco→CPU
    Abre arquivo .obj           :0, 5
    Parse vértices              :5, 30
    Parse faces                 :35, 20
    Carrega .mtl                :55, 10
    Carrega imagem PNG          :65, 50
    
    section CPU→GPU
    glGenBuffers                :115, 1
    glBufferData VBO            :116, 5
    glBufferData EBO            :121, 2
    glVertexAttribPointer       :123, 1
    glGenTextures               :124, 1
    glTexImage2D                :125, 15
    
    section Total
    Tempo total                 :0, 140
```

**Tempos Típicos:**

- Leitura de .obj (500 KB): ~30 ms
- Parse de geometria: ~20 ms
- Carregamento de textura PNG: ~50 ms
- Upload para GPU: ~20 ms
- **Total por objeto: ~120 ms**

Para 20 objetos (sem otimização): **~2.4 segundos**

---

### 3. Otimizações de Transferência

```mermaid
graph TB
    A[Otimizações Implementadas] --> B[Cache de Texturas]
    A --> C[Compartilhamento de Materiais]
    A --> D[Carregamento Assíncrono]
    
    B --> B1[Textura carregada 1x<br/>Reutilizada em N objetos<br/>Economia: ~10 MB VRAM]
    
    C --> C1[Material compartilhado<br/>Economia: ~2 KB RAM por objeto]
    
    D --> D1[Carrega enquanto renderiza<br/>frame anterior<br/>Melhora: tempo percebido]
```

---

## Resumo de Performance

### Dados Transferidos por Frame (60 FPS)

| Tipo de Dado | Quantidade | Tamanho | Frequência |
|--------------|------------|---------|------------|
| **Uniforms** | ~30 uniforms | ~3 KB | 60x/s = 180 KB/s |
| **Comandos Draw** | ~20 draws | ~1 KB | 60x/s = 60 KB/s |
| **Frame Buffer** | 1024×768×4 | 3 MB | 60x/s = 180 MB/s |
| **Texture Fetch** | Variável | - | Dentro da GPU (VRAM) |
| **Vertex Fetch** | Variável | - | Dentro da GPU (VRAM) |

**Bandwidth CPU→GPU:** ~250 KB/s  
**Bandwidth Interno GPU:** ~10 GB/s (texture/vertex fetches)  
**Bandwidth GPU→Monitor:** ~180 MB/s (frame buffer)

---

### Gargalos Comuns

```mermaid
graph TB
    A[Possíveis Gargalos] --> B[CPU Bound]
    A --> C[GPU Bound]
    A --> D[Memory Bound]
    
    B --> B1[Muitos draw calls<br/>Solução: Batching/Instancing]
    B --> B2[Lógica complexa CPU<br/>Solução: Otimizar código]
    
    C --> C1[Muitos fragmentos<br/>Solução: LOD, Culling]
    C --> C2[Shader complexo<br/>Solução: Simplificar cálculos]
    
    D --> D1[Texturas muito grandes<br/>Solução: Mipmaps, Compressão]
    D --> D2[Muitas transferências<br/>Solução: Buffer persistente]
```

---

**Autores:** Ian Rossetti Boniatti e Eduardo Tropea  
**Curso:** Jogos Digitais - Unisinos  
**Disciplina:** Computação Gráfica em Tempo Real  
**Data:** Novembro 2025
