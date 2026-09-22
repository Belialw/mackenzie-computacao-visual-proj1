// Copyright (c) 2026 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Projeto 1 - Computação Visual (UPM FCI CC)
// Integrantes: ver README.md
//------------------------------------------------------------------------------

#include "histogram.h"
#include "log.h"

// Cor dos rótulos do eixo de intensidades do gráfico.
static const SDL_Color AXIS_LABEL_COLOR = { 150, 150, 160, 255 };

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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Histogram_draw(const Histogram *histogram, SDL_Renderer *renderer, const SDL_FRect *area, const TextRenderer *text)
{
  if (!histogram || !renderer || !area)
    return;

  // Fundo da área do gráfico, mais escuro que o da janela, para delimitar onde
  // o histograma começa e termina mesmo nos níveis sem nenhum pixel.
  SDL_SetRenderDrawColor(renderer, 14, 14, 18, 255);
  SDL_RenderFillRect(renderer, area);

  if (histogram->max_count > 0)
  {
    SDL_SetRenderDrawColor(renderer, 118, 170, 240, 255);

    for (int level = 0; level < HISTOGRAM_LEVELS; ++level)
    {
      if (histogram->counts[level] <= 0)
        continue;

      const float ratio = (float)histogram->counts[level] / (float)histogram->max_count;
      const float bar_height = ratio * area->h;

      // Cada nível ocupa uma coluna de um pixel, e a barra cresce de baixo
      // para cima a partir da base da área.
      const SDL_FRect bar = {
        area->x + (float)level,
        area->y + area->h - bar_height,
        1.0f,
        bar_height
      };

      SDL_RenderFillRect(renderer, &bar);
    }
  }

  SDL_SetRenderDrawColor(renderer, 70, 70, 80, 255);
  SDL_RenderRect(renderer, area);

  // Rótulos do eixo das intensidades, alinhados às posições que representam.
  if (text)
  {
    const float label_y = area->y + area->h + 4.0f;

    Text_draw(text, renderer, area->x - 3.0f, label_y, AXIS_LABEL_COLOR, "0");
    Text_draw(text, renderer, area->x + 128.0f - 11.0f, label_y, AXIS_LABEL_COLOR, "128");
    Text_draw(text, renderer, area->x + 255.0f - 22.0f, label_y, AXIS_LABEL_COLOR, "255");
  }
}
