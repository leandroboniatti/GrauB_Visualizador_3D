# 🪟 IMPLEMENTAÇÃO DE TRANSPARÊNCIA DOS VIDROS

**Data:** 21 de Novembro de 2025  
**Projeto:** Visualizador 3D - Grau B  
**Alunos:** Ian Rossetti Boniatti e Eduardo Tropea

---

## 📋 SUMÁRIO

1. [Visão Geral](#visão-geral)
2. [Modificações por Arquivo](#modificações-por-arquivo)
3. [Como Funciona](#como-funciona)
4. [Testes](#testes)

---

## 🎯 VISÃO GERAL

Sistema completo de transparência implementado para renderizar vidros do carro com efeito realista, permitindo visualizar o interior através das janelas.

### Características Implementadas

✅ **Leitura de propriedade `d` (dissolve)** do formato MTL  
✅ **Alpha blending** configurado no OpenGL  
✅ **Depth mask** controlado para objetos transparentes  
✅ **Uniform `d`** enviado para shaders  
✅ **Canal alpha** aplicado no FragColor  

### Valores de Transparência

- `d = 1.0` → **100% opaco** (sem transparência)
- `d = 0.4` → **40% opaco, 60% transparente** (vidros do carro)
- `d = 0.0` → **100% transparente** (invisível)

---

## 📝 MODIFICAÇÕES POR ARQUIVO

### 1️⃣ **include/Material.h** - Estrutura do Material

**Modificação:** Adicionado atributo `float d` para transparência

```cpp
class Material {
public:
    string name;      // Nome do material
    vec3 Ka;          // Coeficiente ambiente
    vec3 Kd;          // Coeficiente difuso
    vec3 Ks;          // Coeficiente especular
    float Ns;         // Expoente especular (shininess)
    float d;          // ← NOVO: Dissolve (transparência): 1.0 = opaco, 0.0 = transparente
    string map_Kd;    // Textura difusa
    
    // Construtor padrão
    Material() 
        : name("default"),
          Ka(0.2f, 0.2f, 0.2f),
          Kd(0.8f, 0.8f, 0.8f),
          Ks(1.0f, 1.0f, 1.0f),
          Ns(32.0f),
          d(1.0f),      // ← NOVO: Opaco por padrão
          map_Kd("")
    {}
    
    // Construtor com parâmetros
    Material(const string& materialName, 
             const vec3& ambient,
             const vec3& diffuse,
             const vec3& specular,
             float shininess,
             float dissolve = 1.0f,  // ← NOVO: Parâmetro de transparência
             const string& texture = "")
        : name(materialName),
          Ka(ambient),
          Kd(diffuse),
          Ks(specular),
          Ns(shininess),
          d(dissolve),  // ← NOVO: Inicializa transparência
          map_Kd(texture)
    {}
    
    bool hasTexture() const { return !map_Kd.empty(); }
};
```

**Linhas modificadas:** 18, 27, 37, 43

---

### 2️⃣ **src/OBJReader.cpp** - Leitura de Arquivos MTL

**Modificação:** Adicionado parsing da propriedade `d`

```cpp
// Dentro da função readFileMTL(), após ler a propriedade Ns
// Localização aproximada: linha 155-165

else if (prefix == "Ns") {  // Expoente especular (brilho/shininess)
    float ns;
    sline >> ns;
    currentMaterial.Ns = ns;
}
else if (prefix == "d") {  // ← NOVO: Dissolve (transparência)
    float dissolve;
    sline >> dissolve;
    currentMaterial.d = dissolve;
}
else if (prefix == "map_Kd") {  // Textura
    sline >> currentMaterial.map_Kd;
}
```

**Linhas adicionadas:** 160-164 (aproximadamente)

**Formato MTL suportado:**
```
newmtl Superb3c_gla
Ka 0.4 0.4 0.4
Kd 1.0 1.0 1.0
Ks 0.8 0.8 0.8
d 0.400000          ← Lido pelo código acima
Ns 64.0
map_Kd ../textures/Tex_Glass.jpg
```

---

### 3️⃣ **src/System.cpp** - Configuração OpenGL e Shaders

#### 3.1 Habilitação de Blending (função `initialize()`)

**Modificação:** Habilitado alpha blending do OpenGL

```cpp
// Localização aproximada: linha 98-105

glEnable(GL_DEPTH_TEST);        // Ativa o teste de profundidade
glClear(GL_DEPTH_BUFFER_BIT);

glEnable(GL_MULTISAMPLE);       // Ativa antialiasing

// ← NOVO: Habilita blending para transparência (vidros)
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
```

**Linhas adicionadas:** 104-105 (aproximadamente)

**Explicação:**
- `glEnable(GL_BLEND)`: Ativa mistura de cores
- `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`: 
  - `SRC_ALPHA` = alpha do fragmento atual
  - `ONE_MINUS_SRC_ALPHA` = 1.0 - alpha do fragmento atual
  - Fórmula: `color_final = color_src * alpha + color_dst * (1 - alpha)`

---

#### 3.2 Fragment Shader - Uniform `d`

**Modificação:** Adicionado uniform para transparência

```glsl
// Localização aproximada: linha 180-186

// Propriedades do material
uniform vec3 Ka;   // Coeficiente ambiente
uniform vec3 Kd;   // Coeficiente difuso
uniform vec3 Ks;   // Coeficiente especular
uniform float Ns;  // Expoente especular (shininess)
uniform float d;   // ← NOVO: Dissolve (transparência): 1.0 = opaco, 0.0 = transparente

// Propriedades da luz
uniform vec3 lightPos;
```

**Linha adicionada:** 185 (aproximadamente)

---

#### 3.3 Fragment Shader - Aplicação de Alpha

**Modificação:** Canal alpha usa valor de transparência

```glsl
// Localização aproximada: linha 260-263

// Interpola entre a cor do objeto e a cor do fog
finalFragmentColor = mix(fogColor, finalFragmentColor, fogFactor);
}

// ← MODIFICADO: Usa o valor de transparência (d) do material para o canal alpha
FragColor = vec4(finalFragmentColor, d); // d: 1.0 = opaco, 0.0 = transparente
}
```

**Antes:**
```glsl
FragColor = vec4(finalFragmentColor, 1.0);
```

**Depois:**
```glsl
FragColor = vec4(finalFragmentColor, d);
```

**Linha modificada:** 262 (aproximadamente)

---

### 4️⃣ **src/Group.cpp** - Renderização de Grupos

#### 4.1 Envio de Uniform `d`

**Modificação:** Enviar transparência para shader

```cpp
// Localização aproximada: linha 135-140

// Envia as propriedades do material para os shaders
glUniform3fv(glGetUniformLocation(shader.ID,"Ka"), 1, value_ptr(material.Ka)); // Ambiente
glUniform3fv(glGetUniformLocation(shader.ID,"Kd"), 1, value_ptr(material.Kd)); // Difusa
glUniform3fv(glGetUniformLocation(shader.ID,"Ks"), 1, value_ptr(material.Ks)); // Especular
glUniform1f (glGetUniformLocation(shader.ID,"Ns"), material.Ns);               // Brilho
glUniform1f (glGetUniformLocation(shader.ID,"d"), material.d);                 // ← NOVO: Transparência
```

**Linha adicionada:** 139 (aproximadamente)

---

#### 4.2 Controle de Depth Mask

**Modificação:** Desabilitar escrita no depth buffer para objetos transparentes

```cpp
// Localização aproximada: linha 150-167

} else {
    glUniform1i(glGetUniformLocation(shader.ID, "hasDiffuseMap"), false);
}

// ← NOVO: Se o material for transparente (d < 1.0), desabilita escrita no depth buffer
// Isso permite que objetos atrás do vidro sejam visíveis
if (material.d < 1.0f) {
    glDepthMask(GL_FALSE); // Não escreve no depth buffer (permite ver através)
}

glBindVertexArray(VAO);
glDrawArrays(GL_TRIANGLES, 0, vertexCount);
glBindVertexArray(0);
glBindTexture(GL_TEXTURE_2D, 0);

// ← NOVO: Restaura escrita no depth buffer
if (material.d < 1.0f) {
    glDepthMask(GL_TRUE);
}
}
```

**Linhas adicionadas:** 153-157, 163-165 (aproximadamente)

**Explicação:**
- `glDepthMask(GL_FALSE)`: Desabilita escrita no Z-buffer
  - Objetos transparentes **ainda fazem teste de profundidade** (são ocultados por opacos à frente)
  - Mas **não bloqueiam objetos atrás** (permite ver através deles)
- `glDepthMask(GL_TRUE)`: Restaura comportamento normal

---

## 🔧 COMO FUNCIONA

### Pipeline de Transparência

```
┌─────────────────────────────────────────────────────────────┐
│  1. ARQUIVO MTL                                             │
│     newmtl Superb3c_gla                                     │
│     d 0.400000  ← Define 60% de transparência               │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│  2. OBJReader.cpp                                           │
│     Lê propriedade "d" do MTL                               │
│     currentMaterial.d = 0.4                                 │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│  3. Material (classe)                                       │
│     Armazena: material.d = 0.4                              │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│  4. Group::render()                                         │
│     A. Verifica: material.d < 1.0? → SIM                    │
│     B. Executa: glDepthMask(GL_FALSE)                       │
│     C. Envia: glUniform1f("d", 0.4)                         │
│     D. Renderiza geometria                                  │
│     E. Restaura: glDepthMask(GL_TRUE)                       │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│  5. Fragment Shader                                         │
│     uniform float d = 0.4                                   │
│     FragColor = vec4(finalColor, d)                         │
│                                  └─> alpha = 0.4            │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│  6. OpenGL Blending                                         │
│     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)       │
│     color_final = vidro * 0.4 + fundo * 0.6                 │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│  7. RESULTADO                                               │
│     Vidro transparente mostrando interior do carro          │
└─────────────────────────────────────────────────────────────┘
```

---

### Fórmula de Blending

```
Cor Final = Cor do Vidro × Alpha + Cor do Fundo × (1 - Alpha)

Exemplo com d = 0.4:
Cor Final = Cor_Vidro × 0.4 + Cor_Interior × 0.6
            └──────────┘       └────────────────┘
            40% vidro          60% transparente
```

---

### Depth Buffer vs Depth Mask

| Aspecto | Depth Test | Depth Mask |
|---------|------------|------------|
| **O que faz** | Testa se fragmento está à frente | Controla se escreve no Z-buffer |
| **Transparência** | Sempre ON | OFF para objetos transparentes |
| **Resultado** | Vidro pode ser ocultado | Vidro não oculta o interior |

**Configuração:**
```cpp
glEnable(GL_DEPTH_TEST);      // Sempre ativo
glDepthMask(GL_FALSE);        // OFF para transparentes
```

---

## ✅ TESTES

### Material de Teste - Vidro do Carro

**Arquivo:** `models/conversivel.mtl`

```
newmtl Superb3c_gla
Ka 0.400000 0.400000 0.400000
Kd 1.000000 1.000000 1.000000
Ks 0.800000 0.800000 0.800000
d 0.400000                      ← Transparência: 40% opaco
illum 2
Ns 64.0
map_Kd ../textures/Tex_Glass.jpg
```

### Verificação Visual

✅ **Vidros aparecem transparentes**  
✅ **Interior do carro visível através das janelas**  
✅ **Objetos atrás não são bloqueados pelo vidro**  
✅ **Depth test ainda funciona** (vidro oculto por objetos opacos à frente)  
✅ **Iluminação Phong aplicada corretamente nos vidros**  

---

## 📊 COMPARAÇÃO ANTES/DEPOIS

| Aspecto | Antes | Depois |
|---------|-------|--------|
| **Vidros** | Opacos e sólidos | Transparentes |
| **Interior** | Não visível | Visível através do vidro |
| **Propriedade MTL** | `d` ignorada | `d` lida e aplicada |
| **Blending OpenGL** | Desabilitado | Habilitado |
| **Depth Mask** | Sempre ON | OFF para transparentes |
| **Alpha channel** | Sempre 1.0 | Usa valor de `d` |

---

## 🎓 CONCEITOS APLICADOS

### 1. Alpha Blending
- **Conceito:** Mistura de cores usando canal alpha
- **Implementação:** `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`

### 2. Depth Buffer Management
- **Conceito:** Controle de escrita no Z-buffer
- **Implementação:** `glDepthMask()` condicional

### 3. Material Properties
- **Conceito:** Propriedades físicas de materiais (MTL)
- **Implementação:** Leitura e aplicação de `d` (dissolve)

### 4. Shader Uniforms
- **Conceito:** Comunicação CPU → GPU
- **Implementação:** `glUniform1f("d", material.d)`

---

## 🔍 TROUBLESHOOTING

### Problema: Vejo o chão através do vidro, não o interior

**Causa:** Depth mask não desabilitado para transparentes  
**Solução:** Código já implementado em `Group.cpp` linhas 153-165

### Problema: Vidros completamente opacos

**Causa:** Blending não habilitado ou `d = 1.0`  
**Solução:** 
- Verificar `glEnable(GL_BLEND)` em `System.cpp`
- Confirmar valor `d < 1.0` no arquivo MTL

### Problema: Interior invisível

**Causa:** Ordem de renderização incorreta  
**Observação:** OpenGL renderiza na ordem do arquivo - objetos transparentes devem vir depois dos opacos para melhor resultado

---

## 📚 REFERÊNCIAS

- **OpenGL Blending:** https://www.khronos.org/opengl/wiki/Blending
- **MTL Format:** http://paulbourke.net/dataformats/mtl/
- **Depth Buffer:** https://learnopengl.com/Advanced-OpenGL/Depth-testing

---

## ✍️ NOTAS FINAIS

Este sistema de transparência é **básico mas funcional**. Para melhorias futuras, considerar:

1. **Order Independent Transparency (OIT)** - Renderização em múltiplos passes
2. **Weighted Blended OIT** - Algoritmo mais avançado
3. **Sorting** - Ordenar objetos transparentes por distância da câmera
4. **Two-pass rendering** - Opacos primeiro, depois transparentes ordenados

**Data de implementação:** 21 de Novembro de 2025  
**Status:** ✅ Funcional e testado
