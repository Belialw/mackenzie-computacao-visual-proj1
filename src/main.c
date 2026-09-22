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
#include <SDL3_image/SDL_image.h>

//------------------------------------------------------------------------------
// Custom types, structs, constants, etc.
//------------------------------------------------------------------------------
static const char *WINDOW_TITLE = "Filter image";

enum constants
{
  DEFAULT_WINDOW_WIDTH = 640,
  DEFAULT_WINDOW_HEIGHT = 480,
};

typedef struct MyWindow MyWindow;
struct MyWindow
{
  SDL_Window *window;
  SDL_Renderer *renderer;
};

typedef struct MyImage MyImage;
struct MyImage
{
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_FRect rect;
};

//------------------------------------------------------------------------------
// Globals (argh!)
//------------------------------------------------------------------------------
static MyWindow g_window = { .window = NULL, .renderer = NULL };
static MyImage g_image = {
  .surface = NULL,
  .texture = NULL,
  .rect = { .x = 0.0f, .y = 0.0f, .w = 0.0f, .h = 0.0f }
};

//------------------------------------------------------------------------------
// Function declaration
//------------------------------------------------------------------------------
static bool MyWindow_initialize(MyWindow *window, const char *title, int width, int height, SDL_WindowFlags window_flags);
static void MyWindow_destroy(MyWindow *window);
static void MyImage_destroy(MyImage *image);
static bool MyImage_update_texture_with_surface(MyImage* image, SDL_Renderer *renderer, SDL_Surface *surface);
static bool MyImage_restore_texture(MyImage* image, SDL_Renderer *renderer);

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

/**
 * Carrega a imagem indicada no parâmetro `filename` e a converte para o formato
 * RGBA32, eliminando dependência do formato original da imagem. A imagem
 * carregada é armazenada em output_image.
 * Caso ocorra algum erro no processo, a função retorna false.
 */
static bool load_rgba32(const char *filename, SDL_Renderer *renderer, MyImage *output_image);

static void reset_image(void);

