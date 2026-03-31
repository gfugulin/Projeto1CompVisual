# Projeto 1 — Processamento de Imagens

**Universidade Presbiteriana Mackenzie**  
Faculdade de Computação e Informática — Ciência da Computação  
Disciplina: **Computação Visual**  
Professor: **André Kishimoto**

## Integrantes do Grupo

| Nome | RA |
|---|---|
| Gustavo Fugulin Soares da Silva | 10418552 |
| Yuri Milliet da Silva | 10417884 |
| Lucas Eleutério da Silva | 10402122 |

---

## Descrição do Projeto

Este software realiza o processamento de imagens utilizando C++17 e a biblioteca SDL3 (Simple DirectMedia Layer). A aplicação recebe uma imagem como argumento de linha de comando, converte automaticamente para escala de cinza, calcula e exibe o histograma com análise estatística (média, desvio padrão, classificação de brilho e contraste), e permite a equalização do histograma para realce de contraste. A interface gráfica é composta por duas janelas integradas que permitem a visualização em tempo real das transformações aplicadas.

---

## Funcionalidades

### 1. Carregamento de Imagem
- Suporte para formatos PNG, JPG e BMP via biblioteca `SDL3_image`.
- Tratamento de erros para arquivos inexistentes ou formatos inválidos.
- Normalização automática interna para o formato `RGBA32` para manipulação uniforme de pixels.

### 2. Conversão para Escala de Cinza
- Detecção automática: verifica se a imagem já está em escala de cinza comparando se `R == G == B` em cada pixel.
- Para imagens coloridas, aplica a fórmula de luminância ITU-R BT.709:
  `Y = 0.2125 * R + 0.7154 * G + 0.0721 * B`

### 3. Interface com Duas Janelas
- **Janela Principal**: Exibe a imagem processada e ajusta automaticamente suas dimensões à resolução da imagem (limitada a 85% da área útil do monitor).
- **Janela Secundária**: Vinculada à principal (janela filha), exibe o histograma gráfico, dados analíticos e controles de interface. Tamanho fixo de 400x600 pixels.

### 4. Análise de Histograma
- Representação gráfica da distribuição de intensidades (0–255) como gráfico de barras normalizado.
- Cálculo de média aritmética para classificação de brilho (Clara, Média, Escura).
- Cálculo de desvio padrão para classificação de contraste (Alto, Médio, Baixo).

### 5. Equalização de Histograma
- Implementação via Função de Distribuição Acumulada (CDF) para redistribuição dos tons de cinza.
- Botão interativo com feedback visual em três estados: Normal, Hover e Pressed.
- Alternância (toggle) entre a imagem equalizada e a original em escala de cinza, sem recarregar o arquivo do disco.

### 6. Exportação
- Tecla **S**: Salva a imagem atualmente exibida no formato PNG como `output_image.png`.

---

## Estrutura do Projeto

```
ComputacaoVisual-Projeto1/
├── src/                            <- Codigo-fonte C++
│   ├── main.cpp                    <- Ponto de entrada, integracao e loop de eventos
│   ├── image_processor.h / .cpp    <- Carga, conversao cinza e salvamento
│   ├── histogram.h / .cpp          <- Histograma: calculo, analise, equalizacao e renderizacao
│   └── gui.h / .cpp                <- Janelas, botao (3 estados) e exibicao de texto
├── assets/                         <- Recursos (fonte Roboto-Regular.ttf)
├── libs/                           <- Bibliotecas SDL3 (nao versionadas no Git)
│   ├── SDL3-3.2.8/
│   ├── SDL3_image-3.2.4/
│   └── SDL3_ttf-3.2.2/
├── build/                          <- Arquivos .o intermediarios (nao versionados)
├── build.bat                       <- Script de compilacao e copia de DLLs
├── .gitignore                      <- Regras de exclusao do Git
└── README.md                       <- Este arquivo
```

A organização segue o princípio de separação de responsabilidades: cada módulo cuida de uma parte do sistema. Os headers (`.h`) declaram a interface pública e as implementações (`.cpp`) contêm a lógica. Isso permite compilar cada módulo independentemente e facilita a manutenção.

---

