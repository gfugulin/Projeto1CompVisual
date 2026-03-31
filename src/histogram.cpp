#include "histogram.h"
#include <cmath>
#include <algorithm>

HistogramData calculateHistogram(SDL_Surface* surface) {
    HistogramData data;

    if (!surface) return data;

    int width = surface->w;
    int height = surface->h;
    data.totalPixels = width * height;

    if (!SDL_LockSurface(surface)) {
        SDL_Log("Erro ao bloquear surface para calcular histograma: %s", SDL_GetError());
        return data;
    }

    Uint8* pixels = (Uint8*)surface->pixels;
    int pitch = surface->pitch;
    int bpp = SDL_BYTESPERPIXEL(surface->format);

    // Conta a frequencia de cada nivel de cinza
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Uint8* pixel = pixels + y * pitch + x * bpp;
            Uint8 gray = pixel[0]; // Em escala de cinza, R == G == B
            data.bins[gray]++;
        }
    }

    SDL_UnlockSurface(surface);

    // Calcula a media de intensidade
    double sum = 0.0;
    for (int i = 0; i < 256; i++) {
        sum += (double)i * data.bins[i];
    }
    data.mean = sum / data.totalPixels;

    // Calcula o desvio padrao
    double variance = 0.0;
    for (int i = 0; i < 256; i++) {
        double diff = (double)i - data.mean;
        variance += diff * diff * data.bins[i];
    }
    variance /= data.totalPixels;
    data.stddev = std::sqrt(variance);

    // Classifica o brilho com base na media
    // 0-85: escura, 85-170: media, 170-255: clara
    if (data.mean < 85.0) {
        data.brightness = "escura";
    } else if (data.mean < 170.0) {
        data.brightness = "media";
    } else {
        data.brightness = "clara";
    }

    // Classifica o contraste com base no desvio padrao
    // 0-40: baixo, 40-80: medio, 80+: alto
    if (data.stddev < 40.0) {
        data.contrast = "baixo";
    } else if (data.stddev < 80.0) {
        data.contrast = "medio";
    } else {
        data.contrast = "alto";
    }

    return data;
}

SDL_Surface* equalizeHistogram(SDL_Surface* surface) {
    if (!surface) return nullptr;

    int width = surface->w;
    int height = surface->h;
    int totalPixels = width * height;

    if (totalPixels <= 0) return nullptr;

    // Primeiro, calcula o histograma
    int histogram[256] = {0};

    if (!SDL_LockSurface(surface)) {
        SDL_Log("Erro ao bloquear surface para equalizacao: %s", SDL_GetError());
        return nullptr;
    }

    Uint8* pixels = (Uint8*)surface->pixels;
    int pitch = surface->pitch;
    int bpp = SDL_BYTESPERPIXEL(surface->format);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Uint8* pixel = pixels + y * pitch + x * bpp;
            histogram[pixel[0]]++;
        }
    }

    // Calcula o CDF (Cumulative Distribution Function)
    int cdf[256] = {0};
    cdf[0] = histogram[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i - 1] + histogram[i];
    }

    // Encontra o menor valor nao-zero do CDF
    int cdfMin = 0;
    for (int i = 0; i < 256; i++) {
        if (histogram[i] > 0) {
            cdfMin = cdf[i];
            break;
        }
    }

    // Calcula a tabela de mapeamento para equalizacao
    Uint8 lookupTable[256];
    int denominator = totalPixels - cdfMin;

    if (denominator <= 0) {
        for (int i = 0; i < 256; i++) {
            lookupTable[i] = (Uint8)i;
        }
    } else {
        for (int i = 0; i < 256; i++) {
            if (cdf[i] == 0) {
                lookupTable[i] = 0;
            } else {
                double val = ((double)(cdf[i] - cdfMin) / (double)denominator) * 255.0;
                lookupTable[i] = (Uint8)std::round(std::max(0.0, std::min(255.0, val)));
            }
        }
    }

    // Cria a nova surface equalizada
    SDL_Surface* equalized = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    if (!equalized) {
        SDL_UnlockSurface(surface);
        return nullptr;
    }

    if (!SDL_LockSurface(equalized)) {
        SDL_UnlockSurface(surface);
        SDL_DestroySurface(equalized);
        return nullptr;
    }

    Uint8* dstPixels = (Uint8*)equalized->pixels;
    int dstPitch = equalized->pitch;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Uint8* srcPixel = pixels + y * pitch + x * bpp;
            Uint8* dstPixel = dstPixels + y * dstPitch + x * bpp;

            Uint8 newGray = lookupTable[srcPixel[0]];
            dstPixel[0] = newGray; // R
            dstPixel[1] = newGray; // G
            dstPixel[2] = newGray; // B
            dstPixel[3] = srcPixel[3]; // A (preserva alpha)
        }
    }

    SDL_UnlockSurface(equalized);
    SDL_UnlockSurface(surface);

    return equalized;
}

void renderHistogram(SDL_Renderer* renderer, const HistogramData& data,
                     float x, float y, float w, float h) {
    // Encontra o valor maximo no histograma para normalizar
    int maxVal = 0;
    for (int i = 0; i < 256; i++) {
        if (data.bins[i] > maxVal) {
            maxVal = data.bins[i];
        }
    }

    if (maxVal == 0) return; // Histograma vazio

    // Desenha o fundo do histograma
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_FRect bgRect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &bgRect);

    // Desenha a borda
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderRect(renderer, &bgRect);

    // Largura de cada barra
    float barWidth = w / 256.0f;

    // Desenha as barras do histograma
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    for (int i = 0; i < 256; i++) {
        float barHeight = ((float)data.bins[i] / (float)maxVal) * h;
        SDL_FRect barRect = {
            x + i * barWidth,
            y + h - barHeight,
            barWidth + 0.5f, // Pequeno overlap para evitar gaps
            barHeight
        };
        SDL_RenderFillRect(renderer, &barRect);
    }
}
