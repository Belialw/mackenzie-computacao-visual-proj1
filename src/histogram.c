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
// Limiares de classificação. O enunciado não define valores, então estes foram
// escolhidos pelo grupo e a justificativa está no README.md.
//
// Brilho: a faixa de intensidades 0-255 dividida em três partes iguais.
//
// Contraste: uma imagem que usa toda a faixa tonal de maneira uniforme tem
// desvio padrão de raiz((256^2 - 1) / 12), cerca de 73,9. O limiar de contraste
// alto fica logo abaixo desse valor de referência, e o de contraste baixo no
// ponto em que a imagem se concentra em uma faixa estreita e parece lavada.
//------------------------------------------------------------------------------
static const float DARK_MAX_MEAN = 85.0f;
static const float LIGHT_MIN_MEAN = 170.0f;
static const float LOW_MAX_STDDEV = 40.0f;
static const float HIGH_MIN_STDDEV = 70.0f;

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
  histogram->mean = 0.0f;
  histogram->stddev = 0.0f;

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

  if (histogram->total_pixels > 0)
  {
    // Média das intensidades, ponderada pela quantidade de pixels de cada
    // nível. A acumulação é feita em double: uma imagem grande soma valores
    // altos o bastante para que float perca precisão ao final.
    double weighted_sum = 0.0;
    for (int level = 0; level < HISTOGRAM_LEVELS; ++level)
      weighted_sum += (double)level * histogram->counts[level];

    const double mean = weighted_sum / histogram->total_pixels;

    // Desvio padrão das intensidades em relação à média.
    double variance_sum = 0.0;
    for (int level = 0; level < HISTOGRAM_LEVELS; ++level)
    {
      const double difference = (double)level - mean;
      variance_sum += histogram->counts[level] * difference * difference;
    }

    histogram->mean = (float)mean;
    histogram->stddev = (float)SDL_sqrt(variance_sum / histogram->total_pixels);
  }

  LOG_DEBUG("\t%d pixels distribuídos em %d níveis; nível mais frequente tem %d pixels",
    histogram->total_pixels, HISTOGRAM_LEVELS, histogram->max_count);
  LOG_DEBUG("	média = %.2f, desvio padrão = %.2f", histogram->mean, histogram->stddev);

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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const char *Histogram_brightness_label(const Histogram *histogram)
{
  if (!histogram || histogram->total_pixels <= 0)
    return "indisponível";

  if (histogram->mean < DARK_MAX_MEAN)
    return "escura";

  if (histogram->mean >= LIGHT_MIN_MEAN)
    return "clara";

  return "média";
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const char *Histogram_contrast_label(const Histogram *histogram)
{
  if (!histogram || histogram->total_pixels <= 0)
    return "indisponível";

  if (histogram->stddev < LOW_MAX_STDDEV)
    return "baixo";

  if (histogram->stddev >= HIGH_MIN_STDDEV)
    return "alto";

  return "médio";
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Histogram_equalization_mapping(const Histogram *histogram, Uint8 *mapping)
{
  LOG_DEBUG(">>> Histogram_equalization_mapping()");

  if (!histogram || !mapping)
  {
    LOG_ERROR("Histograma ou tabela de mapeamento inválidos.");
    LOG_DEBUG("<<< Histogram_equalization_mapping()");
    return false;
  }

  if (histogram->total_pixels <= 0)
  {
    LOG_ERROR("Histograma vazio: não há o que equalizar.");
    LOG_DEBUG("<<< Histogram_equalization_mapping()");
    return false;
  }

  // Soma acumulada das contagens e o primeiro valor acumulado não nulo, que
  // corresponde ao nível mais escuro presente na imagem.
  int cumulative[HISTOGRAM_LEVELS] = { 0 };
  int running_total = 0;
  int cdf_min = 0;

  for (int level = 0; level < HISTOGRAM_LEVELS; ++level)
  {
    running_total += histogram->counts[level];
    cumulative[level] = running_total;

    if (cdf_min == 0 && running_total > 0)
      cdf_min = running_total;
  }

  const int denominator = histogram->total_pixels - cdf_min;

  // Todos os pixels no mesmo nível: não existe faixa a espalhar, e qualquer
  // normalização dividiria por zero. A identidade mantém a imagem como está.
  if (denominator <= 0)
  {
    LOG_DEBUG("\tImagem de intensidade única; mapeamento identidade.");

    for (int level = 0; level < HISTOGRAM_LEVELS; ++level)
      mapping[level] = (Uint8)level;

    LOG_DEBUG("<<< Histogram_equalization_mapping()");
    return true;
  }

  for (int level = 0; level < HISTOGRAM_LEVELS; ++level)
  {
    const double normalized = (double)(cumulative[level] - cdf_min) / (double)denominator;
    const double scaled = normalized * (HISTOGRAM_LEVELS - 1);

    mapping[level] = (Uint8)SDL_round(scaled < 0.0 ? 0.0 : scaled);
  }

  LOG_DEBUG("\tcdf_min = %d, denominador = %d, mapeamento: 0 -> %u, 128 -> %u, 255 -> %u",
    cdf_min, denominator, mapping[0], mapping[128], mapping[HISTOGRAM_LEVELS - 1]);

  LOG_DEBUG("<<< Histogram_equalization_mapping()");
  return true;
}
