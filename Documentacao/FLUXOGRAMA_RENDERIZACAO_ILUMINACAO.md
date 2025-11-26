# Fluxograma Completo da Renderização com Iluminação

```mermaid
flowchart TD
    A[Início: Carregamento da Cena] --> B[system.loadSceneObjects()]
    B --> C[readFileConfiguration()]
    C --> D[sceneObjectsInfo: lista de objetos]
    D --> E[Para cada ObjectInfo]
    E --> F[Cria Object3D]
    F --> G[Object3D.loadObject(modelPath)]
    G --> H[Mesh.readObjectModel(path)]
    H --> I[OBJReader.readFileOBJ(path, ...)]
    I --> J[Mesh.setupMesh()]
    J --> K[Setup buffers OpenGL (VAO/VBO)]
    K --> L[Pronto para renderização]

    L --> M[Loop de Renderização]
    M --> N[Para cada Object3D em cena]
    N --> O[Object3D.render(shader)]
    O --> P[Envia model matrix para shader]
    P --> Q[Mesh.render(shader)]
    Q --> R[Para cada Group]
    R --> S[Envia propriedades do material para shader]
    S --> T[Se tem textura: glBindTexture]
    T --> U[Group.render()]
    U --> V[glBindVertexArray(VAO)]
    V --> W[glDrawArrays(GL_TRIANGLES, ...)]
    W --> X[GPU processa: Vertex Shader]
    X --> Y[GPU processa: Rasterização]
    Y --> Z[GPU processa: Fragment Shader]
    Z --> AA[Iluminação Phong: calcula cor final]
    AA --> AB[Fim do frame]
```

**Resumo:**
- O shader recebe propriedades do material e da luz.
- O Fragment Shader calcula a iluminação Phong (ambiente, difusa, especular).
- O resultado é a cor final do objeto, considerando luz, textura e material.
