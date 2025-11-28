# Fluxo de Implementação do Modelo de Iluminação Phong Completo

**Configurador_Cena.txt → System → Shader → GPU** | Visualizador 3D - OpenGL 4.6

---

## Diagrama de Classes

```mermaid
classDiagram
    class System {
        +loadSystemConfiguration() bool
        +render() void
        -lightPos vec3
        -lightIntensity vec3
        -attConstant float
        -attLinear float
        -attQuadratic float
        -fogEnabled bool
        -fogColor vec3
        -fogDensity float
        -fogType int
        -camera Camera
    }
    
    class Camera {
        +Position vec3
        +Front vec3
        +GetViewMatrix() mat4
        +Zoom float
    }
    
    class Shader {
        +ID GLuint
        -vertexSource string
        -fragmentSource string
    }
    
    class Object3D {
        +render(shader) void
        -mesh Mesh
        -transform mat4
    }
    
    class Mesh {
        +render(shader) void
        -groups vector~Group~
    }
    
    class Group {
        +render(shader) void
        -material Material
        -textureID GLuint
        -VAO GLuint
        -VBO GLuint
    }
    
    class Material {
        +Ka vec3
        +Kd vec3
        +Ks vec3
        +Ns float
        +map_Kd string
    }
    
    System --> Camera : usa
    System --> Shader : usa
    System --> Object3D : renderiza N
    Object3D --> Mesh : contém
    Mesh --> Group : contém N
    Group --> Material : possui
```

---

## Sequência de Implementação

```mermaid
sequenceDiagram
    autonumber
    participant Config as Configurador_Cena.txt
    participant Sys as System
    participant Shd as Shader
    participant Obj as Object3D
    participant Grp as Group
    participant GPU as Fragment Shader GPU
    
    Note over Config: Arquivo de configuração
    Config->>Sys: loadSystemConfiguration()
    Note over Sys: CAMERA 0 2 20
    Sys->>Sys: camera.Position = vec3
    Note over Sys: LIGHT 0 10 5 1 1 1
    Sys->>Sys: lightPos, lightIntensity = vec3
    Note over Sys: ATTENUATION 1.0 0.045 0.0075
    Sys->>Sys: attConstant, attLinear, attQuadratic
    Note over Sys: FOG 1 0.9 0.9 0.9 0.05...
    Sys->>Sys: fogEnabled, fogColor, fogDensity, fogType
    
    Note over Sys: Loop de Renderização
    Sys->>Shd: glUseProgram(ID)
    
    Note over Sys: Uniforms Globais
    Sys->>Shd: glUniform3fv(lightPos)
    Sys->>Shd: glUniform3fv(lightIntensity)
    Sys->>Shd: glUniform3fv(viewPos)
    Sys->>Shd: glUniform1f(attConstant/Linear/Quadratic)
    Sys->>Shd: glUniform1i(fogEnabled)
    Sys->>Shd: glUniform3fv(fogColor)
    Sys->>Shd: glUniform1f(fogDensity)
    Sys->>Shd: glUniform1i(fogType)
    
    loop Para cada objeto
        Sys->>Obj: render(shader)
        Obj->>Grp: render(shader)
        
        Note over Grp: Uniforms de Material
        Grp->>Shd: glUniform3fv(Ka, Kd, Ks)
        Grp->>Shd: glUniform1f(Ns)
        Grp->>Shd: glUniform1i(hasDiffuseMap)
        
        Grp->>GPU: glDrawArrays()
        
        Note over GPU: Cálculo de Phong
        GPU->>GPU: ambient = Ka * lightIntensity
        GPU->>GPU: diffuse = Kd * diff * attenuation
        GPU->>GPU: specular = Ks * spec * attenuation
        GPU->>GPU: phongColor = ambient + diffuse + specular
        
        alt Fog habilitado
            GPU->>GPU: fogFactor = calc(fogType, distance)
            GPU->>GPU: finalColor = mix(fogColor, phongColor, fogFactor)
        else Fog desabilitado
            GPU->>GPU: finalColor = phongColor
        end
        
        GPU->>GPU: FragColor = vec4(finalColor, 1.0)
    end
```