## Bibliotecas Utilizadas

| Biblioteca | Finalidade | Onde é usada |
|---|---|---|
| **SDL3** | Criação de janelas, renderização gráfica, captura de eventos | Todos os módulos |
| **SDL3_image** | Carregamento de imagens em formatos PNG, JPG e BMP | `image_processor.cpp` |
| **SDL3_ttf** | Renderização de texto usando fontes TrueType (.ttf) | `gui.cpp` |

SDL (Simple DirectMedia Layer) é uma biblioteca em C que abstrai o acesso ao hardware gráfico de forma multiplataforma. A versão 3 (SDL3) é a mais recente e foi utilizada neste projeto por ser a versão exigida no enunciado.

---

## Como o Projeto Funciona

### Fluxo geral do programa

```
1. Recebe o caminho da imagem como argumento
2. Inicializa SDL3 e SDL_ttf
3. Carrega a imagem e converte para escala de cinza
4. Calcula o histograma (media, desvio padrao, classificacao)
5. Cria as duas janelas e renderers
6. Carrega a fonte TTF para renderizacao de texto
7. Entra no loop principal de eventos
   -> Trata teclado (S para salvar, ESC para sair)
   -> Trata mouse (hover e clique no botao)
   -> Redesenha as janelas quando necessario
8. Ao encerrar, libera todos os recursos na ordem inversa
```

A seguir, cada módulo é explicado em detalhe.

---

### Módulo: `image_processor.h` / `image_processor.cpp`

Responsável pela manipulação de imagens: carregar do disco, verificar se é colorida ou cinza, converter para escala de cinza e salvar.

#### Conceito: SDL_Surface

Uma `SDL_Surface` é uma representação de uma imagem na memória RAM. Ela contém:
- `w` e `h` — largura e altura em pixels
- `pixels` — ponteiro para os dados brutos dos pixels
- `pitch` — número de bytes por linha (pode ter padding)
- `format` — formato dos pixels (ex: RGBA32 = 4 bytes por pixel: R, G, B, A)

Diferente de uma `SDL_Texture` (que fica na GPU), a Surface permite acesso direto aos pixels, o que é necessário para processamento de imagem.

#### Conceito: RGBA32

Formato de pixel onde cada pixel ocupa 4 bytes:
```
pixel[0] = R (vermelho, 0-255)
pixel[1] = G (verde, 0-255)
pixel[2] = B (azul, 0-255)
pixel[3] = A (alfa/transparencia, 0-255)
```
Ao carregar qualquer imagem, convertemos para RGBA32. Isso simplifica o acesso aos pixels, já que diferentes formatos (JPG usa RGB24, PNG pode usar RGBA) teriam layouts distintos.

#### Função `loadImage(path)`

```cpp
SDL_Surface* loadImage(const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) return nullptr;

    SDL_Surface* converted = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surface);

    return converted;
}
```

A função carrega a imagem com `IMG_Load` (que aceita PNG, JPG, BMP) e converte para RGBA32 para que o restante do código sempre acesse os pixels da mesma forma.

#### Função `isGrayscale(surface)`

```cpp
bool isGrayscale(SDL_Surface* surface) {
    for (cada pixel) {
        Uint8 r = pixel[0], g = pixel[1], b = pixel[2];
        if (r != g || r != b) return false;
    }
    return true;
}
```

Uma imagem em escala de cinza tem R == G == B em todos os pixels. Se encontrarmos qualquer pixel onde isso não vale, a imagem é colorida.

O endereço de cada pixel é calculado como:
```cpp
Uint8* pixel = pixels + y * pitch + x * bpp;
```
Onde `pitch` é o número de bytes por linha e `bpp` é o número de bytes por pixel (4 para RGBA32). Antes de acessar os pixels, usamos `SDL_LockSurface` para garantir que os dados são acessíveis.

#### Função `convertToGrayscale(surface)`

```cpp
Uint8 gray = (Uint8)(0.2125 * r + 0.7154 * g + 0.0721 * b);
dstPixel[0] = gray;  // R
dstPixel[1] = gray;  // G
dstPixel[2] = gray;  // B
dstPixel[3] = a;     // preserva transparencia
```

