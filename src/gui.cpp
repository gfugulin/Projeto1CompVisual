#include "gui.h"
#include <cstring>

SDL_Window* createMainWindow(int imageWidth, int imageHeight) {
    // Obtem informacoes do monitor principal para centralizar
    SDL_DisplayID displayID = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(displayID);

    // Limita o tamanho da janela ao tamanho da tela (com margem)
    int maxW = mode ? (int)(mode->w * 0.85f) : 1280;
    int maxH = mode ? (int)(mode->h * 0.85f) : 720;

    int winW = imageWidth < maxW ? imageWidth : maxW;
    int winH = imageHeight < maxH ? imageHeight : maxH;

    // Cria a janela principal
    SDL_Window* window = SDL_CreateWindow(
        "Processamento de Imagens - Projeto 1",
        winW, winH,
        0 // flags
    );

    if (!window) {
        SDL_Log("Erro ao criar janela principal: %s", SDL_GetError());
        return nullptr;
    }

    // Centraliza a janela no monitor principal, deslocada um pouco para a esquerda
    // para deixar espaco para a janela secundaria
    if (mode) {
        int posX = (mode->w - winW - SECONDARY_WINDOW_WIDTH - 10) / 2;
        int posY = (mode->h - winH) / 2;
        if (posX < 0) posX = 10;
        if (posY < 0) posY = 10;
        SDL_SetWindowPosition(window, posX, posY);
    }

    return window;
}

SDL_Window* createSecondaryWindow(SDL_Window* parentWindow) {
    if (!parentWindow) return nullptr;

    // Obtem a posicao e tamanho da janela principal
    int parentX, parentY, parentW, parentH;
    SDL_GetWindowPosition(parentWindow, &parentX, &parentY);
    SDL_GetWindowSize(parentWindow, &parentW, &parentH);

    // Cria a janela secundaria como filha da janela principal
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Histograma");
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, SECONDARY_WINDOW_WIDTH);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, SECONDARY_WINDOW_HEIGHT);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, parentX + parentW + 10);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, parentY);
    SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_PARENT_POINTER, parentWindow);

    SDL_Window* window = SDL_CreateWindowWithProperties(props);
    SDL_DestroyProperties(props);

    if (!window) {
        SDL_Log("Erro ao criar janela secundaria: %s", SDL_GetError());
        return nullptr;
    }

    return window;
}

Button createButton(float x, float y, float w, float h) {
    Button btn;
    btn.rect = {x, y, w, h};
    btn.state = ButtonState::NORMAL;
    btn.text = "Equalizar";
    btn.equalized = false;
    return btn;
}

void renderButton(SDL_Renderer* renderer, TTF_Font* font, const Button& button) {
    // Define as cores para cada estado
    SDL_Color bgColor;
    switch (button.state) {
        case ButtonState::HOVER:
            bgColor = {96, 165, 250, 255};   // Azul claro #60A5FA
            break;
        case ButtonState::PRESSED:
            bgColor = {29, 78, 216, 255};     // Azul escuro #1D4ED8
            break;
        case ButtonState::NORMAL:
        default:
            bgColor = {59, 130, 246, 255};    // Azul #3B82F6
            break;
    }

    // Desenha o fundo do botao
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, &button.rect);

    // Desenha a borda do botao (mais escura)
    SDL_SetRenderDrawColor(renderer, 30, 64, 175, 255); // #1E40AF
    SDL_RenderRect(renderer, &button.rect);

    // Renderiza o texto do botao centralizado
    SDL_Color textColor = {255, 255, 255, 255}; // Branco
    renderText(renderer, font, button.text,
               button.rect.x + button.rect.w / 2.0f,
               button.rect.y + button.rect.h / 2.0f,
               textColor);
}

void renderText(SDL_Renderer* renderer, TTF_Font* font,
                const std::string& text, float x, float y,
                SDL_Color color) {
    if (!font || text.empty()) return;

    // Cria surface com o texto
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), text.length(), color);
    if (!textSurface) {
        SDL_Log("Erro ao renderizar texto: %s", SDL_GetError());
        return;
    }

    // Cria textura a partir da surface
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        SDL_DestroySurface(textSurface);
        return;
    }

    // Centraliza o texto na posicao fornecida
    float textW = (float)textSurface->w;
    float textH = (float)textSurface->h;
    SDL_FRect dstRect = {x - textW / 2.0f, y - textH / 2.0f, textW, textH};

    SDL_RenderTexture(renderer, textTexture, nullptr, &dstRect);

    SDL_DestroyTexture(textTexture);
    SDL_DestroySurface(textSurface);
}

bool isPointInRect(float px, float py, const SDL_FRect& rect) {
    return (px >= rect.x && px <= rect.x + rect.w &&
            py >= rect.y && py <= rect.y + rect.h);
}
