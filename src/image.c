// Copyright (c) 2026 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Projeto 1 - Computação Visual (UPM FCI CC)
// Integrantes: ver README.md
//------------------------------------------------------------------------------

#include "image.h"
#include "log.h"

#include <SDL3_image/SDL_image.h>

//------------------------------------------------------------------------------
// Pesos da conversão para escala de cinza, conforme a fórmula definida no
// enunciado do projeto: Y = 0.2125*R + 0.7154*G + 0.0721*B.
// Os três somam exatamente 1.0, então o resultado nunca ultrapassa 255 e não
// precisa ser limitado.
//------------------------------------------------------------------------------
static const float GRAYSCALE_WEIGHT_R = 0.2125f;
static const float GRAYSCALE_WEIGHT_G = 0.7154f;
static const float GRAYSCALE_WEIGHT_B = 0.0721f;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyImage_destroy(MyImage *image)
{
  LOG_DEBUG(">>> MyImage_destroy()");

  if (!image)
  {
    LOG_ERROR("Imagem inválida (image == NULL).");
    LOG_DEBUG("<<< MyImage_destroy()");
    return;
  }

  if (image->texture)
  {
    LOG_DEBUG("\tDestruindo MyImage->texture...");
    SDL_DestroyTexture(image->texture);
    image->texture = NULL;
  }

  if (image->processed)
  {
    LOG_DEBUG("	Destruindo MyImage->processed...");
    SDL_DestroySurface(image->processed);
    image->processed = NULL;
  }

  if (image->surface)
  {
    LOG_DEBUG("\tDestruindo MyImage->surface...");
    SDL_DestroySurface(image->surface);
    image->surface = NULL;
  }

  LOG_DEBUG("\tRedefinindo MyImage->rect...");
  image->rect.x = image->rect.y = image->rect.w = image->rect.h = 0.0f;

  LOG_DEBUG("<<< MyImage_destroy()");
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
bool MyImage_update_texture_with_surface(MyImage* image, SDL_Renderer *renderer, SDL_Surface *surface)
{
  LOG_DEBUG(">>> MyImage_update_texture_with_surface()");

  if (!image)
  {
    LOG_ERROR("Imagem inválida (image == NULL).");
    LOG_DEBUG("<<< MyImage_update_texture_with_surface()");
    return false;
  }

  if (!renderer)
  {
    LOG_ERROR("Renderer inválido (renderer == NULL).");
    LOG_DEBUG("<<< MyImage_update_texture_with_surface()");
    return false;
  }

  if (!surface)
  {
    LOG_ERROR("Superfície inválida (surface == NULL).");
    LOG_DEBUG("<<< MyImage_update_texture_with_surface()");
    return false;
  }

  SDL_DestroyTexture(image->texture);

  image->texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (!image->texture)
  {
    LOG_ERROR("Falha ao criar textura: %s", SDL_GetError());
    LOG_DEBUG("<<< MyImage_update_texture_with_surface()");
    return false;
  }

  // Registra qual surface originou a textura, para que a gravacao em disco
  // salve exatamente o que esta sendo exibido.
  image->displayed = surface;

  LOG_DEBUG("\tObtendo dimensões da textura...");
  SDL_GetTextureSize(image->texture, &image->rect.w, &image->rect.h);

  LOG_DEBUG("<<< MyImage_update_texture_with_surface()");
  return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MyImage_restore_texture(MyImage* image, SDL_Renderer *renderer)
{
  LOG_DEBUG(">>> MyImage_restore_texture()");
  
  if (!MyImage_update_texture_with_surface(image, renderer, image->surface))
  {
    LOG_ERROR("Falha ao restaurar a textura da imagem.");
    return false;
  }

  LOG_DEBUG("<<< MyImage_restore_texture()");
  return true;  
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool load_rgba32(const char *filename, SDL_Renderer *renderer, MyImage *output_image)
{
  if (!filename)
  {
    LOG_DEBUG(">>> load_rgba32(NULL)");
    LOG_ERROR("Nome do arquivo inválido (filename == NULL).");
    LOG_DEBUG("<<< load_rgba32(NULL)");
    return false;
  }

  LOG_DEBUG(">>> load_rgba32(\"%s\")", filename);

  if (!renderer)
  {
    LOG_ERROR("Renderer inválido (renderer == NULL).");
    LOG_DEBUG("<<< load_rgba32(\"%s\")", filename);
    return false;
  }

  if (!output_image)
  {
    LOG_ERROR("Imagem de saída inválida (output_image == NULL).");
    LOG_DEBUG("<<< load_rgba32(\"%s\")", filename);
    return false;
  }

  MyImage_destroy(output_image);

  LOG_DEBUG("\tCarregando imagem \"%s\" em uma superfície...", filename);
  SDL_Surface *surface = IMG_Load(filename);
  if (!surface)
  {
    LOG_ERROR("Formato de imagem inválido ou não suportado em %s (%s)", filename, SDL_GetError());
    LOG_DEBUG("<<< load_rgba32(\"%s\")", filename);
    return false;
  }

  LOG_DEBUG("\tConvertendo superfície para formato RGBA32...");
  output_image->surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
  SDL_DestroySurface(surface);
  if (!output_image->surface)
  {
    LOG_ERROR("Falha ao converter superfície para formato RGBA32: %s", SDL_GetError());
    LOG_DEBUG("<<< load_rgba32(\"%s\")", filename);
    return false;
  }

  LOG_DEBUG("\tCriando textura a partir da superfície...");
  if (!MyImage_update_texture_with_surface(output_image, renderer, output_image->surface))
  {
    LOG_ERROR("Falha ao criar textura.");
    LOG_DEBUG("<<< load_rgba32(\"%s\")", filename);
    return false;
  }

  LOG_DEBUG("<<< load_rgba32(\"%s\")", filename);
  return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MyImage_is_grayscale(const MyImage *image)
{
  if (!image || !image->surface)
  {
    LOG_ERROR("Imagem inválida (image == NULL ou image->surface == NULL).");
    return false;
  }

  SDL_Surface *surface = image->surface;
  const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(surface->format);
  bool is_grayscale = true;

  SDL_LockSurface(surface);

  for (int row = 0; row < surface->h && is_grayscale; ++row)
  {
    // A varredura usa o pitch (distância em bytes entre linhas) em vez de
    // supor que uma linha ocupe exatamente w * 4 bytes: a SDL pode alinhar as
    // linhas e inserir bytes de preenchimento no fim de cada uma.
    const Uint32 *pixels = (const Uint32 *)((const Uint8 *)surface->pixels + (size_t)row * surface->pitch);

    for (int col = 0; col < surface->w; ++col)
    {
      Uint8 r = 0;
      Uint8 g = 0;
      Uint8 b = 0;
      SDL_GetRGB(pixels[col], format, NULL, &r, &g, &b);

      if (r != g || g != b)
      {
        is_grayscale = false;
        break;
      }
    }
  }

  SDL_UnlockSurface(surface);

  return is_grayscale;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MyImage_to_grayscale(MyImage *image, SDL_Renderer *renderer)
{
  LOG_DEBUG(">>> MyImage_to_grayscale()");

  if (!image || !image->surface)
  {
    LOG_ERROR("Imagem inválida (image == NULL ou image->surface == NULL).");
    LOG_DEBUG("<<< MyImage_to_grayscale()");
    return false;
  }

  if (!renderer)
  {
    LOG_ERROR("Renderer inválido (renderer == NULL).");
    LOG_DEBUG("<<< MyImage_to_grayscale()");
    return false;
  }

  SDL_Surface *surface = image->surface;
  const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(surface->format);

  SDL_LockSurface(surface);

  for (int row = 0; row < surface->h; ++row)
  {
    Uint32 *pixels = (Uint32 *)((Uint8 *)surface->pixels + (size_t)row * surface->pitch);

    for (int col = 0; col < surface->w; ++col)
    {
      Uint8 r = 0;
      Uint8 g = 0;
      Uint8 b = 0;
      Uint8 a = 0;
      SDL_GetRGBA(pixels[col], format, NULL, &r, &g, &b, &a);

      const float luminance = GRAYSCALE_WEIGHT_R * r + GRAYSCALE_WEIGHT_G * g + GRAYSCALE_WEIGHT_B * b;
      const Uint8 y = (Uint8)SDL_roundf(luminance);

      pixels[col] = SDL_MapRGBA(format, NULL, y, y, y, a);
    }
  }

  SDL_UnlockSurface(surface);

  LOG_DEBUG("\tAtualizando a textura com a imagem em escala de cinza...");
  if (!MyImage_update_texture_with_surface(image, renderer, surface))
  {
    LOG_ERROR("Falha ao atualizar a textura após a conversão para escala de cinza.");
    LOG_DEBUG("<<< MyImage_to_grayscale()");
    return false;
  }

  LOG_DEBUG("<<< MyImage_to_grayscale()");
  return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MyImage_apply_mapping(MyImage *image, SDL_Renderer *renderer, const Uint8 *mapping)
{
  LOG_DEBUG(">>> MyImage_apply_mapping()");

  if (!image || !image->surface)
  {
    LOG_ERROR("Imagem inválida (image == NULL ou image->surface == NULL).");
    LOG_DEBUG("<<< MyImage_apply_mapping()");
    return false;
  }

  if (!renderer || !mapping)
  {
    LOG_ERROR("Renderer ou tabela de mapeamento inválidos.");
    LOG_DEBUG("<<< MyImage_apply_mapping()");
    return false;
  }

  SDL_Surface *source = image->surface;

  // A surface de destino é criada uma única vez e reaproveitada nas aplicações
  // seguintes, já que as dimensões e o formato não mudam.
  if (!image->processed)
  {
    LOG_DEBUG("\tCriando a superfície de processamento...");
    image->processed = SDL_CreateSurface(source->w, source->h, source->format);

    if (!image->processed)
    {
      LOG_ERROR("Falha ao criar a superfície de processamento: %s", SDL_GetError());
      LOG_DEBUG("<<< MyImage_apply_mapping()");
      return false;
    }
  }

  SDL_Surface *destination = image->processed;
  const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(source->format);

  SDL_LockSurface(source);
  SDL_LockSurface(destination);

  for (int row = 0; row < source->h; ++row)
  {
    const Uint32 *source_pixels = (const Uint32 *)((const Uint8 *)source->pixels + (size_t)row * source->pitch);
    Uint32 *destination_pixels = (Uint32 *)((Uint8 *)destination->pixels + (size_t)row * destination->pitch);

    for (int col = 0; col < source->w; ++col)
    {
      Uint8 r = 0;
      Uint8 g = 0;
      Uint8 b = 0;
      Uint8 a = 0;
      SDL_GetRGBA(source_pixels[col], format, NULL, &r, &g, &b, &a);

      // A imagem de origem está em escala de cinza, então basta mapear um dos
      // canais e replicar o resultado nos três.
      const Uint8 level = mapping[r];

      destination_pixels[col] = SDL_MapRGBA(format, NULL, level, level, level, a);
    }
  }

  SDL_UnlockSurface(destination);
  SDL_UnlockSurface(source);

  if (!MyImage_update_texture_with_surface(image, renderer, destination))
  {
    LOG_ERROR("Falha ao exibir a imagem processada.");
    LOG_DEBUG("<<< MyImage_apply_mapping()");
    return false;
  }

  LOG_DEBUG("<<< MyImage_apply_mapping()");
  return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MyImage_save_png(const MyImage *image, const char *filename, bool *existed)
{
  LOG_DEBUG(">>> MyImage_save_png()");

  if (existed)
    *existed = false;

  if (!image || !image->displayed)
  {
    LOG_ERROR("Não há imagem em exibição para salvar.");
    LOG_DEBUG("<<< MyImage_save_png()");
    return false;
  }

  if (!filename)
  {
    LOG_ERROR("Nome de arquivo inválido (filename == NULL).");
    LOG_DEBUG("<<< MyImage_save_png()");
    return false;
  }

  // A existência precisa ser consultada antes da gravação: depois de gravar,
  // o arquivo existe em qualquer caso e a distinção se perde.
  SDL_PathInfo info = { 0 };
  if (existed && SDL_GetPathInfo(filename, &info) && info.type == SDL_PATHTYPE_FILE)
    *existed = true;

  if (!IMG_SavePNG(image->displayed, filename))
  {
    LOG_ERROR("Falha ao salvar %s: %s", filename, SDL_GetError());
    LOG_DEBUG("<<< MyImage_save_png()");
    return false;
  }

  LOG_DEBUG("<<< MyImage_save_png()");
  return true;
}
