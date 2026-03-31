#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <SDL3/SDL.h>
#include <string>

// Estrutura que armazena os dados do histograma e suas analises
struct HistogramData {
    int bins[256] = {0};      // Frequencia de cada nivel de cinza (0-255)
    int totalPixels = 0;      // Total de pixels na imagem
    double mean = 0.0;        // Media de intensidade
    double stddev = 0.0;      // Desvio padrao
    std::string brightness;   // "clara", "media" ou "escura"
    std::string contrast;     // "alto", "medio" ou "baixo"
};

// Calcula o histograma de uma imagem em escala de cinza.
// Tambem calcula media, desvio padrao, e classifica brilho/contraste.
HistogramData calculateHistogram(SDL_Surface* surface);

// Aplica equalizacao de histograma em uma imagem em escala de cinza.
// Retorna uma nova surface com o histograma equalizado.
SDL_Surface* equalizeHistogram(SDL_Surface* surface);

// Renderiza o histograma na janela secundaria usando o renderer fornecido.
// x, y: posicao do canto superior esquerdo do histograma
// w, h: largura e altura da area de desenho do histograma
void renderHistogram(SDL_Renderer* renderer, const HistogramData& data,
                     float x, float y, float w, float h);

#endif // HISTOGRAM_H
