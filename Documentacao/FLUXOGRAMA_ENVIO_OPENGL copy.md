# Fluxograma: Envio de Informações para OpenGL

## Diagrama Principal

```mermaid
graph TD
    subgraph INICIALIZAÇÃO
        A1[Carrega arquivo .OBJ] --> A2[Parse de vértices, normais, texCoords]
        A2 --> A3[Cria estruturas Face e Group]
        A3 --> A4[Mesh.setupMesh]
    end

    subgraph SETUP_BUFFERS["Setup de Buffers OpenGL"]
        B1[glGenVertexArrays - Cria VAO]
        B2[glGenBuffers - Cria VBO]
        B3[glBindVertexArray - Vincula VAO]
        B4[glBindBuffer - Vincula VBO]
        B5[glBufferData - Envia vértices para GPU]
        B6[glVertexAttribPointer - Layout 0: Position]
        B7[glVertexAttribPointer - Layout 1: Normal]
        B8[glVertexAttribPointer - Layout 2: TexCoords]
        B9[glEnableVertexAttribArray - Ativa atributos]
        
        B1 --> B2 --> B3 --> B4 --> B5 --> B6 --> B7 --> B8 --> B9
    end

    subgraph TEXTURAS["Carregamento de Texturas"]
        C1[stbi_load - Carrega imagem do disco]
        C2[glGenTextures - Cria ID de textura]
        C3[glBindTexture - Vincula textura]
        C4[glTexImage2D - Envia pixels para GPU]
        C5[glGenerateMipmap - Gera mipmaps]
        C6[glTexParameteri - Configura filtros]
        
        C1 --> C2 --> C3 --> C4 --> C5 --> C6
    end

    subgraph SHADER["Compilação de Shaders"]
        D1[Lê código do vertex shader]
        D2[Lê código do fragment shader]
        D3[glCreateShader - Cria shader objects]
        D4[glShaderSource - Envia código fonte]
        D5[glCompileShader - Compila shaders]
        D6[glCreateProgram - Cria programa]
        D7[glAttachShader - Anexa shaders]
        D8[glLinkProgram - Linka programa]
        
        D1 --> D3
        D2 --> D3
        D3 --> D4 --> D5 --> D6 --> D7 --> D8
    end

    subgraph RENDER_LOOP["Loop de Renderização"]
        E1[glClear - Limpa buffers]
        E2[glUseProgram - Ativa shader]
        E3[Envia uniforms: view, projection]
        E4[Para cada objeto na cena]
        E5[Envia uniform: model matrix]
        E6[Envia uniforms: material Ka, Kd, Ks, Ns]
        E7[Envia uniforms: luz posição, cor, intensidade]
        E8[glActiveTexture + glBindTexture]
        E9[glBindVertexArray - Vincula VAO]
        E10[glDrawArrays - Desenha triângulos]
        E11[glfwSwapBuffers - Troca buffers]
        
        E1 --> E2 --> E3 --> E4 --> E5 --> E6 --> E7 --> E8 --> E9 --> E10 --> E11
        E11 --> E1
    end

    A4 --> B1
    A4 --> C1
    SHADER --> E2
    SETUP_BUFFERS --> E9
    TEXTURAS --> E8
```

---

## Detalhamento: Dados Enviados para GPU

### 1. Vértices (VBO)

```mermaid
graph LR
    subgraph CPU["Memória CPU"]
        V1["vector<Vertex> vertices"]
        V2["Vertex: Position (vec3)"]
        V3["Vertex: Normal (vec3)"]
        V4["Vertex: TexCoords (vec2)"]
    end

    subgraph GPU["Memória GPU"]
        G1["VBO (Vertex Buffer Object)"]
        G2["Dados entrelaçados"]
    end

    V1 --> |glBufferData| G1
    V2 --> G2
    V3 --> G2
    V4 --> G2
```

### 2. Layout de Atributos

| Location | Atributo   | Tipo  | Tamanho | Offset                        |
|----------|------------|-------|---------|-------------------------------|
| 0        | Position   | vec3  | 12 bytes| 0                             |
| 1        | Normal     | vec3  | 12 bytes| offsetof(Vertex, Normal)      |
| 2        | TexCoords  | vec2  | 8 bytes | offsetof(Vertex, TexCoords)   |

