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

#include "histogram.h"
#include "image.h"
#include "text.h"
#include "log.h"
#include "window.h"

//------------------------------------------------------------------------------
// Custom types, structs, constants, etc.
//------------------------------------------------------------------------------
static const char *MAIN_WINDOW_TITLE = "Projeto 1 - Processamento de imagens";
static const char *SECONDARY_WINDOW_TITLE = "Histograma e análise";

enum constants
{
  // Tamanho inicial da janela principal, definido pelo enunciado.
  MAIN_WINDOW_WIDTH = 1024,
  MAIN_WINDOW_HEIGHT = 768,

  // Tamanho fixo da janela secundária, escolhido pelo grupo para comportar o
  // histograma de 256 níveis desenhado a um pixel por nível, as informações de
  // análise e os dois botões, um abaixo do outro.
  SECONDARY_WINDOW_WIDTH = 420,
  SECONDARY_WINDOW_HEIGHT = 560,
};

// Área reservada ao gráfico do histograma dentro da janela secundária: 256
// pixels de largura, um por nível de intensidade, centralizada na janela.
static const SDL_FRect HISTOGRAM_AREA = { 82.0f, 24.0f, 256.0f, 200.0f };

// Tamanho em pontos e cores da fonte usada nos textos da janela secundária.
static const float FONT_SIZE = 15.0f;
static const SDL_Color TEXT_COLOR = { 232, 232, 238, 255 };
static const SDL_Color TEXT_MUTED_COLOR = { 150, 150, 160, 255 };

/**
 * Estado da aplicação. Substitui as variáveis globais do código original: em
 * vez de cada função acessar diretamente a janela e a imagem, ambas são
 * passadas por parâmetro a partir de main().
 */