A fórmula `Y = 0.2125R + 0.7154G + 0.0721B` é a fórmula de luminância ITU-R BT.709, utilizada em conversão de vídeo HD. Os pesos refletem a sensibilidade do olho humano a cada cor:
- Verde (71.54%): maior sensibilidade
- Vermelho (21.25%): sensibilidade intermediária
- Azul (7.21%): menor sensibilidade

Uma média simples `(R+G+B)/3` não preservaria a percepção de brilho corretamente. A função cria uma nova surface em vez de modificar a original, pois o programa precisa manter a versão original para permitir a reversão sem recarregar do disco.

#### Função `saveImage(surface, path)`

```cpp
bool saveImage(SDL_Surface* surface, const std::string& path) {
    if (!IMG_SavePNG(surface, path.c_str())) {
        SDL_Log("Erro ao salvar...");
        return false;
    }
    return true;
}
```

Salva a surface atual como arquivo PNG usando `IMG_SavePNG`. Sobrescreve o arquivo destino caso ele já exista.

---

### Módulo: `histogram.h` / `histogram.cpp`

Responsável pelo cálculo, análise, equalização e renderização do histograma.

#### Estrutura `HistogramData`

```cpp
struct HistogramData {
    int bins[256] = {0};      // frequencia de cada nivel de cinza (0-255)
    int totalPixels = 0;      // total de pixels na imagem
    double mean = 0.0;        // media de intensidade
    double stddev = 0.0;      // desvio padrao
    std::string brightness;   // "clara", "media" ou "escura"
    std::string contrast;     // "alto", "medio" ou "baixo"
};
```

Um histograma de imagem é um gráfico de barras com 256 posições (uma para cada nível de cinza, de 0=preto a 255=branco). Cada posição indica quantos pixels têm aquele nível de intensidade.

#### Função `calculateHistogram(surface)`

```cpp
HistogramData calculateHistogram(SDL_Surface* surface) {
    // 1. Conta a frequencia de cada nivel de cinza
    for (cada pixel) {
        Uint8 gray = pixel[0];
        data.bins[gray]++;
    }

    // 2. Calcula a media: soma(i * bins[i]) / totalPixels
    for (int i = 0; i < 256; i++)
        sum += (double)i * data.bins[i];
    data.mean = sum / data.totalPixels;

    // 3. Calcula o desvio padrao: raiz(soma((i - media)^2 * bins[i]) / totalPixels)
    for (int i = 0; i < 256; i++) {
        double diff = (double)i - data.mean;
        variance += diff * diff * data.bins[i];
    }
    data.stddev = sqrt(variance / data.totalPixels);

    // 4. Classifica brilho e contraste
    // Brilho: media < 85 -> escura, < 170 -> media, >= 170 -> clara
    // Contraste: desvio < 40 -> baixo, < 80 -> medio, >= 80 -> alto
}
```

A média de intensidade indica o brilho geral da imagem: valores baixos indicam imagem escura, valores altos indicam imagem clara. O desvio padrão indica o contraste: desvio baixo significa pixels concentrados em um intervalo pequeno (pouco contraste), desvio alto significa pixels espalhados por toda a faixa 0-255 (alto contraste). Os limites de classificação (85, 170 para brilho; 40, 80 para contraste) dividem o espectro em três faixas.

#### Função `equalizeHistogram(surface)`

Esta é a função mais complexa do projeto. Ela redistribui os níveis de cinza para que a imagem tenha melhor contraste.

```cpp
SDL_Surface* equalizeHistogram(SDL_Surface* surface) {
    // 1. Calcula o histograma original
    int histogram[256] = {0};
    for (cada pixel) histogram[pixel[0]]++;

    // 2. Calcula o CDF (Cumulative Distribution Function)
    int cdf[256];
    cdf[0] = histogram[0];
    for (int i = 1; i < 256; i++)
        cdf[i] = cdf[i-1] + histogram[i];

    // 3. Encontra o menor CDF nao-zero (cdfMin)
    int cdfMin = /* primeiro cdf[i] > 0 */;

    // 4. Cria a tabela de mapeamento (lookup table)
    for (int i = 0; i < 256; i++)
        lookupTable[i] = round(((cdf[i] - cdfMin) / (totalPixels - cdfMin)) * 255);

    // 5. Aplica o mapeamento em cada pixel
    for (cada pixel)
        novoCinza = lookupTable[cinzaOriginal];
}
```

