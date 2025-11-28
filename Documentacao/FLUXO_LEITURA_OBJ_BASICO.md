# 📖 Fluxo Básico de Leitura de Arquivo OBJ

> **Documentação:** Este documento apresenta o fluxo simplificado de leitura de arquivos OBJ desde a inicialização do System até o envio dos dados para a GPU.

---

## 🔄 Fluxo Geral

```mermaid
graph TB
    Start[main.cpp] --> System[System::loadSceneObjects]
    System --> ReadConfig[readObjectsInfos]
    ReadConfig --> ConfigFile[Lê Configurador_Cena.txt]
    
    ConfigFile --> Loop{Para cada<br/>objeto}
    
    Loop --> CreateObj[Cria Object3D]
    CreateObj --> LoadObj[Object3D::loadObject]
    LoadObj --> MeshRead[Mesh::readObjectModel]
    MeshRead --> OBJRead[OBJReader::readFileOBJ]
    
    OBJRead --> ParseOBJ[Processa arquivo .obj]
    ParseOBJ --> ParseMTL[Lê arquivo .mtl]
    
    ParseOBJ --> FillVectors[Preenche vetores:<br/>vertices, texCoords,<br/>normals, groups]
    ParseMTL --> FillMaterials[Preenche mapa:<br/>materials]
    
    FillVectors --> LoadTextures[Group::loadMaterialTexture]
    FillMaterials --> LoadTextures
    
    LoadTextures --> SetupBuffers[Group::setupBuffers]
    SetupBuffers --> GPU[Envia para GPU:<br/>VAO, VBO]
    
    GPU --> AddToScene[Adiciona objeto<br/>ao sceneObjects]
    AddToScene --> Loop
    
    Loop --> End[Retorna ao main]
```

---

## 📋 Sequência de Chamadas

```mermaid
sequenceDiagram
    participant Main as main.cpp
    participant Sys as System
    participant Obj as Object3D
    participant Mesh as Mesh
    participant OBJ as OBJReader
    participant Grp as Group
    participant GPU as OpenGL GPU

    Main->>Sys: loadSceneObjects()
    Sys->>Sys: readObjectsInfos()
    Note over Sys: Lê Configurador_Cena.txt
    
    loop Para cada objeto
        Sys->>Obj: new Object3D(name)
        Sys->>Obj: loadObject(modelPath)
        Obj->>Mesh: readObjectModel(path)
        Mesh->>OBJ: readFileOBJ(...)
        
        Note over OBJ: Abre arquivo .obj
        OBJ->>OBJ: Lê linha por linha
        
        alt Linha "mtllib"
            OBJ->>OBJ: readFileMTL(...)
            Note over OBJ: Carrega materiais
        end
        
        alt Linha "v"
            OBJ->>OBJ: parseVertice()
            OBJ->>Mesh: vertices.push_back()
        end
        
        alt Linha "vt"
            OBJ->>OBJ: parseTexCoord()
            OBJ->>Mesh: texCoords.push_back()
        end
        
        alt Linha "vn"
            OBJ->>OBJ: parseNormal()
            OBJ->>Mesh: normals.push_back()
        end
        
        alt Linha "g" ou "o"
            OBJ->>Mesh: groups.emplace_back()
        end
        
        alt Linha "usemtl"
            OBJ->>Grp: group.material = materials[name]
        end
        
        alt Linha "f"
            OBJ->>OBJ: parseFace()
            OBJ->>Grp: group.addFace(face)
        end
        
        OBJ-->>Mesh: Retorna true
        
        loop Para cada grupo
            Mesh->>Grp: loadMaterialTexture()
            Note over Grp: Carrega texturas PNG/JPG
            Mesh->>Grp: setupBuffers()
            Grp->>GPU: glGenVertexArrays(VAO)
            Grp->>GPU: glGenBuffers(VBO)
            Grp->>GPU: glBufferData(dados)
        end
        
        Mesh-->>Obj: Retorna true
        Obj-->>Sys: Retorna true
        
        Sys->>Obj: setPosition/Rotation/Scale()
        Sys->>Sys: sceneObjects.push_back()
    end
    
    Sys-->>Main: Retorna true
```

---

## 📂 Estrutura de Dados

### 1. Configurador_Cena.txt

```ini
# Nome  Modelo              Pos(x,y,z)      Rot(x,y,z)    Escala       Elim
Pista   models/pista.obj    0.0 -0.5 0.0    0.0 0.0 0.0   1.0 1.0 1.0  0
Carro   models/car.obj      0.0 0.0 0.0     0.0 90.0 0.0  1.0 1.0 1.0  1
```

↓

### 2. System::readObjectsInfos()

