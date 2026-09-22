// Copyright (c) 2026 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Projeto 1 - Computação Visual (UPM FCI CC)
// Integrantes: ver README.md
//------------------------------------------------------------------------------

#include "button.h"

//------------------------------------------------------------------------------
// Cores dos três estados. O enunciado sugere azul para o estado neutro, azul
// claro para o ponteiro sobre o botão e azul escuro para o botão pressionado.
//------------------------------------------------------------------------------
static const SDL_Color NEUTRAL_COLOR = {  42,  92, 170, 255 };
static const SDL_Color HOVER_COLOR   = {  74, 134, 224, 255 };
static const SDL_Color PRESSED_COLOR = {  24,  54, 110, 255 };
static const SDL_Color BORDER_COLOR  = { 150, 180, 230, 255 };
static const SDL_Color LABEL_COLOR   = { 240, 244, 250, 255 };

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static bool contains(const SDL_FRect *bounds, float x, float y)
{
  return x >= bounds->x && x < bounds->x + bounds->w
      && y >= bounds->y && y < bounds->y + bounds->h;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Button_initialize(Button *button, SDL_FRect bounds, const char *label)
{
  if (!button)
    return;

  button->bounds = bounds;
  button->label = label;
  button->state = BUTTON_STATE_NEUTRAL;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Button_set_label(Button *button, const char *label)
{
  if (button)
    button->label = label;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Button_handle_mouse_motion(Button *button, float x, float y)
{
  if (!button)
    return false;

  // Um botão pressionado continua pressionado enquanto o mouse não for solto,
  // mesmo que o ponteiro passeie para fora da área.
  if (button->state == BUTTON_STATE_PRESSED)
    return false;

  const ButtonState previous = button->state;
  button->state = contains(&button->bounds, x, y) ? BUTTON_STATE_HOVER : BUTTON_STATE_NEUTRAL;

  return button->state != previous;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Button_handle_mouse_down(Button *button, float x, float y)
{
  if (!button || !contains(&button->bounds, x, y))
    return false;

  const ButtonState previous = button->state;
  button->state = BUTTON_STATE_PRESSED;

  return button->state != previous;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Button_handle_mouse_up(Button *button, float x, float y, bool *activated)
{
  if (activated)
    *activated = false;

  if (!button)
    return false;

  const bool was_pressed = (button->state == BUTTON_STATE_PRESSED);
  const bool inside = contains(&button->bounds, x, y);

  // A ação só acontece quando o clique começa e termina dentro do botão, que é
  // o comportamento esperado de uma interface: arrastar o ponteiro para fora
  // antes de soltar cancela o clique.
  if (was_pressed && inside && activated)
    *activated = true;

  const ButtonState previous = button->state;
  button->state = inside ? BUTTON_STATE_HOVER : BUTTON_STATE_NEUTRAL;

  return button->state != previous;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Button_draw(const Button *button, SDL_Renderer *renderer, const TextRenderer *text)
{
  if (!button || !renderer)
    return;

  SDL_Color fill = NEUTRAL_COLOR;
  switch (button->state)
  {
    case BUTTON_STATE_HOVER:   fill = HOVER_COLOR;   break;
    case BUTTON_STATE_PRESSED: fill = PRESSED_COLOR; break;
    case BUTTON_STATE_NEUTRAL: fill = NEUTRAL_COLOR; break;
  }

  SDL_SetRenderDrawColor(renderer, fill.r, fill.g, fill.b, fill.a);
  SDL_RenderFillRect(renderer, &button->bounds);

  SDL_SetRenderDrawColor(renderer, BORDER_COLOR.r, BORDER_COLOR.g, BORDER_COLOR.b, BORDER_COLOR.a);
  SDL_RenderRect(renderer, &button->bounds);

  if (!text || !button->label)
    return;

  int label_width = 0;
  int label_height = 0;
  if (!Text_measure(text, button->label, &label_width, &label_height))
    return;

  const float label_x = button->bounds.x + (button->bounds.w - (float)label_width) * 0.5f;
  const float label_y = button->bounds.y + (button->bounds.h - (float)label_height) * 0.5f;

  Text_draw(text, renderer, label_x, label_y, LABEL_COLOR, button->label);
}