typedef struct App App;
struct App
{
  MyWindow main_window;
  MyWindow secondary_window;
  MyImage image;
  TextRenderer text;
  Histogram histogram;
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

/**
 * Desenha o conteúdo de cada janela. render() atualiza as duas de uma vez.
 */
static void render_main(const App *app);
static void render_secondary(const App *app);
static void render(const App *app);
static void loop(App *app);

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
void reset_image(App *app)
{
  LOG_DEBUG(">>> reset_image()");

  MyImage_restore_texture(&app->image, app->main_window.renderer);
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

  // As duas janelas nascem ocultas para poderem ser posicionadas antes de
  // aparecer. Criadas visíveis, surgiriam na posição padrão do sistema e
  // saltariam para o lugar certo no quadro seguinte.
  LOG_DEBUG("\tCriando a janela principal...");
  if (!MyWindow_initialize(&app->main_window, MAIN_WINDOW_TITLE, MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT, SDL_WINDOW_HIDDEN))
  {
    LOG_ERROR("Falha ao criar a janela principal: %s", SDL_GetError());
    LOG_DEBUG("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  LOG_DEBUG("\tCriando a janela secundária...");
  if (!MyWindow_initialize(&app->secondary_window, SECONDARY_WINDOW_TITLE, SECONDARY_WINDOW_WIDTH, SECONDARY_WINDOW_HEIGHT, SDL_WINDOW_HIDDEN))
  {
    LOG_ERROR("Falha ao criar a janela secundária: %s", SDL_GetError());
    LOG_DEBUG("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  // O enunciado pede a janela secundária como filha da principal. Além do
  // vínculo lógico, isso faz a secundária acompanhar a principal ao minimizar
  // e permanecer à frente dela.
  LOG_DEBUG("\tTornando a janela secundária filha da principal...");
  if (!SDL_SetWindowParent(app->secondary_window.window, app->main_window.window))
  {
    LOG_ERROR("Falha ao vincular a janela secundária à principal: %s", SDL_GetError());
    LOG_DEBUG("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  // Janela principal centralizada no monitor principal, que não é
  // necessariamente o primeiro da lista do sistema.
  const SDL_DisplayID primary_display = SDL_GetPrimaryDisplay();
  const int centered = (int)SDL_WINDOWPOS_CENTERED_DISPLAY(primary_display);
  SDL_SetWindowPosition(app->main_window.window, centered, centered);

  // O item 3 pede a janela secundária na coordenada (0, 0) da tela.
  // SDL_SetWindowPosition posiciona a área de cliente, então pedir (0, 0)
  // literal empurra a barra de título e a borda para fora da tela: a janela
  // fica sem título visível e sem como ser arrastada ou fechada pelo botão.
  // Deslocar pela espessura da borda encosta a janela inteira no canto
  // superior esquerdo, que é o que a exigência descreve. O exemplo da
  // disciplina faz o mesmo ajuste, pelo mesmo motivo.
  int border_top = 0;
  int border_left = 0;
  SDL_GetWindowBordersSize(app->secondary_window.window, &border_top, &border_left, NULL, NULL);
  SDL_SetWindowPosition(app->secondary_window.window, border_left, border_top);
  LOG_DEBUG("\tBorda da janela secundária: topo %d, esquerda %d", border_top, border_left);

  // A secundária foi criada oculta explicitamente, então não é reexibida junto
  // com a principal e precisa ser mostrada por conta própria.
  SDL_ShowWindow(app->main_window.window);
  SDL_ShowWindow(app->secondary_window.window);

  // A fonte é carregada depois das janelas porque os textos só são desenhados
  // por meio de um renderizador.
  LOG_DEBUG("\tCarregando a fonte dos textos...");
  if (!Text_initialize(&app->text, FONT_SIZE))
  {
    LOG_DEBUG("<<< initialize()");
    return SDL_APP_FAILURE;
  }

  if (DEBUG_ENABLED)
  {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    SDL_Rect bounds = { 0, 0, 0, 0 };

    SDL_GetDisplayUsableBounds(primary_display, &bounds);
    LOG_DEBUG("\tÁrea útil do monitor primário: %dx%d em (%d, %d)", bounds.w, bounds.h, bounds.x, bounds.y);

    SDL_GetWindowPosition(app->main_window.window, &x, &y);
    SDL_GetWindowSize(app->main_window.window, &w, &h);
    LOG_DEBUG("\tJanela principal: %dx%d em (%d, %d)", w, h, x, y);

    SDL_GetWindowPosition(app->secondary_window.window, &x, &y);
    SDL_GetWindowSize(app->secondary_window.window, &w, &h);
    LOG_DEBUG("\tJanela secundária: %dx%d em (%d, %d)", w, h, x, y);
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

  Text_shutdown(&app->text);
  MyImage_destroy(&app->image);

  // A janela filha é destruída antes da janela pai.
  MyWindow_destroy(&app->secondary_window);
  MyWindow_destroy(&app->main_window);

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
  if (!SDL_GetWindowSize(app->main_window.window, &window_width, &window_height))
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
void render_main(const App *app)
{
  SDL_Renderer *renderer = app->main_window.renderer;

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
void render_secondary(const App *app)
{
  SDL_Renderer *renderer = app->secondary_window.renderer;

  SDL_SetRenderDrawColor(renderer, 24, 24, 28, 255);
  SDL_RenderClear(renderer);

  Text_draw(&app->text, renderer, HISTOGRAM_AREA.x, 2.0f, TEXT_COLOR, "Histograma");

  // O gráfico do histograma e as informações de análise (item 4) e os dois
  // botões (itens 5 e 6) entram nesta janela. Por enquanto só a área reservada
  // ao gráfico é delimitada.
  SDL_SetRenderDrawColor(renderer, 70, 70, 80, 255);
  SDL_RenderRect(renderer, &HISTOGRAM_AREA);

  Text_draw(&app->text, renderer, 32.0f, 244.0f, TEXT_COLOR, "Informações da imagem");
  Text_draw(&app->text, renderer, 32.0f, 272.0f, TEXT_MUTED_COLOR, "Brilho e contraste: itens 4 e 5.");

  SDL_RenderPresent(renderer);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void render(const App *app)
{
  render_main(app);
  render_secondary(app);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void loop(App *app)
{
  LOG_DEBUG(">>> loop()");

  render(app);

  // As duas janelas têm conteúdo próprio, então cada evento precisa ser
  // encaminhado para a janela de origem, identificada pelo windowID.
  const SDL_WindowID main_window_id = SDL_GetWindowID(app->main_window.window);
  const SDL_WindowID secondary_window_id = SDL_GetWindowID(app->secondary_window.window);

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

      // Fechar qualquer uma das janelas encerra o programa. A secundária
      // concentra os controles, e deixá-la fechada tornaria a equalização e a
      // troca de resolução inacessíveis.
      case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        isRunning = false;
        break;

      // Redesenha apenas a janela que precisou ser reexibida.
      case SDL_EVENT_WINDOW_EXPOSED:
        if (event.window.windowID == main_window_id)
          render_main(app);
        else if (event.window.windowID == secondary_window_id)
          render_secondary(app);
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
    .main_window      = { .window = NULL, .renderer = NULL },
    .secondary_window = { .window = NULL, .renderer = NULL },
    .text = { .font = NULL, .initialized = false },
    .histogram = { .counts = { 0 }, .max_count = 0, .total_pixels = 0 },
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

  if (!load_rgba32(image_filename, app.main_window.renderer, &app.image))
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

    if (!MyImage_to_grayscale(&app.image, app.main_window.renderer))
    {
      shutdown(&app);
      return EXIT_FAILURE;
    }
  }

  // O histograma é calculado sobre a imagem em escala de cinza, que é a base
  // das operações seguintes.
  if (!Histogram_compute(&app.histogram, app.image.surface))
  {
    shutdown(&app);
    return EXIT_FAILURE;
  }

  loop(&app);

  shutdown(&app);
  return EXIT_SUCCESS;
}
