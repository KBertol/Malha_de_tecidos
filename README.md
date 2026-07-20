# Simulação de Tecido com Sistema Massa-Mola em OpenGL

Simulação de tecido em tempo real usando um **sistema massa-mola**,
renderizada em **C++17 / OpenGL 3.3 Core Profile**, com câmera
orbital, vento interativo e colisão com o chão.

Trabalho de **Computação Gráfica**, desenvolvido em dupla.

![Linguagem](https://img.shields.io/badge/C%2B%2B-17-blue)
![OpenGL](https://img.shields.io/badge/OpenGL-3.3%20Core-green)

## Como funciona a simulação

O tecido é modelado como uma grade de **partículas** conectadas por
**molas**, seguindo o modelo clássico de Provot para simulação de
tecidos:

- **Partículas** (`Particle.h`): cada uma tem posição, velocidade,
  força acumulada e massa; a integração usa Euler semi-implícito.
- **Molas** (`Spring.h`): três tipos, cada um restringindo um tipo de
  deformação diferente —
  - **Structural**: conecta vizinhos diretos (mantém a distância entre
    partículas adjacentes)
  - **Shear**: conecta diagonais (resiste a cisalhamento)
  - **Bend**: conecta partículas a 2 de distância (resiste a dobras
    bruscas, dá rigidez ao tecido)

  A força de cada mola segue a **Lei de Hooke** com um termo de
  amortecimento proporcional à velocidade relativa entre as duas
  partículas, evitando oscilação infinita.
- **Tecido** (`Cloth.h`/`Cloth.cpp`): monta a grade de partículas e
  molas, aplica forças externas (gravidade e vento), resolve colisão
  simples com o chão e recalcula as normais a cada frame para a
  iluminação.
- A atualização física roda em múltiplos **sub-passos** por frame
  para manter a simulação estável mesmo com molas rígidas.

## Controles

| Tecla | Ação |
|---|---|
| Setas | Orbitar a câmera |
| `W` / `S` | Aproximar / afastar a câmera |
| `F` | Ligar/desligar o vento |
| `V` | Alternar modo wireframe |
| Mouse | Controla a direção do vento |
| `ESC` | Sair |

## Estrutura do projeto

```
├── src/
│   ├── main.cpp        -> loop principal, câmera, entrada e renderização
│   ├── Particle.h        -> partícula (posição, velocidade, integração)
│   ├── Spring.h           -> mola (Lei de Hooke + amortecimento)
│   ├── Cloth.h / .cpp      -> grade de partículas/molas, física do tecido
│   ├── Scene.h              -> geometria do chão e da haste de sustentação
│   └── Shader.h / .cpp       -> carregamento e compilação dos shaders GLSL
├── shaders/
│   ├── cloth.vert       -> vertex shader
│   └── cloth.frag       -> fragment shader
├── external/glad/        -> loader OpenGL vendorizado
└── CMakeLists.txt
```

> O projeto usa **glad** (vendorizado em `external/glad/`) em vez de
> GLEW para carregar as funções OpenGL, evitando um problema de
> incompatibilidade do GLEW 2.2.0 com versões recentes do Mesa em
> sistemas Linux/Wayland.

## Como executar

### Pré-requisitos
- CMake ≥ 3.10
- Compilador C++17
- OpenGL, GLFW3 e GLM instalados no sistema

### Build

```bash
mkdir build && cd build
cmake ..
make
./cloth_sim
```

## Tecnologias

- C++17
- OpenGL 3.3 Core Profile (via glad)
- GLFW3 (janela e entrada)
- GLM (álgebra linear)
- CMake

## Equipe

- Integrantes: Karla Bertol & Vitor Cardoso Burgarelli
