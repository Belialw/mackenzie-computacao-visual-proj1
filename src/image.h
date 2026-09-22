// Copyright (c) 2026 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Projeto 1 - Computação Visual (UPM FCI CC)
// Integrantes: ver README.md
//
// Carregamento e representação da imagem em memória.
//------------------------------------------------------------------------------

#ifndef IMAGE_H
#define IMAGE_H

#include <stdbool.h>
#include <SDL3/SDL.h>

/**
 * Imagem carregada: a surface guarda os pixels (sempre em RGBA32) e serve de
 * base para o processamento; a textura é o que vai para a tela; o rect guarda
 * as dimensões da textura.
 */
typedef struct MyImage MyImage;
struct MyImage
{
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_FRect rect;
};

/**
 * Libera a textura e a surface de `image` e zera o rect.
 */
void MyImage_destroy(MyImage *image);

/**
 * Recria a textura de `image` a partir de `surface`, que pode ser uma surface
 * processada e diferente de `image->surface`. Retorna false em caso de erro.
 */
bool MyImage_update_texture_with_surface(MyImage* image, SDL_Renderer *renderer, SDL_Surface *surface);

/**
 * Recria a textura de `image` a partir da própria `image->surface`, ou seja,
 * desfaz o processamento exibido sem recarregar o arquivo do disco.
 */
bool MyImage_restore_texture(MyImage* image, SDL_Renderer *renderer);

/**
 * Carrega a imagem indicada no parâmetro `filename` e a converte para o formato
 * RGBA32, eliminando dependência do formato original da imagem. A imagem
 * carregada é armazenada em output_image.
 * Caso ocorra algum erro no processo, a função retorna false.
 */
bool load_rgba32(const char *filename, SDL_Renderer *renderer, MyImage *output_image);

#endif // IMAGE_H
