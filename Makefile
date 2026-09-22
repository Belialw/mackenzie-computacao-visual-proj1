# ==============================================================================
# Projeto 1 - Computacao Visual (UPM FCI CC)
# Processamento de imagens em C com SDL3.
#
# Integrantes: ver README.md
#
# ------------------------------------------------------------------------------
# Uso
# ------------------------------------------------------------------------------
#   make            compila (padrao: build de release)
#   make DEBUG=1    compila com simbolos de depuracao e logs habilitados
#   make run        compila e executa (use IMG=... para escolher a imagem)
#   make clean      remove a pasta build/
#
# No Windows, com o MinGW/MSYS2, o comando e "mingw32-make" no lugar de "make".
#
# ------------------------------------------------------------------------------
# Dependencias
# ------------------------------------------------------------------------------
# SDL3, SDL3_image e SDL3_ttf.
#
# Windows: instalacao manual dos pacotes "*-devel-*-mingw.zip" (metodo adotado
#   na disciplina), com as tres bibliotecas mescladas em uma unica pasta. Se a
#   sua instalacao estiver em outro lugar, sobrescreva SDL_DIR:
#
#     mingw32-make SDL_DIR=c:/libs/SDL3
#
# Linux/WSL: pacotes de desenvolvimento das tres bibliotecas, localizados via
#   pkg-config (sdl3, sdl3-image, sdl3-ttf).
# ==============================================================================

# ------------------------------------------------------------------------------
# Shell usado pelas regras
#
# O GNU make escolhe sh.exe ou cmd.exe dependendo do terminal que iniciou a
# build: chamado pelo Git Bash ele encontra o sh.exe do Git no PATH; chamado
# pelo PowerShell ou pelo cmd ele cai no interpretador do Windows. Sem forcar,
# os comandos das regras (mkdir, copy, rmdir) teriam sintaxe valida em um
# terminal e invalida no outro. Precisa vir antes de qualquer $(shell ...).
# ------------------------------------------------------------------------------
ifeq ($(OS),Windows_NT)
  SHELL       := cmd.exe
  .SHELLFLAGS := /C
endif

TARGET    := imgproc
SRC_DIR   := src
BUILD_DIR := build

CC := gcc

# ------------------------------------------------------------------------------
# Padrao da linguagem
#
# O enunciado exige C99 ou mais recente e o repositorio da disciplina usa
# -std=c23. Porem o gcc so passou a aceitar o nome "c23" na versao 14: em
# versoes anteriores o mesmo padrao se chama "c2x". Sem esta deteccao, a build
# falha com "unrecognized command-line option" em qualquer gcc 13 ou anterior.
# ------------------------------------------------------------------------------
GCC_MAJOR := $(firstword $(subst ., ,$(shell $(CC) -dumpversion)))
ifeq ($(filter $(GCC_MAJOR),1 2 3 4 5 6 7 8 9 10 11 12 13),)
  CSTD := -std=c23
else
  CSTD := -std=c2x
endif

CFLAGS  := $(CSTD) -Wall -Wextra -Wpedantic
LDFLAGS :=
LDLIBS  :=

# ------------------------------------------------------------------------------
# Build de debug x build de release
#
# O codigo-base da disciplina registra em log praticamente toda chamada de
# funcao. Esses logs sao uteis durante o desenvolvimento e ruido para o usuario
# final, entao ficam restritos a build de debug (macro DEBUG).
# ------------------------------------------------------------------------------
DEBUG ?= 0
ifeq ($(DEBUG),1)
  CFLAGS += -g -O0 -DDEBUG
else
  CFLAGS += -O2 -DNDEBUG
endif

# ------------------------------------------------------------------------------
# Localizacao das bibliotecas SDL3, por plataforma
# ------------------------------------------------------------------------------
ifeq ($(OS),Windows_NT)
  SDL_DIR ?= d:/dev/compvis/libs/SDL3

  CFLAGS  += -I"$(SDL_DIR)/include"
  LDFLAGS += -L"$(SDL_DIR)/lib"
  LDLIBS  += -lSDL3 -lSDL3_image -lSDL3_ttf
  EXE     := .exe

  # Versoes dos caminhos com barra invertida, para os comandos do cmd.exe.
  SDL_DIR_WIN   := $(subst /,\,$(SDL_DIR))
  BUILD_DIR_WIN := $(subst /,\,$(BUILD_DIR))

  MKDIR_BUILD := if not exist "$(BUILD_DIR_WIN)" mkdir "$(BUILD_DIR_WIN)"
  RM_BUILD    := if exist "$(BUILD_DIR_WIN)" rmdir /s /q "$(BUILD_DIR_WIN)"
else
  SDL_PKGS := sdl3 sdl3-image sdl3-ttf

  CFLAGS += $(shell pkg-config --cflags $(SDL_PKGS))
  LDLIBS += $(shell pkg-config --libs $(SDL_PKGS)) -lm
  EXE    :=

  MKDIR_BUILD := mkdir -p $(BUILD_DIR)
  RM_BUILD    := rm -rf $(BUILD_DIR)
endif

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)
BIN  := $(BUILD_DIR)/$(TARGET)$(EXE)

# O cmd.exe nao aceita "build/imgproc.exe" como comando (interpreta a barra como
# inicio de uma opcao), entao o alvo "run" precisa do caminho com barra
# invertida no Windows.
ifeq ($(OS),Windows_NT)
  RUN_BIN := $(subst /,\,$(BIN))
else
  RUN_BIN := ./$(BIN)
endif

# Imagem usada por "make run". Sobrescreva com: make run IMG=caminho/imagem.png
IMG ?= samples/kodim23.png

.PHONY: all dlls assets run clean

ifeq ($(SRCS),)
all:
	@echo Nenhum arquivo .c encontrado em $(SRC_DIR)/.
	@exit 1
else
all: $(BIN) dlls assets
endif

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	@$(MKDIR_BUILD)

# ------------------------------------------------------------------------------
# No Windows o executavel precisa das DLLs da SDL ao lado dele. No Linux o
# carregador encontra as bibliotecas pelo sistema, entao a regra nao faz nada.
# ------------------------------------------------------------------------------
dlls: | $(BUILD_DIR)
ifeq ($(OS),Windows_NT)
	@copy /y "$(SDL_DIR_WIN)\bin\SDL3.dll" "$(BUILD_DIR_WIN)\SDL3.dll" >nul
	@copy /y "$(SDL_DIR_WIN)\bin\SDL3_image.dll" "$(BUILD_DIR_WIN)\SDL3_image.dll" >nul
	@copy /y "$(SDL_DIR_WIN)\bin\SDL3_ttf.dll" "$(BUILD_DIR_WIN)\SDL3_ttf.dll" >nul
endif


# ------------------------------------------------------------------------------
# A fonte usada nos textos e qualquer outro recurso sao copiados para junto do
# executavel. O programa monta o caminho da fonte a partir de SDL_GetBasePath(),
# que devolve o diretorio do proprio executavel, e nao do diretorio de trabalho.
# Assim o programa encontra a fonte de onde quer que seja chamado.
# ------------------------------------------------------------------------------
assets: | $(BUILD_DIR)
ifeq ($(OS),Windows_NT)
	@xcopy /e /i /y /q "assets" "$(BUILD_DIR_WIN)\assets" >nul
else
	@cp -r assets $(BUILD_DIR)/
endif

run: all
	$(RUN_BIN) $(IMG)

clean:
	@$(RM_BUILD)

-include $(DEPS)