---

## Fluxo de Dados: Configuração → GPU

```mermaid
flowchart TB
    subgraph "Configurador_Cena.txt"
        A1[CAMERA x y z]
        A2[LIGHT pos.x pos.y pos.z<br/>intensity.r intensity.g intensity.b]
        A3[ATTENUATION constant linear quadratic]
        A4[FOG enabled r g b density start end type]
    end
    
    subgraph "System - CPU"
        B1[camera.Position]
        B2[lightPos, lightIntensity]
        B3[attConstant, attLinear, attQuadratic]
        B4[fogEnabled, fogColor, fogDensity, fogType]
    end
    
    subgraph "Shader Uniforms - GPU"
        C1[uniform vec3 viewPos]
        C2[uniform vec3 lightPos<br/>uniform vec3 lightIntensity]
        C3[uniform float attConstante<br/>uniform float attLinear<br/>uniform float attQuadratica]
        C4[uniform bool fogEnabled<br/>uniform vec3 fogColor<br/>uniform float fogDensity<br/>uniform int fogType]
    end
    
    subgraph "Material - CPU/GPU"
        D1[Material.Ka, Kd, Ks, Ns]
        D2[Group.textureID]
    end
    
    subgraph "Fragment Shader - Cálculo"
        E1[Componente Ambiente<br/>Ka * lightIntensity]
        E2[Componente Difusa<br/>Kd * diff * attenuation]
        E3[Componente Especular<br/>Ks * spec * attenuation]
        E4[Fog<br/>mix fogColor, phongColor]
        E5[FragColor final]
    end
    
    A1 --> B1
    A2 --> B2
    A3 --> B3
    A4 --> B4
    
    B1 --> C1
    B2 --> C2
    B3 --> C3
    B4 --> C4
    
    C1 --> E2
    C1 --> E3
    C1 --> E4
    C2 --> E1
    C2 --> E2
    C2 --> E3
    C3 --> E2
    C3 --> E3
    C4 --> E4
    
    D1 --> E1
    D1 --> E2
    D1 --> E3
    D2 --> E1
    D2 --> E2
    
    E1 --> E5
    E2 --> E5
    E3 --> E5
    E4 --> E5
```

---

## Métodos Principais

| Classe | Métodos Chave |
|--------|---------------|
| **System** | `loadSystemConfiguration()` `render()` |
| **Object3D** | `render(shader)` |
| **Mesh** | `render(shader)` |
| **Group** | `render(shader)` - Envia Ka, Kd, Ks, Ns, textureID |
| **Shader** | `glUseProgram()` `glUniform*()` |

### Parâmetros do Configurador_Cena.txt

```text
CAMERA x y z
  → camera.Position = vec3(x, y, z)

LIGHT pos.x pos.y pos.z intensity.r intensity.g intensity.b
  → lightPos = vec3(pos.x, pos.y, pos.z)
  → lightIntensity = vec3(intensity.r, intensity.g, intensity.b)

ATTENUATION constant linear quadratic
  → attConstant = constant (c1 nos slides)
  → attLinear = linear (c2 nos slides)
  → attQuadratic = quadratic (c3 nos slides)
  → attenuation = 1.0 / (c1 + c2*d + c3*d²)

FOG enabled r g b density start end type
  → fogEnabled = bool(enabled)
  → fogColor = vec3(r, g, b)
  → fogDensity = density
  → fogType = type (0=linear, 1=exponencial, 2=exponencial²)
```

---

## Cálculos no Fragment Shader

### 1. Componente Ambiente

```glsl
vec3 ambient = Ka * lightIntensity * baseColor;
```

### 2. Componente Difusa

```glsl
vec3 lightDir = normalize(lightPos - elementPosition);
float diff = max(dot(norm, lightDir), 0.0);
float distance = length(lightPos - elementPosition);
float attenuation = 1.0 / (attConstante + attLinear * distance + attQuadratica * distance²);
vec3 diffuse = Kd * diff * attenuation * lightIntensity * baseColor;
```

