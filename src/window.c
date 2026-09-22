// Copyright (c) 2026 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Projeto 1 - Computação Visual (UPM FCI CC)
// Integrantes: ver README.md
//------------------------------------------------------------------------------

#include "window.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MyWindow_initialize(MyWindow *window, const char *title, int width, int height, SDL_WindowFlags window_flags)
{
  SDL_Log("\tMyWindow_initialize(%s, %d, %d)", title, width, height);

  if (!window)
  {
    SDL_Log("\t\t*** Erro: Janela/renderizador inválidos (window == NULL).");
    return false;
  }

  return SDL_CreateWindowAndRenderer(title, width, height, window_flags, &window->window, &window->renderer);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyWindow_destroy(MyWindow *window)
{
  SDL_Log(">>> MyWindow_destroy()");

  if (!window)
  {
    SDL_Log("\t*** Erro: Janela/renderizador inválidos (window == NULL).");
    SDL_Log("<<< MyWindow_destroy()");
    return;
  }

  SDL_Log("\tDestruindo MyWindow->renderer...");
  SDL_DestroyRenderer(window->renderer);
  window->renderer = NULL;

  SDL_Log("\tDestruindo MyWindow->window...");
  SDL_DestroyWindow(window->window);
  window->window = NULL;

  SDL_Log("<<< MyWindow_destroy()");
}