O algoritmo passo a passo:
1. **Histograma**: conta quantos pixels possuem cada nível de cinza.
2. **CDF (Função de Distribuição Acumulada)**: para cada nível `i`, soma todos os pixels do nível 0 até `i`. Se `histogram = [10, 20, 30]`, então `cdf = [10, 30, 60]`.
3. **cdfMin**: o primeiro valor não-zero do CDF, usado para normalizar a distribuição.
4. **Lookup Table**: a fórmula `((cdf[i] - cdfMin) / (total - cdfMin)) * 255` mapeia cada nível antigo para um nível novo, redistribuindo os valores uniformemente no intervalo 0-255.
5. **Aplicação**: substitui o valor de cinza de cada pixel usando a tabela.

A lookup table é uma otimização: em vez de recalcular o mapeamento para cada pixel individualmente, calculamos os 256 mapeamentos uma vez e depois apenas consultamos a tabela.

O resultado prático é que imagens com pouco contraste (pixels concentrados num intervalo pequeno) ganham mais definição, com os tons redistribuídos por toda a faixa de intensidades.

#### Função `renderHistogram(renderer, data, x, y, w, h)`

```cpp
void renderHistogram(SDL_Renderer* renderer, const HistogramData& data,
                     float x, float y, float w, float h) {
    int maxVal = max(bins[0..255]);       // valor maximo para normalizar
    float barWidth = w / 256.0f;

    for (int i = 0; i < 256; i++) {
        float barHeight = (bins[i] / maxVal) * h;
        // desenha a barra de baixo para cima
    }
}
```

As barras são normalizadas: a barra mais alta (nível de cinza mais frequente) ocupa toda a altura disponível, e as demais são proporcionais. Sem essa normalização, valores muito grandes distorceriam o gráfico.

---

### Módulo: `gui.h` / `gui.cpp`

Gerencia a interface gráfica: criação das duas janelas, o botão de equalização com três estados visuais e a renderização de texto.

#### Estruturas

```cpp
enum class ButtonState {
    NORMAL,   // Azul #3B82F6
    HOVER,    // Azul claro #60A5FA (mouse sobre o botao)
    PRESSED   // Azul escuro #1D4ED8 (durante o clique)
};

struct Button {
    SDL_FRect rect;        // posicao e dimensoes {x, y, largura, altura}
    ButtonState state;     // estado visual atual
    std::string text;      // "Equalizar" ou "Ver original"
    bool equalized;        // flag de toggle
};
```

O `enum class` é utilizado para os estados do botão em vez de um enum tradicional, pois oferece escopo próprio e maior segurança de tipos.

#### Função `createMainWindow(imageWidth, imageHeight)`

```cpp
SDL_Window* createMainWindow(int imageWidth, int imageHeight) {
    // 1. Obtem as dimensoes do monitor principal
    SDL_DisplayID displayID = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(displayID);

    // 2. Limita o tamanho da janela a 85% da tela
    int maxW = mode->w * 0.85f;
    int maxH = mode->h * 0.85f;
    int winW = min(imageWidth, maxW);
    int winH = min(imageHeight, maxH);

    // 3. Cria a janela e centraliza deslocando para a esquerda
    //    (para dar espaco a janela secundaria ao lado)
    SDL_Window* window = SDL_CreateWindow("Processamento de Imagens", winW, winH, 0);
    int posX = (mode->w - winW - SECONDARY_WINDOW_WIDTH - 10) / 2;
    int posY = (mode->h - winH) / 2;
    SDL_SetWindowPosition(window, posX, posY);
}
```

O limite de 85% da tela evita que uma imagem muito grande crie uma janela maior que o monitor. O deslocamento para a esquerda garante que as duas janelas caibam lado a lado.

#### Função `createSecondaryWindow(parentWindow)`

