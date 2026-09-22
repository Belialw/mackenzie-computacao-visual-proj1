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
