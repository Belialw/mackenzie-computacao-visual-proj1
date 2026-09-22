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
#include "log.h"
#include "window.h"

//------------------------------------------------------------------------------
// Custom types, structs, constants, etc.
//------------------------------------------------------------------------------
static const char *WINDOW_TITLE = "Projeto 1 - Processamento de imagens";

enum constants
{
  DEFAULT_WINDOW_WIDTH = 1024,
  DEFAULT_WINDOW_HEIGHT = 768,
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
/**
 * Calcula onde a imagem deve ser desenhada dentro da janela principal: a maior
 * escala que couber sem distorcer as proporções, centralizada.
 */
static bool compute_image_destination(const App *app, SDL_FRect *destination);

static void render(const App *app);
static void loop(App *app);

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
void reset_image(App *app)
{
  LOG_DEBUG(">>> reset_image()");

  MyImage_restore_texture(&app->image, app->window.renderer);
  render(app);

  LOG_DEBUG("<<< reset_image()");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SDL_AppResult initialize(App *app)
{
  LOG_DEBUG(">>> initialize()");

  LOG_DEBUG("\tIniciando SDL...");
  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    LOG_ERROR("Falha ao iniciar a SDL: %s", SDL_GetError());
    LOG_DEBUG("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  // A janela nasce oculta para poder ser posicionada antes de aparecer. Criada
  // visível, ela surgiria na posição padrão do sistema e só então saltaria
  // para o centro do monitor.
  LOG_DEBUG("\tCriando janela e renderizador...");
  if (!MyWindow_initialize(&app->window, WINDOW_TITLE, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, SDL_WINDOW_HIDDEN))
  {
    LOG_ERROR("Falha ao criar a janela e/ou renderizador: %s", SDL_GetError());
    LOG_DEBUG("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  // O enunciado exige que a janela principal inicie centralizada no monitor
  // principal, que não é necessariamente o primeiro da lista do sistema.
  const SDL_DisplayID primary_display = SDL_GetPrimaryDisplay();
  const int centered = (int)SDL_WINDOWPOS_CENTERED_DISPLAY(primary_display);

  LOG_DEBUG("\tCentralizando a janela principal no monitor primário (id %u)...", (unsigned)primary_display);
  SDL_SetWindowPosition(app->window.window, centered, centered);
  SDL_ShowWindow(app->window.window);

  if (DEBUG_ENABLED)
  {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    SDL_Rect bounds = { 0, 0, 0, 0 };
    SDL_GetWindowPosition(app->window.window, &x, &y);
    SDL_GetWindowSize(app->window.window, &w, &h);
    SDL_GetDisplayUsableBounds(primary_display, &bounds);
    LOG_DEBUG("\tJanela principal: %dx%d em (%d, %d); área útil do monitor: %dx%d em (%d, %d)",
      w, h, x, y, bounds.w, bounds.h, bounds.x, bounds.y);
  }

  LOG_DEBUG("<<< initialize()");
  return SDL_APP_CONTINUE;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void shutdown(App *app)
{
  LOG_DEBUG(">>> shutdown()");

  MyImage_destroy(&app->image);
  MyWindow_destroy(&app->window);

  LOG_DEBUG("\tEncerrando SDL...");
  SDL_Quit();

  LOG_DEBUG("<<< shutdown()");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool compute_image_destination(const App *app, SDL_FRect *destination)
{
  if (!app || !app->image.texture || !destination)
    return false;

  int window_width = 0;
  int window_height = 0;
  if (!SDL_GetWindowSize(app->window.window, &window_width, &window_height))
    return false;

  const float image_width = app->image.rect.w;
  const float image_height = app->image.rect.h;

  if (image_width <= 0.0f || image_height <= 0.0f || window_width <= 0 || window_height <= 0)
    return false;

  // Maior fator de escala que mantém a imagem inteira dentro da janela sem
  // distorcer as proporções. O que sobrar vira margem dos dois lados.
  const float scale = SDL_min((float)window_width / image_width,
                              (float)window_height / image_height);

  destination->w = image_width * scale;
  destination->h = image_height * scale;
  destination->x = ((float)window_width - destination->w) * 0.5f;
  destination->y = ((float)window_height - destination->h) * 0.5f;

  return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void render(const App *app)
{
  SDL_Renderer *renderer = app->window.renderer;

  SDL_SetRenderDrawColor(renderer, 32, 32, 32, 255);
  SDL_RenderClear(renderer);

  SDL_FRect destination = { 0.0f, 0.0f, 0.0f, 0.0f };
  if (compute_image_destination(app, &destination))
    SDL_RenderTexture(renderer, app->image.texture, NULL, &destination);

  SDL_RenderPresent(renderer);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void loop(App *app)
{
  LOG_DEBUG(">>> loop()");

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
  
  LOG_DEBUG("<<< loop()");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void print_usage(const char *program_name)
{
  LOG_INFO("Uso: %s caminho_da_imagem.ext", program_name);
  LOG_INFO("Exemplo: %s samples/kodim23.png", program_name);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool check_image_path(const char *filename)
{
  SDL_PathInfo info = { 0 };

  if (!SDL_GetPathInfo(filename, &info) || info.type == SDL_PATHTYPE_NONE)
  {
    LOG_ERROR("Arquivo não encontrado: %s", filename);
    return false;
  }

  if (info.type == SDL_PATHTYPE_DIRECTORY)
  {
    LOG_ERROR("%s é um diretório, não um arquivo de imagem.", filename);
    return false;
  }

  if (info.type != SDL_PATHTYPE_FILE)
  {
    LOG_ERROR("%s não é um arquivo comum.", filename);
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
    LOG_ERROR("Caminho da imagem não informado.");
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  if (argc > 2)
  {
    LOG_ERROR("O programa recebe apenas um argumento.");
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

  // O enunciado exige informar no terminal se a imagem de entrada é colorida
  // ou já está em escala de cinza, e converter apenas no primeiro caso. A
  // imagem em escala de cinza é a base de todas as operações seguintes.
  if (MyImage_is_grayscale(&app.image))
  {
    LOG_INFO("A imagem de entrada já está em escala de cinza.");
  }
  else
  {
    LOG_INFO("A imagem de entrada é colorida. Convertendo para escala de cinza...");

    if (!MyImage_to_grayscale(&app.image, app.window.renderer))
    {
      shutdown(&app);
      return EXIT_FAILURE;
    }
  }

  loop(&app);

  shutdown(&app);
  return EXIT_SUCCESS;
}
