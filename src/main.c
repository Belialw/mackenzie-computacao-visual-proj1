// Copyright (c) 2026 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Projeto 1 - Computação Visual (UPM FCI CC)
// Prof. André Kishimoto
//
// Integrantes:
//   Enzo Ponte Gamberi              - RA 10389931
//   Luís Henrique Ribeiro Fernandes - RA 10420046
//   Raphael Grizante da Silva       - RA 10416979
//   Vinícius Brait Lorimier         - RA 10420046
//
// Programa de processamento de imagens: carrega a imagem informada na linha de
// comando, converte para escala de cinza e permite analisá-la e processá-la em
// uma interface gráfica de duas janelas.
//
//   imgproc caminho_da_imagem.ext
//
// Baseado no exemplo src/05-filter_image do repositório da disciplina
// (https://github.com/profkishimoto/CompVis262), de autoria do professor.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "image.h"
#include "window.h"

//------------------------------------------------------------------------------
// Custom types, structs, constants, etc.
//------------------------------------------------------------------------------
static const char *WINDOW_TITLE = "Filter image";

enum constants
{
  DEFAULT_WINDOW_WIDTH = 640,
  DEFAULT_WINDOW_HEIGHT = 480,
};

/**
 * Estado da aplicação. Substitui as variáveis globais do código original: em
 * vez de cada função acessar diretamente a janela e a imagem, ambas são
 * passadas por parâmetro a partir de main().
 */
typedef struct App App;
struct App
{
  MyWindow window;
  MyImage image;
};

//------------------------------------------------------------------------------
// Function declaration
//------------------------------------------------------------------------------

/**
 * Exibe no terminal como o programa deve ser chamado.
 */
static void print_usage(const char *program_name);

/**
 * Verifica se o caminho recebido como argumento do programa aponta para um
 * arquivo existente, exibindo uma mensagem de erro pertinente no terminal caso
 * contrário. A validação do conteúdo (se o arquivo é mesmo uma imagem, em um
 * formato suportado) fica por conta do IMG_Load(), em load_rgba32().
 */
static bool check_image_path(const char *filename);

static void reset_image(App *app);

static SDL_AppResult initialize(App *app);
static void shutdown(App *app);
static void render(const App *app);
static void loop(App *app);

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
void reset_image(App *app)
{
  SDL_Log(">>> reset_image()");

  MyImage_restore_texture(&app->image, app->window.renderer);
  render(app);

  SDL_Log("<<< reset_image()");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SDL_AppResult initialize(App *app)
{
  SDL_Log(">>> initialize()");

  SDL_Log("\tIniciando SDL...");
  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    SDL_Log("\t*** Erro ao iniciar a SDL: %s", SDL_GetError());
    SDL_Log("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  SDL_Log("\tCriando janela e renderizador...");
  if (!MyWindow_initialize(&app->window, WINDOW_TITLE, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, 0))
  {
    SDL_Log("\t*** Erro ao criar a janela e/ou renderizador: %s", SDL_GetError());
    SDL_Log("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  SDL_Log("<<< initialize()");
  return SDL_APP_CONTINUE;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void shutdown(App *app)
{
  SDL_Log(">>> shutdown()");

  MyImage_destroy(&app->image);
  MyWindow_destroy(&app->window);

  SDL_Log("\tEncerrando SDL...");
  SDL_Quit();

  SDL_Log("<<< shutdown()");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void render(const App *app)
{
  SDL_SetRenderDrawColor(app->window.renderer, 128, 128, 128, 255);
  SDL_RenderClear(app->window.renderer);

  SDL_RenderTexture(app->window.renderer, app->image.texture, &app->image.rect, &app->image.rect);

  SDL_RenderPresent(app->window.renderer);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void loop(App *app)
{
  SDL_Log(">>> loop()");

  render(app);

  SDL_Event event;
  bool isRunning = true;
  while (isRunning)
  {
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
      case SDL_EVENT_QUIT:
        isRunning = false;
        break;

      case SDL_EVENT_KEY_DOWN:
        if (!event.key.repeat)
        {
          switch (event.key.key)
          {
            case SDLK_R: // fallthrough.
            case SDLK_0: reset_image(app); break;
          }
        }
        break;
      }
    }

    // Breve pausa para diminuir o processamento contínuo do programa...
    SDL_Delay(50);
  }
  
  SDL_Log("<<< loop()");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void print_usage(const char *program_name)
{
  SDL_Log("Uso: %s caminho_da_imagem.ext", program_name);
  SDL_Log("Exemplo: %s samples/kodim23.png", program_name);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool check_image_path(const char *filename)
{
  SDL_PathInfo info = { 0 };

  if (!SDL_GetPathInfo(filename, &info) || info.type == SDL_PATHTYPE_NONE)
  {
    SDL_Log("*** Erro: arquivo não encontrado: %s", filename);
    return false;
  }

  if (info.type == SDL_PATHTYPE_DIRECTORY)
  {
    SDL_Log("*** Erro: %s é um diretório, não um arquivo de imagem.", filename);
    return false;
  }

  if (info.type != SDL_PATHTYPE_FILE)
  {
    SDL_Log("*** Erro: %s não é um arquivo comum.", filename);
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    SDL_Log("*** Erro: caminho da imagem não informado.");
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  if (argc > 2)
  {
    SDL_Log("*** Erro: o programa recebe apenas um argumento.");
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  const char *image_filename = argv[1];

  if (!check_image_path(image_filename))
    return EXIT_FAILURE;

  App app = {
    .window = { .window = NULL, .renderer = NULL },
    .image = {
      .surface = NULL,
      .texture = NULL,
      .rect = { .x = 0.0f, .y = 0.0f, .w = 0.0f, .h = 0.0f }
    }
  };

  if (initialize(&app) == SDL_APP_FAILURE)
  {
    shutdown(&app);
    return EXIT_FAILURE;
  }

  if (!load_rgba32(image_filename, app.window.renderer, &app.image))
  {
    shutdown(&app);
    return EXIT_FAILURE;
  }

  // Altera tamanho da janela se a imagem for maior do que o tamanho padrão
  // e reposiciona no canto superior esquerdo da tela.
  int imageWidth = (int)app.image.rect.w;
  int imageHeight = (int)app.image.rect.h;
  if (imageWidth > DEFAULT_WINDOW_WIDTH || imageHeight > DEFAULT_WINDOW_HEIGHT)
  {
    // Obtém o tamanho da borda da janela: posicionar a janela na coordenada
    // (0, 0) faria com que a borda do programa ficasse fora da região da tela.
    int top = 0;
    int left = 0;
    SDL_GetWindowBordersSize(app.window.window, &top, &left, NULL, NULL);

    SDL_Log("Redefinindo dimensões da janela, de (%d, %d) para (%d, %d), e alterando a posição para (%d, %d).",
      DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, imageWidth, imageHeight, left, top);

    SDL_SetWindowSize(app.window.window, imageWidth, imageHeight);
    SDL_SetWindowPosition(app.window.window, left, top);

    SDL_SyncWindow(app.window.window);
  }

  loop(&app);

  shutdown(&app);
  return EXIT_SUCCESS;
}