### 3. Componente Especular

```glsl
vec3 viewDir = normalize(viewPos - elementPosition);
vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), Ns);
vec3 specular = Ks * spec * attenuation * lightIntensity;
```

### 4. Fog

```glsl
if (fogEnabled) {
    float fogDistance = length(viewPos - elementPosition);
    float fogFactor;
    
    if (fogType == 0)      // Linear
        fogFactor = 1 / fogDistance;
    else if (fogType == 1) // Exponencial
        fogFactor = exp(-fogDensity * fogDistance);
    else if (fogType == 2) // Exponencial²
        fogFactor = exp(-pow(fogDensity * fogDistance, 2.0));
    
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    finalColor = mix(fogColor, phongColor, fogFactor);
}
```

### 5. Cor Final

```glsl
vec3 phongColor = ambient + diffuse + specular;
vec3 finalColor = fogEnabled ? mix(fogColor, phongColor, fogFactor) : phongColor;
FragColor = vec4(finalColor, 1.0);
```

---

## Estrutura de Uniforms

### Enviados 1 vez por Frame (Globais)

| Uniform | Tipo | Origem |
|---------|------|--------|
| `projection` | mat4 | Camera.Zoom + aspect ratio |
| `view` | mat4 | Camera.GetViewMatrix() |
| `lightPos` | vec3 | System.lightPos |
| `lightIntensity` | vec3 | System.lightIntensity |
| `viewPos` | vec3 | Camera.Position |
| `attConstant` | float | System.attConstant |
| `attLinear` | float | System.attLinear |
| `attQuadratic` | float | System.attQuadratic |
| `fogEnabled` | bool | System.fogEnabled |
| `fogColor` | vec3 | System.fogColor |
| `fogDensity` | float | System.fogDensity |
| `fogType` | int | System.fogType |

### Enviados "N" vezes por frame (N = número de Objetos da cena)

| Uniform | Tipo | Origem |
|---------|------|--------|
| `model` | mat4 | Object3D.transform |
| `Ka` | vec3 | Material.Ka |
| `Kd` | vec3 | Material.Kd |
| `Ks` | vec3 | Material.Ks |
| `Ns` | float | Material.Ns |
| `hasDiffuseMap` | bool | Group.textureID != 0 |
| `diffuseMap` | sampler2D | Group.textureID |

---

## Exemplo de Configuração

**Configurador_Cena.txt:**

```ini
# Iluminação e Efeitos
CAMERA 0.0 2.0 20.0
LIGHT 0.0 10.0 5.0 1.0 1.0 1.0
ATTENUATION 1.0 0.045 0.0075
FOG 1 0.9 0.9 0.9 0.05 10.0 50.0 1

# Objetos
Pista models/pista.obj 0.0 -0.5 0.0 0.0 0.0 0.0 1.0 1.0 1.0 0
```

**Resultado no Shader:**

```glsl
// Globais (1x por frame)
viewPos = vec3(0.0, 2.0, 20.0)           // Posição da câmera
lightPos = vec3(0.0, 10.0, 5.0)          // Luz acima da cena
lightIntensity = vec3(1.0, 1.0, 1.0)     // Luz branca
attConstante = 1.0, attLinear = 0.045, attQuadratica = 0.0075
fogEnabled = true
fogColor = vec3(0.9, 0.9, 0.9)           // Névoa cinza claro
fogDensity = 0.05
fogType = 1                               // Exponencial

// Por objeto (N vezes por frame)
Ka = vec3(0.2, 0.2, 0.2)                 // Ambiente escuro
Kd = vec3(0.8, 0.8, 0.8)                 // Difusa clara
Ks = vec3(0.5, 0.5, 0.5)                 // Especular média
Ns = 32.0                                 // Brilho moderado
```

---

**Autores:** Ian Rossetti Boniatti e Eduardo Tropea  
**Curso:** Jogos Digitais - Unisinos  
**Disciplina:** Computação Gráfica em Tempo Real  
**Data:** Novembro 2025
