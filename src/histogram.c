// Copyright (c) 2026 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Projeto 1 - Computação Visual (UPM FCI CC)
// Integrantes: ver README.md
//------------------------------------------------------------------------------

#include "histogram.h"
#include "log.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Histogram_compute(Histogram *histogram, SDL_Surface *surface)
{
  LOG_DEBUG(">>> Histogram_compute()");

  if (!histogram)
  {
    LOG_ERROR("Histograma inválido (histogram == NULL).");
    LOG_DEBUG("<<< Histogram_compute()");
    return false;
  }

  if (!surface || !surface->pixels)
  {
    LOG_ERROR("Superfície inválida (surface == NULL ou sem pixels).");
    LOG_DEBUG("<<< Histogram_compute()");
    return false;
  }

  SDL_zeroa(histogram->counts);
  histogram->max_count = 0;
  histogram->total_pixels = 0;

  const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(surface->format);

  SDL_LockSurface(surface);

  for (int row = 0; row < surface->h; ++row)
  {
    // A varredura usa o pitch em vez de supor que cada linha ocupe w * 4
    // bytes: a SDL pode alinhar as linhas e inserir bytes de preenchimento.
    const Uint32 *pixels = (const Uint32 *)((const Uint8 *)surface->pixels + (size_t)row * surface->pitch);

    for (int col = 0; col < surface->w; ++col)
    {
      Uint8 r = 0;
      Uint8 g = 0;
      Uint8 b = 0;
      SDL_GetRGB(pixels[col], format, NULL, &r, &g, &b);

      // A imagem já está em escala de cinza, então r, g e b são iguais e
      // qualquer um deles representa a intensidade do pixel.
      ++histogram->counts[r];
    }
  }

  SDL_UnlockSurface(surface);

  for (int level = 0; level < HISTOGRAM_LEVELS; ++level)
  {
    histogram->total_pixels += histogram->counts[level];

    if (histogram->counts[level] > histogram->max_count)
      histogram->max_count = histogram->counts[level];
  }

  LOG_DEBUG("\t%d pixels distribuídos em %d níveis; nível mais frequente tem %d pixels",
    histogram->total_pixels, HISTOGRAM_LEVELS, histogram->max_count);

  LOG_DEBUG("<<< Histogram_compute()");
  return true;
}