```cpp
SDL_Window* createSecondaryWindow(SDL_Window* parentWindow) {
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Histograma");
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, 400);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, 600);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, parentX + parentW + 10);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, parentY);

    // Define a janela como filha da principal
    SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_PARENT_POINTER, parentWindow);

    SDL_Window* window = SDL_CreateWindowWithProperties(props);
}
```

A função `SDL_CreateWindowWithProperties` é utilizada em vez da `SDL_CreateWindow` simples porque permite definir a propriedade `PARENT_POINTER`, tornando a janela filha da principal. Uma janela filha é vinculada ao seu pai e, em alguns sistemas, se move junto com ele e não aparece separadamente na barra de tarefas.

#### Função `renderButton(renderer, font, button)`

```cpp
void renderButton(SDL_Renderer* renderer, TTF_Font* font, const Button& button) {
    SDL_Color bgColor;
    switch (button.state) {
        case HOVER:   bgColor = {96, 165, 250, 255};  break;
        case PRESSED: bgColor = {29, 78, 216, 255};    break;
        default:      bgColor = {59, 130, 246, 255};   break;
    }

    SDL_RenderFillRect(renderer, &button.rect);   // fundo colorido
    SDL_RenderRect(renderer, &button.rect);       // borda
    renderText(renderer, font, button.text, centroX, centroY, branco);
}
```

O botão é desenhado inteiramente com primitivas da SDL (retângulos preenchidos e bordas), sem uso de imagens. A cor de fundo muda conforme o estado.

#### Função `renderText(renderer, font, text, x, y, color)`

```cpp
void renderText(SDL_Renderer* renderer, TTF_Font* font,
                const std::string& text, float x, float y, SDL_Color color) {
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), text.length(), color);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    SDL_FRect dstRect = {x - textW/2, y - textH/2, textW, textH};
    SDL_RenderTexture(renderer, textTexture, nullptr, &dstRect);

    SDL_DestroyTexture(textTexture);
    SDL_DestroySurface(textSurface);
}
```

A `SDL_ttf` gera texto como uma Surface (RAM). Para exibir na tela, é necessário converter para Texture (GPU) via `SDL_CreateTextureFromSurface`. A textura é criada e destruída a cada renderização.

---

### Módulo: `main.cpp`

Ponto de entrada do programa. Integra todos os módulos, gerencia o ciclo de vida da aplicação e trata eventos do usuário.

#### Loop Principal

```cpp
while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (SDLK_ESCAPE) running = false;
                if (SDLK_S) saveImage(...);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                // atualiza estado do botao (NORMAL / HOVER)
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                // atualiza estado do botao (PRESSED)
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                // executa acao: equalizar ou reverter
                break;
        }
    }

    if (needsRedraw) { /* redesenha janela principal */ }
    if (needsSecondaryRedraw) { /* redesenha janela secundaria */ }

    SDL_Delay(16);  // ~60 FPS
}
```

O programa utiliza `SDL_PollEvent` (não-bloqueante) em vez de `SDL_WaitEvent` (bloqueante), combinado com `SDL_Delay(16)` para manter aproximadamente 60 frames por segundo com uso mínimo de CPU. A flag `needsRedraw` evita redesenhar a tela desnecessariamente, atualizando apenas quando algo de fato muda.

#### Lógica do Botão

```cpp
case SDL_EVENT_MOUSE_BUTTON_UP:
    if (clicou_no_botao) {
        if (!eqButton.equalized) {
            // versao original -> equaliza
            equalizedSurface = equalizeHistogram(graySurface);
            currentSurface = equalizedSurface;
            eqButton.text = "Ver original";
            eqButton.equalized = true;
        } else {
            // equalizada -> reverte para original
            currentSurface = graySurface;
            eqButton.text = "Equalizar";
            eqButton.equalized = false;
        }
        histData = calculateHistogram(currentSurface);
    }
```

O botão funciona como toggle: alterna entre a versão equalizada e a original. A reversão não recarrega a imagem do disco — as duas versões (original e equalizada) são mantidas simultaneamente na memória, e o ponteiro `currentSurface` é trocado conforme a ação do usuário.

#### Gerenciamento de Memória