```cpp
vector<ObjectInfo> {
    ObjectInfo {
        name: "Pista",
        modelPath: "models/pista.obj",
        position: vec3(0.0, -0.5, 0.0),
        rotation: vec3(0.0, 0.0, 0.0),
        scale: vec3(1.0, 1.0, 1.0),
        eliminable: false
    },
    ObjectInfo {
        name: "Carro",
        modelPath: "models/car.obj",
        ...
    }
}
```

↓

### 3. Object3D::loadObject("models/pista.obj")

```cpp
Object3D {
    name: "Pista",
    mesh: Mesh {
        vertices: vector<vec3>,
        texCoords: vector<vec2>,
        normals: vector<vec3>,
        groups: vector<Group>,
        materials: map<string, Material>
    },
    position: vec3(0.0, -0.5, 0.0),
    rotation: vec3(0.0, 0.0, 0.0),
    scale: vec3(1.0, 1.0, 1.0)
}
```

---

## 🔍 Detalhamento: OBJReader::readFileOBJ

### Processamento Linha por Linha

```mermaid
graph TB
    Start[Abre arquivo .obj] --> ReadLine[Lê linha]
    
    ReadLine --> CheckEmpty{Linha vazia<br/>ou comentário?}
    CheckEmpty -->|Sim| ReadLine
    CheckEmpty -->|Não| GetPrefix[Extrai prefixo]
    
    GetPrefix --> CheckPrefix{Qual prefixo?}
    
    CheckPrefix -->|mtllib| MTL[readFileMTL<br/>Carrega materiais]
    CheckPrefix -->|v| V[parseVertice<br/>vertices.push_back]
    CheckPrefix -->|vt| VT[parseTexCoord<br/>texCoords.push_back]
    CheckPrefix -->|vn| VN[parseNormal<br/>normals.push_back]
    CheckPrefix -->|g ou o| G[Cria novo Group<br/>groups.emplace_back]
    CheckPrefix -->|usemtl| U[Atribui material<br/>ao grupo atual]
    CheckPrefix -->|f| F[parseFace<br/>group.addFace]
    
    MTL --> ReadLine
    V --> ReadLine
    VT --> ReadLine
    VN --> ReadLine
    G --> ReadLine
    U --> ReadLine
    F --> ReadLine
    
    ReadLine --> EOF{Fim do<br/>arquivo?}
    EOF -->|Não| ReadLine
    EOF -->|Sim| Return[Retorna true]
```

### Exemplo de Processamento

**Arquivo pista.obj:**
```obj
# Comentário
mtllib pista.mtl

v -10.0 0.0 -10.0
v  10.0 0.0 -10.0
v  10.0 0.0  10.0
v -10.0 0.0  10.0

vt 0.0 0.0
vt 1.0 0.0
vt 1.0 1.0
vt 0.0 1.0

vn 0.0 1.0 0.0
vn 0.0 1.0 0.0
vn 0.0 1.0 0.0
vn 0.0 1.0 0.0

g Pista
usemtl PistaMaterial
f 1/1/1 2/2/2 3/3/3
f 1/1/1 3/3/3 4/4/4
```

**Resultado em memória:**

```cpp
// Mesh::vertices
vertices[0] = vec3(-10.0, 0.0, -10.0)
vertices[1] = vec3( 10.0, 0.0, -10.0)
vertices[2] = vec3( 10.0, 0.0,  10.0)
vertices[3] = vec3(-10.0, 0.0,  10.0)

// Mesh::texCoords
texCoords[0] = vec2(0.0, 0.0)
texCoords[1] = vec2(1.0, 0.0)
texCoords[2] = vec2(1.0, 1.0)
texCoords[3] = vec2(0.0, 1.0)

// Mesh::normals
normals[0] = vec3(0.0, 1.0, 0.0)
normals[1] = vec3(0.0, 1.0, 0.0)
normals[2] = vec3(0.0, 1.0, 0.0)
normals[3] = vec3(0.0, 1.0, 0.0)

// Mesh::groups
groups[0] = Group {
    name: "Pista",
    material: materials["PistaMaterial"],
    faces: [
        Face {
            vertexIndices:   [0, 1, 2],
            texCoordIndices: [0, 1, 2],
            normalIndices:   [0, 1, 2]
        },
        Face {
            vertexIndices:   [0, 2, 3],
            texCoordIndices: [0, 2, 3],
            normalIndices:   [0, 2, 3]
        }
    ]
}
```

---

## 🎨 Carregamento de Materiais e Texturas

### 1. Leitura do MTL

```mermaid
graph LR
    A[arquivo.obj] -->|mtllib pista.mtl| B[readFileMTL]
    B --> C[Processa pista.mtl]
    C --> D[Para cada newmtl]
    D --> E[Lê Ka, Kd, Ks, Ns]
    D --> F[Lê map_Kd texture.png]
    E --> G[materials map]
    F --> G
```

