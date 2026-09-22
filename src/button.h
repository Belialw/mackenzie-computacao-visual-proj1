// Copyright (c) 2026 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Projeto 1 - Computação Visual (UPM FCI CC)
// Integrantes: ver README.md
//
// Botão desenhado com primitivas da SDL, como exigem os itens 5 e 6 do escopo.
//------------------------------------------------------------------------------

#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>
#include <SDL3/SDL.h>

#include "text.h"

/**
 * Estado visual do botão, que precisa refletir a interação do usuário.
 */
typedef enum ButtonState
{
  BUTTON_STATE_NEUTRAL,
  BUTTON_STATE_HOVER,
  BUTTON_STATE_PRESSED,
} ButtonState;

/**
 * O rótulo não é copiado: o botão guarda o ponteiro recebido, que precisa
 * apontar para uma string de tempo de vida maior que o do botão. Neste projeto
 * os rótulos são literais, que existem durante toda a execução.
 */
typedef struct Button Button;
struct Button
{
  SDL_FRect bounds;
  const char *label;
  ButtonState state;
};

/**
 * Prepara o botão com a área e o rótulo iniciais.
 */
void Button_initialize(Button *button, SDL_FRect bounds, const char *label);

/**
 * Troca o texto do botão, que deve refletir a ação do próximo clique.
 */
void Button_set_label(Button *button, const char *label);

/**
 * Atualiza o estado visual conforme o ponteiro do mouse entra e sai da área do
 * botão. Retorna true se o estado mudou, caso em que a janela precisa ser
 * redesenhada.
 */
bool Button_handle_mouse_motion(Button *button, float x, float y);

/**
 * Registra o pressionar do botão do mouse sobre a área. Retorna true se o
 * estado visual mudou.
 */
bool Button_handle_mouse_down(Button *button, float x, float y);

/**
 * Registra o soltar do botão do mouse. Preenche `activated` com true quando o
 * clique começou e terminou dentro da área, que é quando a ação deve ser
 * executada. Retorna true se o estado visual mudou.
 */
bool Button_handle_mouse_up(Button *button, float x, float y, bool *activated);

/**
 * Desenha o botão com primitivas da SDL e o rótulo centralizado.
 */
void Button_draw(const Button *button, SDL_Renderer *renderer, const TextRenderer *text);

#endif // BUTTON_H
