// Copyright (c) 2026 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Projeto 1 - Computação Visual (UPM FCI CC)
// Integrantes: ver README.md
//
// Carregamento da fonte e desenho de texto com a SDL_ttf.
//------------------------------------------------------------------------------

#ifndef TEXT_H
#define TEXT_H

#include <stdbool.h>
#include <SDL3/SDL.h>

//------------------------------------------------------------------------------
// A SDL_ttf é apenas sugerida pelo enunciado, não obrigatória: as bibliotecas
// exigidas são a SDL3 e a SDL_image. Compilar com USE_SDL_TTF=0 remove a
// dependência, e o programa continua funcionando — apenas sem os textos.
// Existe para que a ausência da biblioteca na máquina de quem compila não
// impeça o projeto de ser construído. Ver README.md.
//------------------------------------------------------------------------------
#ifndef USE_SDL_TTF
  #define USE_SDL_TTF 1
#endif

#if USE_SDL_TTF
  #include <SDL3_ttf/SDL_ttf.h>
#endif

/**
 * Fonte carregada para os textos do programa. Fica guardada no estado da
 * aplicação, e não em uma variável global do módulo, pelo mesmo motivo que
 * levou à remoção das variáveis globais do código original.
 */
typedef struct TextRenderer TextRenderer;
struct TextRenderer
{
#if USE_SDL_TTF
  TTF_Font *font;
#endif
  bool initialized;
};

/**
 * Inicializa a SDL_ttf e carrega, no tamanho indicado, a fonte distribuída
 * junto com o programa.
 *
 * O caminho da fonte é montado a partir de SDL_GetBasePath(), que devolve o
 * diretório do executável. Isso é o que garante a exigência do item 8 do
 * escopo: a fonte é encontrada independentemente do sistema operacional e do
 * diretório de onde o programa for chamado, sem depender de nenhuma fonte
 * instalada na máquina.
 */
bool Text_initialize(TextRenderer *text, float size);

/**
 * Libera a fonte e encerra a SDL_ttf. Chamar com `text` nulo é seguro.
 */
void Text_shutdown(TextRenderer *text);

/**
 * Desenha `string` com o canto superior esquerdo em (x, y). Retorna false e
 * registra o erro caso não consiga desenhar.
 */
bool Text_draw(const TextRenderer *text, SDL_Renderer *renderer, float x, float y, SDL_Color color, const char *string);


/**
 * Mede a largura e a altura que `string` ocuparia ao ser desenhada, sem
 * desenhá-la. Usado para centralizar rótulos dentro dos botões.
 */
bool Text_measure(const TextRenderer *text, const char *string, int *width, int *height);

#endif // TEXT_H
