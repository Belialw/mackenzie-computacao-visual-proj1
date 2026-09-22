// Copyright (c) 2026 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Projeto 1 - Computação Visual (UPM FCI CC)
// Integrantes: ver README.md
//
// Separa as mensagens de diagnóstico das mensagens destinadas ao usuário.
//
// O código-base registrava em log a entrada e a saída de praticamente toda
// função. Isso é útil durante o desenvolvimento e é apenas ruído para quem usa
// o programa, ainda mais porque o enunciado exige que algumas mensagens
// específicas apareçam no terminal — que se perdem no meio do rastreamento.
//
// LOG_DEBUG só produz saída na build de depuração (make DEBUG=1).
// LOG_INFO e LOG_ERROR existem nas duas builds.
//------------------------------------------------------------------------------

#ifndef LOG_H
#define LOG_H

#include <SDL3/SDL.h>

/**
 * Rastreamento interno: entrada e saída de funções, etapas do processamento.
 * Não aparece na build de release.
 *
 * A forma `if (0)` mantém a chamada visível para o compilador, que continua
 * verificando o formato e os argumentos e continua considerando as variáveis
 * como usadas, mas descarta o código na otimização. Um `((void)0)` puro
 * deixaria erros de formato passarem despercebidos na build de release.
 */
#ifdef DEBUG
  #define LOG_DEBUG(...) SDL_Log(__VA_ARGS__)
#else
  #define LOG_DEBUG(...) do { if (0) SDL_Log(__VA_ARGS__); } while (0)
#endif

/**
 * Informação dirigida a quem está usando o programa.
 */
/**
 * Permite escrever blocos que só interessam à depuração sem recorrer a #ifdef
 * no meio do código: o compilador analisa o bloco nas duas builds e o descarta
 * na otimização quando DEBUG_ENABLED é falso.
 */
#ifdef DEBUG
  #define DEBUG_ENABLED 1
#else
  #define DEBUG_ENABLED 0
#endif

#define LOG_INFO(...) SDL_Log(__VA_ARGS__)

/**
 * Erro dirigido a quem está usando o programa.
 */
#define LOG_ERROR(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)

#endif // LOG_H