**Exemplo pista.mtl:**
```mtl
newmtl PistaMaterial
Ka 0.2 0.2 0.2
Kd 0.8 0.8 0.8
Ks 0.5 0.5 0.5
Ns 32.0
map_Kd textures/pista_diffuse.png
```

### 2. Carregamento de Textura

```mermaid
sequenceDiagram
    participant Mesh
    participant Group
    participant Texture
    participant STB
    participant GPU

    Mesh->>Group: loadMaterialTexture(modelDir)
    
    alt Material tem map_Kd
        Group->>Texture: loadFromFile(texturePath)
        Texture->>STB: stbi_load(path)
        STB-->>Texture: Dados RGB/RGBA
        Texture->>GPU: glGenTextures()
        Texture->>GPU: glBindTexture()
        Texture->>GPU: glTexImage2D(data)
        Texture->>GPU: glGenerateMipmap()
        GPU-->>Texture: textureID
        Texture-->>Group: textureID
        Group->>Group: material.textureID = id
    end
```

---

## 🎯 Configuração dos Buffers OpenGL

### Group::setupBuffers

```mermaid
graph TB
    Start[setupBuffers] --> Loop{Para cada face}
    
    Loop --> GetIndices[Obtém índices:<br/>vIdx, vtIdx, vnIdx]
    GetIndices --> GetData[Busca dados nos vetores:<br/>vertices vIdx<br/>texCoords vtIdx<br/>normals vnIdx]
    
    GetData --> Interleave[Intercala no buffer:<br/>pos.x pos.y pos.z<br/>tex.u tex.v<br/>norm.x norm.y norm.z]
    
    Interleave --> Loop
    
    Loop --> GenVAO[glGenVertexArrays]
    GenVAO --> BindVAO[glBindVertexArray]
    BindVAO --> GenVBO[glGenBuffers]
    GenVBO --> BindVBO[glBindBuffer GL_ARRAY_BUFFER]
    BindVBO --> BufferData[glBufferData vertices]
    
    BufferData --> Attr0[glVertexAttribPointer 0<br/>position: 3 floats offset 0]
    Attr0 --> Enable0[glEnableVertexAttribArray 0]
    
    Enable0 --> Attr1[glVertexAttribPointer 1<br/>texCoord: 2 floats offset 12]
    Attr1 --> Enable1[glEnableVertexAttribArray 1]
    
    Enable1 --> Attr2[glVertexAttribPointer 2<br/>normal: 3 floats offset 20]
    Attr2 --> Enable2[glEnableVertexAttribArray 2]
    
    Enable2 --> Unbind[Unbind VAO e VBO]
    Unbind --> End[Fim]
```

### Layout do VBO (32 bytes por vértice)

```text
Vértice 0:
  [0-11]   position:  vec3 (12 bytes)
  [12-19]  texCoord:  vec2 (8 bytes)
  [20-31]  normal:    vec3 (12 bytes)

Vértice 1:
  [32-43]  position:  vec3 (12 bytes)
  [44-51]  texCoord:  vec2 (8 bytes)
  [52-63]  normal:    vec3 (12 bytes)

...
```

---

## 📊 Resumo do Fluxo

| Etapa | Classe | Método | Dados Processados |
|-------|--------|--------|-------------------|
| 1 | System | loadSceneObjects() | Inicia processo de carga |
| 2 | System | readObjectsInfos() | Lê Configurador_Cena.txt → ObjectInfo[] |
| 3 | Object3D | loadObject() | Recebe path do modelo |
| 4 | Mesh | readObjectModel() | Coordena leitura do OBJ |
| 5 | OBJReader | readFileOBJ() | Processa .obj linha por linha |
| 6 | OBJReader | readFileMTL() | Carrega materiais do .mtl |
| 7 | OBJReader | parseVertice/Normal/etc | Preenche vetores vertices/normals/etc |
| 8 | Group | loadMaterialTexture() | Carrega texturas PNG/JPG |
| 9 | Group | setupBuffers() | Cria VAO/VBO e envia para GPU |
| 10 | System | sceneObjects.push_back() | Adiciona à cena |

---

## ⏱️ Estimativa de Tempo

Para um modelo médio (1000 vértices, 1 textura):

```text
1. Leitura arquivo .obj      ~10ms
2. Parse geometria            ~5ms
3. Leitura arquivo .mtl       ~2ms
4. Carregamento textura       ~30ms
5. Setup buffers GPU          ~5ms
─────────────────────────────────
   TOTAL por objeto:          ~52ms
```

Para 20 objetos: **~1 segundo**

---

**Autores:** Ian Rossetti Boniatti e Eduardo Tropea  
**Curso:** Jogos Digitais - Unisinos  
**Disciplina:** Computação Gráfica em Tempo Real  
**Data:** Novembro 2025
