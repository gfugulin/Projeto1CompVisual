#ifndef GUI_H
#define GUI_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

// Estados possiveis do botao
enum class ButtonState {
    NORMAL,   // Estado neutro (azul)
    HOVER,    // Mouse sobre o botao (azul claro)
    PRESSED   // Botao clicado (azul escuro)
};

// Estrutura que representa o botao de equalizacao
struct Button {
    SDL_FRect rect;           // Posicao e dimensoes do botao
    ButtonState state;        // Estado atual
    std::string text;         // Texto exibido no botao
    bool equalized;           // Se a imagem esta equalizada ou nao
};

// Dimensoes da janela secundaria
constexpr int SECONDARY_WINDOW_WIDTH = 400;
constexpr int SECONDARY_WINDOW_HEIGHT = 600;

// Cria a janela principal centralizada no monitor, adaptada ao tamanho da imagem.
// Retorna nullptr em caso de erro.
SDL_Window* createMainWindow(int imageWidth, int imageHeight);

// Cria a janela secundaria (filha da janela principal) posicionada ao lado.
// Retorna nullptr em caso de erro.
SDL_Window* createSecondaryWindow(SDL_Window* parentWindow);

// Inicializa o botao com posicao e dimensoes padrao.
Button createButton(float x, float y, float w, float h);

// Renderiza o botao no renderer com base no seu estado atual.
void renderButton(SDL_Renderer* renderer, TTF_Font* font, const Button& button);

// Renderiza texto em uma posicao especifica usando SDL_ttf.
void renderText(SDL_Renderer* renderer, TTF_Font* font,
                const std::string& text, float x, float y,
                SDL_Color color);

// Verifica se um ponto (x, y) esta dentro de um retangulo.
bool isPointInRect(float px, float py, const SDL_FRect& rect);

#endif // GUI_H
