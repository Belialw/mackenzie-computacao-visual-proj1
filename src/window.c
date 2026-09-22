// Copyright (c) 2026 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Projeto 1 - Computação Visual (UPM FCI CC)
// Integrantes: ver README.md
//------------------------------------------------------------------------------

#include "log.h"
#include "window.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MyWindow_initialize(MyWindow *window, const char *title, int width, int height, SDL_WindowFlags window_flags)
{
  LOG_DEBUG("\tMyWindow_initialize(%s, %d, %d)", title, width, height);

  if (!window)
  {
    LOG_ERROR("Janela/renderizador inválidos (window == NULL).");
    return false;
  }

  return SDL_CreateWindowAndRenderer(title, width, height, window_flags, &window->window, &window->renderer);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyWindow_destroy(MyWindow *window)
{
  LOG_DEBUG(">>> MyWindow_destroy()");

  if (!window)
  {
    LOG_ERROR("Janela/renderizador inválidos (window == NULL).");
    LOG_DEBUG("<<< MyWindow_destroy()");
    return;
  }

  LOG_DEBUG("\tDestruindo MyWindow->renderer...");
  SDL_DestroyRenderer(window->renderer);
  window->renderer = NULL;

  LOG_DEBUG("\tDestruindo MyWindow->window...");
  SDL_DestroyWindow(window->window);
  window->window = NULL;

  LOG_DEBUG("<<< MyWindow_destroy()");
}
