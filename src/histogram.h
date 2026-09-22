// Copyright (c) 2026 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Projeto 1 - Computação Visual (UPM FCI CC)
// Integrantes: ver README.md
//
// Cálculo do histograma da imagem em escala de cinza.
//------------------------------------------------------------------------------

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <stdbool.h>
#include <SDL3/SDL.h>

enum
{
  // Um nível para cada intensidade possível em 8 bits.
  HISTOGRAM_LEVELS = 256,
};

/**
 * Distribuição das intensidades da imagem.
 *
 * `counts[i]` é a quantidade de pixels com intensidade `i`. `max_count` é a
 * maior dessas quantidades e serve de referência para desenhar o gráfico em
 * proporção à altura disponível. `total_pixels` é a soma de todas elas.
 */
typedef struct Histogram Histogram;
struct Histogram
{
  int counts[HISTOGRAM_LEVELS];
  int max_count;
  int total_pixels;
};

/**
 * Calcula o histograma de `surface`, que deve estar em escala de cinza e no
 * formato RGBA32. Como a imagem já está em escala de cinza, os três canais de
 * cor têm o mesmo valor e a intensidade de cada pixel é lida de um deles.
 *
 * Retorna false, sem alterar `histogram`, caso os parâmetros sejam inválidos.
 */
bool Histogram_compute(Histogram *histogram, SDL_Surface *surface);

#endif // HISTOGRAM_H
