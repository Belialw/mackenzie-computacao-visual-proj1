// Copyright (c) 2026 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Projeto 1 - Computação Visual (UPM FCI CC)
// Integrantes: ver README.md
//------------------------------------------------------------------------------

#include "text.h"
#include "log.h"

// Caminho da fonte relativo ao diretório do executável. O Makefile copia a
// pasta assets/ para junto do binário justamente para que este caminho valha.
static const char *FONT_RELATIVE_PATH = "assets/fonts/DejaVuSans.ttf";

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Text_initialize(TextRenderer *text, float size)
{
  LOG_DEBUG(">>> Text_initialize()");

  if (!text)
  {
    LOG_ERROR("Estado de texto inválido (text == NULL).");
    LOG_DEBUG("<<< Text_initialize()");
    return false;
  }

  text->font = NULL;
  text->initialized = false;

  if (!TTF_Init())
  {
    LOG_ERROR("Falha ao iniciar a SDL_ttf: %s", SDL_GetError());
    LOG_DEBUG("<<< Text_initialize()");
    return false;
  }

  text->initialized = true;

  // SDL_GetBasePath() devolve o diretório do executável, já com o separador no
  // fim. Um caminho relativo ao diretório de trabalho faria o programa perder
  // a fonte sempre que fosse chamado de outra pasta.
  const char *base_path = SDL_GetBasePath();
  if (!base_path)
  {
    LOG_ERROR("Não foi possível determinar o diretório do executável: %s", SDL_GetError());
    LOG_DEBUG("<<< Text_initialize()");
    return false;
  }

  char font_path[1024] = { 0 };
  const int written = SDL_snprintf(font_path, sizeof(font_path), "%s%s", base_path, FONT_RELATIVE_PATH);
  if (written < 0 || (size_t)written >= sizeof(font_path))
  {
    LOG_ERROR("O caminho da fonte não coube no buffer (base: %s).", base_path);
    LOG_DEBUG("<<< Text_initialize()");
    return false;
  }

  LOG_DEBUG("\tCarregando a fonte %s...", font_path);
  text->font = TTF_OpenFont(font_path, size);
  if (!text->font)
  {
    LOG_ERROR("Falha ao carregar a fonte %s: %s", font_path, SDL_GetError());
    LOG_ERROR("A fonte é distribuída com o programa e deve estar em %s, ao lado do executável.", FONT_RELATIVE_PATH);
    LOG_DEBUG("<<< Text_initialize()");
    return false;
  }

  LOG_DEBUG("<<< Text_initialize()");
  return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Text_shutdown(TextRenderer *text)
{
  LOG_DEBUG(">>> Text_shutdown()");

  if (!text)
  {
    LOG_DEBUG("<<< Text_shutdown()");
    return;
  }

  if (text->font)
  {
    LOG_DEBUG("\tLiberando a fonte...");
    TTF_CloseFont(text->font);
    text->font = NULL;
  }

  // Só encerra a SDL_ttf se ela chegou a ser iniciada, para não desequilibrar
  // a contagem interna da biblioteca quando a inicialização falha antes disso.
  if (text->initialized)
  {
    LOG_DEBUG("\tEncerrando a SDL_ttf...");
    TTF_Quit();
    text->initialized = false;
  }

  LOG_DEBUG("<<< Text_shutdown()");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Text_draw(const TextRenderer *text, SDL_Renderer *renderer, float x, float y, SDL_Color color, const char *string)
{
  if (!text || !text->font || !renderer || !string)
    return false;

  // O comprimento 0 indica à SDL_ttf que a string termina em '\0'.
  SDL_Surface *surface = TTF_RenderText_Blended(text->font, string, 0, color);
  if (!surface)
  {
    LOG_ERROR("Falha ao desenhar o texto: %s", SDL_GetError());
    return false;
  }

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_DestroySurface(surface);

  if (!texture)
  {
    LOG_ERROR("Falha ao criar a textura do texto: %s", SDL_GetError());
    return false;
  }

  SDL_FRect destination = { x, y, 0.0f, 0.0f };
  SDL_GetTextureSize(texture, &destination.w, &destination.h);
  SDL_RenderTexture(renderer, texture, NULL, &destination);
  SDL_DestroyTexture(texture);

  return true;
}
