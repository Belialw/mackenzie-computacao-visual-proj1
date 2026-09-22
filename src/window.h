// Copyright (c) 2026 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Projeto 1 - Computação Visual (UPM FCI CC)
// Integrantes: ver README.md
//
// Janela e renderizador da SDL.
//------------------------------------------------------------------------------

#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>
#include <SDL3/SDL.h>

/**
 * Agrupa uma janela da SDL e o renderizador associado a ela, que sempre são
 * criados e destruídos juntos.
 */
typedef struct MyWindow MyWindow;
struct MyWindow
{
  SDL_Window *window;
  SDL_Renderer *renderer;
};

/**
 * Cria a janela e o renderizador em `window`. Retorna false em caso de erro.
 */
bool MyWindow_initialize(MyWindow *window, const char *title, int width, int height, SDL_WindowFlags window_flags);

/**
 * Destrói o renderizador e a janela de `window`, deixando ambos os ponteiros
 * nulos. Chamar com `window` nulo é seguro.
 */
void MyWindow_destroy(MyWindow *window);

#endif // WINDOW_H
