#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>

// Carrega uma imagem a partir do caminho especificado.
// Retorna nullptr em caso de erro (arquivo nao encontrado, formato invalido, etc).
SDL_Surface* loadImage(const std::string& path);

// Verifica se uma surface esta em escala de cinza (R == G == B para todos os pixels).
bool isGrayscale(SDL_Surface* surface);

// Converte uma surface colorida para escala de cinza usando a formula:
// Y = 0.2125 * R + 0.7154 * G + 0.0721 * B
// Retorna uma nova surface em escala de cinza (a original nao e modificada).
SDL_Surface* convertToGrayscale(SDL_Surface* surface);

// Salva a surface como arquivo PNG no caminho especificado.
bool saveImage(SDL_Surface* surface, const std::string& path);

#endif // IMAGE_PROCESSOR_H
