# Fluxo de Leitura de Arquivo OBJ

**System → Object3D → Mesh → OBJReader → Group → GPU** | Visualizador 3D - OpenGL 4.6

---

## Diagrama de Classes

```mermaid
classDiagram
    class System {
        +loadSceneObjects() bool
        +readObjectsInfos() vector~ObjectInfo~
        -sceneObjects vector~Object3D~
    }
    
    class Object3D {
        +loadObject(path) bool
        -mesh Mesh
        -name string
        -position vec3
    }
    
    class Mesh {
        +readObjectModel(path) bool
        -vertices vector~vec3~
        -texCoords vector~vec2~
        -normals vector~vec3~
        -groups vector~Group~
        -materials map~string,Material~
    }
    
    class OBJReader {
        +readFileOBJ(path, vertices, texCoords, normals, groups, materials) bool
        +readFileMTL(path, materials) bool
        -parseVertice(line, vertices) void
        -parseTexCoord(line, texCoords) void
        -parseNormal(line, normals) void
        -parseFace(line, face) void
    }
    
    class Group {
        +loadMaterialTexture(dir) void
        +setupBuffers(vertices, texCoords, normals) void
        -faces vector~Face~
        -material Material
        -VAO GLuint
        -VBO GLuint
    }
    
    class Texture {
        +loadFromFile(path) GLuint
        -textureID GLuint
    }
    
    System --> Object3D : cria
    Object3D --> Mesh : contém
    Mesh --> OBJReader : usa
    Mesh --> Group : contém N
    Group --> Texture : usa
    OBJReader ..> Mesh : preenche vetores
    OBJReader ..> Group : preenche faces
```

---

## Sequência de Chamadas

```mermaid
sequenceDiagram
    autonumber
    participant Sys as System
    participant Obj as Object3D
    participant Msh as Mesh
    participant OBJ as OBJReader
    participant Grp as Group
    
    Sys->>Sys: loadSceneObjects()
    loop Para cada objeto
        Sys->>Obj: loadObject(path)
        Obj->>Msh: readObjectModel(path)
        Msh->>OBJ: readFileOBJ(vertices, texCoords, normals, groups, materials)
        Note over OBJ: Parse .obj + .mtl
        OBJ-->>Msh: true
        Msh->>Grp: loadMaterialTexture()
        Msh->>Grp: setupBuffers()
        Note over Grp: GPU: VAO/VBO
        Msh-->>Obj: true
        Sys->>Sys: sceneObjects.push_back()
    end
```

---

## Métodos Principais

| Classe | Métodos Chave |
|--------|---------------|
| **System** | `loadSceneObjects()` `readObjectsInfos()` |
| **Object3D** | `loadObject(path)` |
| **Mesh** | `readObjectModel(path)` |
| **OBJReader** | `readFileOBJ(...)` `readFileMTL(...)` `parseVertice()` `parseTexCoord()` `parseNormal()` `parseFace()` |
| **Group** | `loadMaterialTexture(dir)` `setupBuffers(vertices, texCoords, normals)` |
| **Texture** | `loadFromFile(path)` |

### Estrutura de Dados

**CPU:** System → vector\<Object3D\> → Mesh (vertices, texCoords, normals) → vector\<Group\> (faces, material)  
**GPU:** Group (VAO, VBO) + Material (textureID)

---

## Fluxo Resumido

```mermaid
flowchart LR
    A[System::loadSceneObjects] --> B[readObjectsInfos]
    B --> C[Object3D::loadObject]
    C --> D[Mesh::readObjectModel]
    D --> E[OBJReader::readFileOBJ]
    E --> F[Parse .obj/.mtl]
    F --> G[Group::loadMaterialTexture]
    G --> H[Group::setupBuffers]
    H --> I[GPU: VAO/VBO]
```

---

**Autores:** Ian Rossetti Boniatti e Eduardo Tropea  
**Curso:** Jogos Digitais - Unisinos  
**Disciplina:** Computação Gráfica em Tempo Real  
**Data:** Novembro 2025
