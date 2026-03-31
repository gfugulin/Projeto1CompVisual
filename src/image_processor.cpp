#include "image_processor.h"
#include <cstdio>

SDL_Surface* loadImage(const std::string& path) {
    // Tenta carregar a imagem usando SDL_image
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        SDL_Log("Erro ao carregar imagem '%s': %s", path.c_str(), SDL_GetError());
        return nullptr;
    }

    // Converte para formato RGBA32 para facilitar manipulacao de pixels
    SDL_Surface* converted = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surface);

    if (!converted) {
        SDL_Log("Erro ao converter formato da imagem: %s", SDL_GetError());
        return nullptr;
    }

    return converted;
}

bool isGrayscale(SDL_Surface* surface) {
    if (!surface) return false;

    int width = surface->w;
    int height = surface->h;

    // Bloqueia a surface para acessar pixels diretamente
    if (!SDL_LockSurface(surface)) {
        SDL_Log("Erro ao bloquear surface: %s", SDL_GetError());
        return false;
    }

    Uint8* pixels = (Uint8*)surface->pixels;
    int pitch = surface->pitch;
    int bpp = SDL_BYTESPERPIXEL(surface->format);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Uint8* pixel = pixels + y * pitch + x * bpp;

            Uint8 r, g, b;
            // Para RGBA32: os canais estao em ordem R, G, B, A
            r = pixel[0];
            g = pixel[1];
            b = pixel[2];
            // a = pixel[3]; // nao precisamos do alpha aqui

            // Se R != G ou R != B, a imagem e colorida
            if (r != g || r != b) {
                SDL_UnlockSurface(surface);
                return false;
            }
        }
    }

    SDL_UnlockSurface(surface);
    return true;
}

SDL_Surface* convertToGrayscale(SDL_Surface* surface) {
    if (!surface) return nullptr;

    int width = surface->w;
    int height = surface->h;

    // Cria uma nova surface para a imagem em escala de cinza
    SDL_Surface* graySurface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    if (!graySurface) {
        SDL_Log("Erro ao criar surface para escala de cinza: %s", SDL_GetError());
        return nullptr;
    }

    if (!SDL_LockSurface(surface)) {
        SDL_DestroySurface(graySurface);
        return nullptr;
    }
    if (!SDL_LockSurface(graySurface)) {
        SDL_UnlockSurface(surface);
        SDL_DestroySurface(graySurface);
        return nullptr;
    }

    Uint8* srcPixels = (Uint8*)surface->pixels;
    Uint8* dstPixels = (Uint8*)graySurface->pixels;
    int srcPitch = surface->pitch;
    int dstPitch = graySurface->pitch;
    int bpp = SDL_BYTESPERPIXEL(surface->format);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Uint8* srcPixel = srcPixels + y * srcPitch + x * bpp;
            Uint8* dstPixel = dstPixels + y * dstPitch + x * bpp;

            Uint8 r = srcPixel[0];
            Uint8 g = srcPixel[1];
            Uint8 b = srcPixel[2];
            Uint8 a = srcPixel[3];

            // Formula: Y = 0.2125 * R + 0.7154 * G + 0.0721 * B
            Uint8 gray = (Uint8)(0.2125 * r + 0.7154 * g + 0.0721 * b);

            dstPixel[0] = gray; // R
            dstPixel[1] = gray; // G
            dstPixel[2] = gray; // B
            dstPixel[3] = a;    // A (preserva transparencia)
        }
    }

    SDL_UnlockSurface(graySurface);
    SDL_UnlockSurface(surface);

    return graySurface;
}

bool saveImage(SDL_Surface* surface, const std::string& path) {
    if (!surface) {
        SDL_Log("Erro: surface nula ao tentar salvar imagem.");
        return false;
    }

    if (!IMG_SavePNG(surface, path.c_str())) {
        SDL_Log("Erro ao salvar imagem em '%s': %s", path.c_str(), SDL_GetError());
        return false;
    }

    SDL_Log("Imagem salva com sucesso em '%s'", path.c_str());
    return true;
}
