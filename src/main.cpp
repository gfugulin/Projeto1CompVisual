/*
 * Projeto 1 - Processamento de Imagens
 * Computacao Visual - Universidade Presbiteriana Mackenzie
 * Prof. Andre Kishimoto
 *
 * Integrantes:
 *   Gustavo Fugulin Soares da Silva - 10418552
 *   Otto Martins Mota - 10418170
 *   Renan Garrido - 10417093
 *   Rodrigo Roveratti Guerrero - 10417090
 *
 * Descricao: Software de processamento de imagens usando SDL3.
 * Funcionalidades: carregamento de imagem, conversao para escala de cinza,
 * analise e exibicao de histograma, equalizacao de histograma,
 * interface grafica com duas janelas, e salvamento de imagem.
 */

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "image_processor.h"
#include "histogram.h"
#include "gui.h"

#include <cstdio>
#include <string>

// Funcao auxiliar para renderizar a janela secundaria completa
void renderSecondaryWindow(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* fontSmall,
                           const HistogramData& histData, const Button& button) {
    // Limpa a janela com fundo escuro
    SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
    SDL_RenderClear(renderer);

    // Titulo
    SDL_Color titleColor = {255, 255, 255, 255};
    renderText(renderer, font, "Histograma", SECONDARY_WINDOW_WIDTH / 2.0f, 25.0f, titleColor);

    // Renderiza o histograma
    float histX = 20.0f;
    float histY = 50.0f;
    float histW = SECONDARY_WINDOW_WIDTH - 40.0f;
    float histH = 250.0f;
    renderHistogram(renderer, histData, histX, histY, histW, histH);

    // Informacoes de analise
    SDL_Color infoColor = {180, 180, 180, 255};
    SDL_Color valueColor = {100, 200, 255, 255};

    float infoY = histY + histH + 30.0f;
    float lineHeight = 30.0f;

    // Media de intensidade
    char meanText[128];
    snprintf(meanText, sizeof(meanText), "Media de intensidade: %.1f", histData.mean);
    renderText(renderer, fontSmall, meanText, SECONDARY_WINDOW_WIDTH / 2.0f, infoY, infoColor);

    // Classificacao do brilho
    infoY += lineHeight;
    std::string brightnessText = "Classificacao: " + histData.brightness;
    renderText(renderer, fontSmall, brightnessText, SECONDARY_WINDOW_WIDTH / 2.0f, infoY, valueColor);

    // Desvio padrao
    infoY += lineHeight * 1.5f;
    char stddevText[128];
    snprintf(stddevText, sizeof(stddevText), "Desvio padrao: %.1f", histData.stddev);
    renderText(renderer, fontSmall, stddevText, SECONDARY_WINDOW_WIDTH / 2.0f, infoY, infoColor);

    // Classificacao do contraste
    infoY += lineHeight;
    std::string contrastText = "Contraste: " + histData.contrast;
    renderText(renderer, fontSmall, contrastText, SECONDARY_WINDOW_WIDTH / 2.0f, infoY, valueColor);

    // Renderiza o botao
    renderButton(renderer, font, button);

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    SDL_SetMainReady();
    fprintf(stderr, "DEBUG: Programa iniciado.\n");
    fflush(stderr);
    // Verifica argumento de linha de comando
    if (argc < 2) {
        SDL_Log("Uso: %s <caminho_da_imagem>", argv[0]);
        SDL_Log("Formatos suportados: PNG, JPG, BMP");
        SDL_Log("Exemplo: imgproc.exe teste.jpg");
        return 1;
    }

    // Reconstroi o caminho completo caso ele tenha espacos
    // (PowerShell pode dividir caminhos com espaco em multiplos argumentos)
    std::string imagePath = argv[1];
    for (int i = 2; i < argc; i++) {
        imagePath += " ";
        imagePath += argv[i];
    }

    SDL_Log("DEBUG: Caminho da imagem: '%s'", imagePath.c_str());

    // Inicializa SDL3
    SDL_Log("DEBUG: Inicializando SDL3...");
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Erro ao inicializar SDL: %s", SDL_GetError());
        return 1;
    }
    SDL_Log("DEBUG: SDL3 inicializado com sucesso.");

    // Inicializa SDL_ttf
    SDL_Log("DEBUG: Inicializando SDL_ttf...");
    if (!TTF_Init()) {
        SDL_Log("Erro ao inicializar SDL_ttf: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_Log("DEBUG: SDL_ttf inicializado com sucesso.");

    // ===== CARREGAMENTO DA IMAGEM =====
    SDL_Log("Carregando imagem: %s", imagePath.c_str());
    SDL_Surface* originalSurface = loadImage(imagePath);
    if (!originalSurface) {
        SDL_Log("Erro: Nao foi possivel carregar a imagem '%s'.", imagePath.c_str());
        SDL_Log("Verifique se o arquivo existe e se e um formato valido (PNG, JPG, BMP).");
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_Log("Imagem carregada: %dx%d pixels", originalSurface->w, originalSurface->h);

    // ===== CONVERSAO PARA ESCALA DE CINZA =====
    SDL_Surface* graySurface = nullptr;
    if (isGrayscale(originalSurface)) {
        SDL_Log("A imagem ja esta em escala de cinza.");
        graySurface = SDL_DuplicateSurface(originalSurface);
    } else {
        SDL_Log("Imagem colorida detectada. Convertendo para escala de cinza...");
        graySurface = convertToGrayscale(originalSurface);
    }

    if (!graySurface) {
        SDL_Log("Erro ao processar a imagem para escala de cinza.");
        SDL_DestroySurface(originalSurface);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_DestroySurface(originalSurface); // Nao precisamos mais da original colorida

    // A imagem em escala de cinza original (para poder reverter apos equalizacao)
    SDL_Surface* originalGray = SDL_DuplicateSurface(graySurface);

    // Imagem atualmente exibida (pode ser a cinza original ou a equalizada)
    SDL_Surface* currentSurface = graySurface;

    // ===== CALCULO DO HISTOGRAMA =====
    HistogramData histData = calculateHistogram(currentSurface);
    SDL_Log("Histograma calculado - Media: %.1f (%s), Desvio: %.1f (%s)",
            histData.mean, histData.brightness.c_str(),
            histData.stddev, histData.contrast.c_str());

    // ===== CRIACAO DAS JANELAS =====
    SDL_Window* mainWindow = createMainWindow(currentSurface->w, currentSurface->h);
    if (!mainWindow) {
        SDL_DestroySurface(graySurface);
        SDL_DestroySurface(originalGray);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Window* secondaryWindow = createSecondaryWindow(mainWindow);
    if (!secondaryWindow) {
        SDL_DestroyWindow(mainWindow);
        SDL_DestroySurface(graySurface);
        SDL_DestroySurface(originalGray);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Cria renderers para cada janela
    SDL_Renderer* mainRenderer = SDL_CreateRenderer(mainWindow, nullptr);
    SDL_Renderer* secondaryRenderer = SDL_CreateRenderer(secondaryWindow, nullptr);

    if (!mainRenderer || !secondaryRenderer) {
        SDL_Log("Erro ao criar renderers: %s", SDL_GetError());
        if (mainRenderer) SDL_DestroyRenderer(mainRenderer);
        if (secondaryRenderer) SDL_DestroyRenderer(secondaryRenderer);
        SDL_DestroyWindow(secondaryWindow);
        SDL_DestroyWindow(mainWindow);
        SDL_DestroySurface(graySurface);
        SDL_DestroySurface(originalGray);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // ===== CARREGA FONTE =====
    TTF_Font* font = TTF_OpenFont("assets/Roboto-Regular.ttf", 20.0f);
    TTF_Font* fontSmall = TTF_OpenFont("assets/Roboto-Regular.ttf", 16.0f);
    if (!font || !fontSmall) {
        SDL_Log("Erro ao carregar fonte: %s", SDL_GetError());
        SDL_Log("Certifique-se de que o arquivo 'assets/Roboto-Regular.ttf' existe.");
        // Continua sem fonte (textos nao serao exibidos)
    }

    // ===== CRIA BOTAO =====
    float btnW = 200.0f;
    float btnH = 50.0f;
    float btnX = (SECONDARY_WINDOW_WIDTH - btnW) / 2.0f;
    float btnY = SECONDARY_WINDOW_HEIGHT - btnH - 30.0f;
    Button eqButton = createButton(btnX, btnY, btnW, btnH);

    // Surface equalizada (sera criada quando necessario)
    SDL_Surface* equalizedSurface = nullptr;

    // ===== LOOP PRINCIPAL =====
    bool running = true;
    bool needsRedraw = true;
    bool needsSecondaryRedraw = true;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;

                case SDL_EVENT_KEY_DOWN:
                    if (event.key.key == SDLK_ESCAPE) {
                        running = false;
                    } else if (event.key.key == SDLK_S) {
                        // Salva a imagem atual como output_image.png
                        saveImage(currentSurface, "output_image.png");
                    }
                    break;

                case SDL_EVENT_MOUSE_MOTION: {
                    // Verifica se o mouse esta sobre o botao (na janela secundaria)
                    SDL_WindowID secWinID = SDL_GetWindowID(secondaryWindow);
                    if (event.motion.windowID == secWinID) {
                        float mx = event.motion.x;
                        float my = event.motion.y;
                        ButtonState newState = isPointInRect(mx, my, eqButton.rect)
                            ? ButtonState::HOVER : ButtonState::NORMAL;
                        if (newState != eqButton.state) {
                            eqButton.state = newState;
                            needsSecondaryRedraw = true;
                        }
                    }
                    break;
                }

                case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                    SDL_WindowID secWinID = SDL_GetWindowID(secondaryWindow);
                    if (event.button.windowID == secWinID) {
                        float mx = event.button.x;
                        float my = event.button.y;
                        if (isPointInRect(mx, my, eqButton.rect)) {
                            eqButton.state = ButtonState::PRESSED;
                            needsSecondaryRedraw = true;
                        }
                    }
                    break;
                }

                case SDL_EVENT_MOUSE_BUTTON_UP: {
                    SDL_WindowID secWinID = SDL_GetWindowID(secondaryWindow);
                    if (event.button.windowID == secWinID) {
                        float mx = event.button.x;
                        float my = event.button.y;
                        if (isPointInRect(mx, my, eqButton.rect)) {
                            // Clique no botao - toggle equalizacao
                            if (!eqButton.equalized) {
                                // Equaliza a imagem
                                if (!equalizedSurface) {
                                    equalizedSurface = equalizeHistogram(graySurface);
                                }
                                if (equalizedSurface) {
                                    currentSurface = equalizedSurface;
                                    eqButton.equalized = true;
                                    eqButton.text = "Ver original";
                                    SDL_Log("Histograma equalizado.");
                                }
                            } else {
                                // Reverte para a original
                                currentSurface = graySurface;
                                eqButton.equalized = false;
                                eqButton.text = "Equalizar";
                                SDL_Log("Voltando para imagem original em escala de cinza.");
                            }

                            // Recalcula o histograma
                            histData = calculateHistogram(currentSurface);
                            needsRedraw = true;
                            needsSecondaryRedraw = true;

                            eqButton.state = isPointInRect(mx, my, eqButton.rect)
                                ? ButtonState::HOVER : ButtonState::NORMAL;
                        }
                    }
                    break;
                }

                case SDL_EVENT_WINDOW_EXPOSED:
                    needsRedraw = true;
                    needsSecondaryRedraw = true;
                    break;
            }
        }

        // ===== RENDERIZA JANELA PRINCIPAL =====
        if (needsRedraw) {
            SDL_SetRenderDrawColor(mainRenderer, 0, 0, 0, 255);
            SDL_RenderClear(mainRenderer);

            // Cria textura da imagem atual e renderiza
            SDL_Texture* imgTexture = SDL_CreateTextureFromSurface(mainRenderer, currentSurface);
            if (imgTexture) {
                SDL_RenderTexture(mainRenderer, imgTexture, nullptr, nullptr);
                SDL_DestroyTexture(imgTexture);
            }

            SDL_RenderPresent(mainRenderer);
            needsRedraw = false;
        }

        // ===== RENDERIZA JANELA SECUNDARIA =====
        if (needsSecondaryRedraw) {
            renderSecondaryWindow(secondaryRenderer, font, fontSmall, histData, eqButton);
            needsSecondaryRedraw = false;
        }

        // Pequeno delay para nao sobrecarregar a CPU
        SDL_Delay(16); // ~60 FPS
    }

    // ===== CLEANUP =====
    SDL_Log("Encerrando programa...");

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

    return 0;
}
