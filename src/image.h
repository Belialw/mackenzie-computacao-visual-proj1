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

  // Resultado do processamento atual, como a equalização. Fica separado de
  // `surface` para que a imagem em escala de cinza continue intacta em
  // memória: é ela que MyImage_restore_texture() devolve, sem reler o arquivo.
  SDL_Surface *processed;

  // Aponta para a surface que deu origem à textura em exibição: `surface` ou
  // `processed`. É ela que a gravação em disco usa, de modo que o arquivo
  // salvo corresponda ao que está na janela principal.
  SDL_Surface *displayed;
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


/**
 * Indica se a imagem já está em escala de cinza, isto é, se R, G e B têm o
 * mesmo valor em todos os pixels. O canal alpha não é considerado.
 */
bool MyImage_is_grayscale(const MyImage *image);

/**
 * Converte a surface da imagem para escala de cinza, no lugar, usando a
 * fórmula definida no enunciado do projeto, e atualiza a textura exibida. O
 * canal alpha de cada pixel é preservado.
 *
 * A conversão acontece sobre `image->surface`, de modo que é a imagem em escala
 * de cinza que passa a ser a base das operações seguintes, e é para ela que
 * MyImage_restore_texture() devolve a exibição.
 */
bool MyImage_to_grayscale(MyImage *image, SDL_Renderer *renderer);


/**
 * Aplica a tabela de mapeamento de intensidades sobre a imagem em escala de
 * cinza, guardando o resultado em `image->processed` e exibindo-o.
 *
 * A imagem original em escala de cinza (`image->surface`) não é alterada, de
 * modo que MyImage_restore_texture() volte a exibi-la sem recarregar o arquivo
 * do disco.
 *
 * O canal alpha de cada pixel é preservado.
 */
bool MyImage_apply_mapping(MyImage *image, SDL_Renderer *renderer, const Uint8 *mapping);


/**
 * Salva em `filename`, no formato PNG, a imagem que está em exibição — isto é,
 * a surface que originou a textura atual. Um arquivo existente é sobrescrito.
 *
 * `existed` recebe true quando o arquivo já estava no disco antes da gravação,
 * para que a mensagem no terminal possa distinguir criação de sobrescrita. A
 * verificação precisa acontecer antes de gravar, já que depois não há como
 * saber se o arquivo é novo.
 */
bool MyImage_save_png(const MyImage *image, const char *filename, bool *existed);

#endif // IMAGE_H