```cpp
if (equalizedSurface) SDL_DestroySurface(equalizedSurface);
SDL_DestroySurface(graySurface);
SDL_DestroySurface(originalGray);

if (fontSmall) TTF_CloseFont(fontSmall);
if (font) TTF_CloseFont(font);

SDL_DestroyRenderer(secondaryRenderer);
SDL_DestroyRenderer(mainRenderer);
SDL_DestroyWindow(secondaryWindow);
SDL_DestroyWindow(mainWindow);

TTF_Quit();
SDL_Quit();
```

A liberação de recursos segue a ordem inversa da criação: primeiro os recursos gráficos (surfaces, fonts), depois os renderers, depois as janelas, e por último as bibliotecas. A ordem é importante porque destruir um recurso antes de outro que depende dele pode causar falhas de acesso à memória.

---

### Arquivo `build.bat`

```batch
@echo off
REM Compila cada .cpp em .o (objeto)
g++ -std=c++17 -Wall -Wextra -O2 -static-libgcc -static-libstdc++ -c src\main.cpp -o build\main.o
g++ ... -c src\image_processor.cpp -o build\image_processor.o
g++ ... -c src\histogram.cpp -o build\histogram.o
g++ ... -c src\gui.cpp -o build\gui.o

REM Liga todos os .o num unico executavel
g++ ... build\*.o -o imgproc.exe -lSDL3 -lSDL3_image -lSDL3_ttf

REM Copia as DLLs para a pasta do executavel
copy SDL3.dll .
copy SDL3_image.dll .
copy SDL3_ttf.dll .
```

A compilação ocorre em duas etapas:
1. **Compilação** (`.cpp` para `.o`): o compilador verifica a sintaxe e gera código objeto para cada arquivo separadamente.
2. **Linkagem** (`.o` para `.exe`): o linker junta todos os arquivos objeto e resolve referências cruzadas entre eles.

A flag `-static-libgcc -static-libstdc++` incorpora as bibliotecas runtime do GCC no executável, eliminando dependências externas do compilador. As flags `-lSDL3 -lSDL3_image -lSDL3_ttf` indicam ao linker quais bibliotecas SDL conectar.

---

## Requisitos e Dependências

| Componente | Especificação |
|---|---|
| Compilador | g++ 15.1.0 (MinGW-w64) |
| Padrão C++ | C++17 |
| SDL3 | Versão 3.2.8 |
| SDL3_image | Versão 3.2.4 |
| SDL3_ttf | Versão 3.2.2 |

---

## Compilação e Execução

### Configuração das Bibliotecas

As bibliotecas SDL3 devem ser baixadas nas seguintes URLs e extraídas no diretório `libs/`:
- SDL3: https://github.com/libsdl-org/SDL/releases/tag/release-3.2.8
- SDL3_image: https://github.com/libsdl-org/SDL_image/releases/tag/release-3.2.4
- SDL3_ttf: https://github.com/libsdl-org/SDL_ttf/releases/tag/release-3.2.2

A estrutura esperada é: `libs/SDL3-<versao>/x86_64-w64-mingw32/`

A fonte Roboto-Regular.ttf deve ser colocada na pasta `assets/`.

### Compilação

No terminal, na raiz do projeto:
```powershell
.\build.bat
```

### Execução

```powershell
.\imgproc.exe caminho_da_imagem.ext
```

Exemplos:
```powershell
.\imgproc.exe teste.jpg
.\imgproc.exe "C:\Minhas Fotos\foto.png"
```

### Controles

| Acao | Comando |
|---|---|
| Salvar imagem atual | Tecla **S** |
| Encerrar aplicacao | Tecla **ESC** ou fechar a janela |
| Equalizar histograma | Clique no botao "Equalizar" |
| Reverter para original | Clique no botao "Ver original" |

---

## Contribuições

| Integrante | Responsabilidade |
|---|---|
| **Gustavo Fugulin Soares da Silva** | Integração geral (`main.cpp`), documentação (README) |
| **Yuri Milliet da Silva** | Carregamento de imagens e conversão para escala de cinza (`image_processor.cpp`) |
| **Lucas Eleutério da Silva** | Interface gráfica, janelas e componentes de botão (`gui.cpp`), cálculo, equalização e renderização do histograma (`histogram.cpp`) |