**Stride total:** `sizeof(Vertex)` = 32 bytes

### 3. Uniforms Enviados por Frame

```mermaid
graph TD
    subgraph MATRIZES["Matrizes de Transformação"]
        M1["mat4 model - posição/rotação/escala do objeto"]
        M2["mat4 view - posição/orientação da câmera"]
        M3["mat4 projection - perspectiva"]
    end

    subgraph MATERIAL["Propriedades do Material"]
        MA1["vec3 Ka - coeficiente ambiente"]
        MA2["vec3 Kd - coeficiente difuso"]
        MA3["vec3 Ks - coeficiente especular"]
        MA4["float Ns - expoente especular"]
    end

    subgraph LUZ["Propriedades da Luz"]
        L1["vec3 lightPos - posição da luz"]
        L2["vec3 lightColor - cor da luz"]
        L3["float lightIntensity - intensidade"]
        L4["vec3 viewPos - posição da câmera"]
    end

    subgraph FOG["Parâmetros de Névoa"]
        F1["float fogDensity"]
        F2["float fogGradient"]
        F3["vec3 fogColor"]
    end

    subgraph ATENUACAO["Atenuação da Luz"]
        AT1["float constant"]
        AT2["float linear"]
        AT3["float quadratic"]
    end

    MATRIZES --> |glUniformMatrix4fv| SHADER
    MATERIAL --> |glUniform3fv / glUniform1f| SHADER
    LUZ --> |glUniform3fv / glUniform1f| SHADER
    FOG --> |glUniform1f / glUniform3fv| SHADER
    ATENUACAO --> |glUniform1f| SHADER
```

---

## Resumo das Funções OpenGL Utilizadas

| Função                    | Propósito                                      |
|---------------------------|------------------------------------------------|
| `glGenVertexArrays`       | Cria Vertex Array Object                       |
| `glGenBuffers`            | Cria Vertex Buffer Object                      |
| `glBindVertexArray`       | Ativa VAO para uso                             |
| `glBindBuffer`            | Ativa VBO para uso                             |
| `glBufferData`            | Envia dados de vértices para GPU               |
| `glVertexAttribPointer`   | Define layout dos atributos                    |
| `glEnableVertexAttribArray`| Ativa atributo de vértice                     |
| `glGenTextures`           | Cria objeto de textura                         |
| `glBindTexture`           | Ativa textura para uso                         |
| `glTexImage2D`            | Envia pixels da textura para GPU               |
| `glTexParameteri`         | Configura filtros de textura                   |
| `glGenerateMipmap`        | Gera níveis de mipmap                          |
| `glCreateShader`          | Cria objeto de shader                          |
| `glShaderSource`          | Envia código fonte do shader                   |
| `glCompileShader`         | Compila shader                                 |
| `glCreateProgram`         | Cria programa de shader                        |
| `glAttachShader`          | Anexa shader ao programa                       |
| `glLinkProgram`           | Linka programa de shader                       |
| `glUseProgram`            | Ativa programa de shader                       |
| `glUniformMatrix4fv`      | Envia matriz 4x4 para shader                   |
| `glUniform3fv`            | Envia vetor 3D para shader                     |
| `glUniform1f`             | Envia float para shader                        |
| `glUniform1i`             | Envia inteiro para shader                      |
| `glDrawArrays`            | Renderiza primitivas                           |
| `glClear`                 | Limpa buffers de cor/profundidade              |

---

## Pipeline Completo

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              FLUXO DE DADOS                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Arquivo .OBJ ──► Parse ──► CPU (vector<Vertex>) ──► GPU (VBO)              │
│                                                                             │
│  Arquivo .MTL ──► Parse ──► Material (Ka, Kd, Ks, Ns) ──► Uniforms          │
│                                                                             │
│  Arquivo .PNG/JPG ──► stbi_load ──► CPU (pixels) ──► GPU (Textura)          │
│                                                                             │
│  Arquivo .glsl ──► Leitura ──► glShaderSource ──► glCompileShader           │
│                                                                             │
│  Camera/Transform ──► Matrizes (model, view, proj) ──► Uniforms             │
│                                                                             │
│  Configurador_Cena.txt ──► Parâmetros (luz, fog, atenuação) ──► Uniforms    │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```