static SDL_AppResult initialize(void);
static void shutdown(void);
static void render(void);
static void loop(void);

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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyImage_destroy(MyImage *image)
{
  SDL_Log(">>> MyImage_destroy()");

  if (!image)
  {
    SDL_Log("\t*** Erro: Imagem inválida (image == NULL).");
    SDL_Log("<<< MyImage_destroy()");
    return;
  }

  if (image->texture)
  {
    SDL_Log("\tDestruindo MyImage->texture...");
    SDL_DestroyTexture(image->texture);
    image->texture = NULL;
  }

  if (image->surface)
  {
    SDL_Log("\tDestruindo MyImage->surface...");
    SDL_DestroySurface(image->surface);
    image->surface = NULL;
  }

  SDL_Log("\tRedefinindo MyImage->rect...");
  image->rect.x = image->rect.y = image->rect.w = image->rect.h = 0.0f;

  SDL_Log("<<< MyImage_destroy()");
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
bool MyImage_update_texture_with_surface(MyImage* image, SDL_Renderer *renderer, SDL_Surface *surface)
{
  SDL_Log(">>> MyImage_update_texture_with_surface()");

  if (!image)
  {
    SDL_Log("\t*** Erro: Imagem inválida (image == NULL).");
    SDL_Log("<<< MyImage_update_texture_with_surface()");
    return false;
  }

  if (!renderer)
  {
    SDL_Log("\t*** Erro: Renderer inválido (renderer == NULL).");
    SDL_Log("<<< MyImage_update_texture_with_surface()");
    return false;
  }

  if (!surface)
  {
    SDL_Log("\t*** Erro: Superfície inválida (surface == NULL).");
    SDL_Log("<<< MyImage_update_texture_with_surface()");
    return false;
  }

  SDL_DestroyTexture(image->texture);

  image->texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (!image->texture)
  {
    SDL_Log("\t*** Erro ao criar textura: %s", SDL_GetError());
    SDL_Log("<<< MyImage_update_texture_with_surface()");
    return false;
  }

  SDL_Log("\tObtendo dimensões da textura...");
  SDL_GetTextureSize(image->texture, &image->rect.w, &image->rect.h);

  SDL_Log("<<< MyImage_update_texture_with_surface()");
  return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool MyImage_restore_texture(MyImage* image, SDL_Renderer *renderer)
{
  SDL_Log(">>> MyImage_restore_texture()");
  
  if (!MyImage_update_texture_with_surface(image, renderer, image->surface))
  {
    SDL_Log("\t*** Erro ao restaurar a textura da imagem.");
    return false;
  }

  SDL_Log("<<< MyImage_restore_texture()");
  return true;  
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool load_rgba32(const char *filename, SDL_Renderer *renderer, MyImage *output_image)
{
  if (!filename)
  {
    SDL_Log(">>> load_rgba32(NULL)");
    SDL_Log("\t*** Erro: Nome do arquivo inválido (filename == NULL).");
    SDL_Log("<<< load_rgba32(NULL)");
    return false;
  }

  SDL_Log(">>> load_rgba32(\"%s\")", filename);

  if (!renderer)
  {
    SDL_Log("\t*** Erro: Renderer inválido (renderer == NULL).");
    SDL_Log("<<< load_rgba32(\"%s\")", filename);
    return false;
  }

  if (!output_image)
  {
    SDL_Log("\t*** Erro: Imagem de saída inválida (output_image == NULL).");
    SDL_Log("<<< load_rgba32(\"%s\")", filename);
    return false;
  }

  MyImage_destroy(output_image);

  SDL_Log("\tCarregando imagem \"%s\" em uma superfície...", filename);
  SDL_Surface *surface = IMG_Load(filename);
  if (!surface)
  {
    SDL_Log("\t*** Erro: formato de imagem inválido ou não suportado em %s (%s)", filename, SDL_GetError());
    SDL_Log("<<< load_rgba32(\"%s\")", filename);
    return false;
  }

  SDL_Log("\tConvertendo superfície para formato RGBA32...");
  output_image->surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
  SDL_DestroySurface(surface);
  if (!output_image->surface)
  {
    SDL_Log("\t*** Erro ao converter superfície para formato RGBA32: %s", SDL_GetError());
    SDL_Log("<<< load_rgba32(\"%s\")", filename);
    return false;
  }

  SDL_Log("\tCriando textura a partir da superfície...");
  if (!MyImage_update_texture_with_surface(output_image, renderer, output_image->surface))
  {
    SDL_Log("\t*** Erro ao criar textura.");
    SDL_Log("<<< load_rgba32(\"%s\")", filename);
    return false;
  }

  SDL_Log("<<< load_rgba32(\"%s\")", filename);
  return true;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
void reset_image(void)
{
  SDL_Log(">>> reset_image()");

  MyImage_restore_texture(&g_image, g_window.renderer);
  render();

  SDL_Log("<<< reset_image()");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SDL_AppResult initialize(void)
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
  if (!MyWindow_initialize(&g_window, WINDOW_TITLE, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, 0))
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
void shutdown(void)
{
  SDL_Log(">>> shutdown()");

  MyImage_destroy(&g_image);
  MyWindow_destroy(&g_window);

  SDL_Log("\tEncerrando SDL...");
  SDL_Quit();

  SDL_Log("<<< shutdown()");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void render(void)
{
  SDL_SetRenderDrawColor(g_window.renderer, 128, 128, 128, 255);
  SDL_RenderClear(g_window.renderer);

  SDL_RenderTexture(g_window.renderer, g_image.texture, &g_image.rect, &g_image.rect);

  SDL_RenderPresent(g_window.renderer);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void loop(void)
{
  SDL_Log(">>> loop()");

  render();

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
            case SDLK_0: reset_image(); break;
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

  atexit(shutdown);

  if (initialize() == SDL_APP_FAILURE)
    return SDL_APP_FAILURE;

  if (!load_rgba32(image_filename, g_window.renderer, &g_image))
    return SDL_APP_FAILURE;

  // Altera tamanho da janela se a imagem for maior do que o tamanho padrão
  // e reposiciona no canto superior esquerdo da tela.
  int imageWidth = (int)g_image.rect.w;
  int imageHeight = (int)g_image.rect.h;
  if (imageWidth > DEFAULT_WINDOW_WIDTH || imageHeight > DEFAULT_WINDOW_HEIGHT)
  {
    // Obtém o tamanho da borda da janela. Neste exemplo, só queremos saber
    // o lado superior e o lado esquerdo, para posicionar a janela corretamente
    // (posicionar a janela na coordenada (0, 0) faria com que a borda do
    // programa ficasse fora da região da tela).
    int top = 0;
    int left = 0;
    SDL_GetWindowBordersSize(g_window.window, &top, &left, NULL, NULL);

    SDL_Log("Redefinindo dimensões da janela, de (%d, %d) para (%d, %d), e alterando a posição para (%d, %d).",
      DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, imageWidth, imageHeight, left, top);

    SDL_SetWindowSize(g_window.window, imageWidth, imageHeight);
    SDL_SetWindowPosition(g_window.window, left, top);

    SDL_SyncWindow(g_window.window);
  }

  loop();

  return 0;
}
