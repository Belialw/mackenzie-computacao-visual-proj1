// Copyright (c) 2026 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Projeto 1 - Computação Visual (UPM FCI CC)
// Integrantes: ver README.md
//
// Cálculo do histograma da imagem em escala de cinza.
//------------------------------------------------------------------------------

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <stdbool.h>
#include <SDL3/SDL.h>

#include "text.h"

enum
{
  // Um nível para cada intensidade possível em 8 bits.
  HISTOGRAM_LEVELS = 256,
};

/**
 * Distribuição das intensidades da imagem.
 *
 * `counts[i]` é a quantidade de pixels com intensidade `i`. `max_count` é a
 * maior dessas quantidades e serve de referência para desenhar o gráfico em
 * proporção à altura disponível. `total_pixels` é a soma de todas elas.
 */
typedef struct Histogram Histogram;
struct Histogram
{
  int counts[HISTOGRAM_LEVELS];
  int max_count;
  int total_pixels;

  // Média das intensidades e desvio padrão em relação a ela, ambos em níveis
  // de intensidade (0 a 255).
  float mean;
  float stddev;
};

/**
 * Calcula o histograma de `surface`, que deve estar em escala de cinza e no
 * formato RGBA32. Como a imagem já está em escala de cinza, os três canais de
 * cor têm o mesmo valor e a intensidade de cada pixel é lida de um deles.
 *
 * Retorna false, sem alterar `histogram`, caso os parâmetros sejam inválidos.
 */
bool Histogram_compute(Histogram *histogram, SDL_Surface *surface);


/**
 * Desenha o gráfico do histograma dentro de `area`, que deve ter exatamente
 * HISTOGRAM_LEVELS pixels de largura para que cada nível ocupe uma coluna de
 * um pixel, sem barras de espessura irregular por arredondamento.
 *
 * A altura de cada barra é proporcional ao nível mais frequente, de modo que a
 * barra mais alta sempre ocupe toda a altura da área, independentemente do
 * tamanho da imagem.
 *
 * `text` pode ser nulo, caso em que os rótulos do eixo não são desenhados.
 */
void Histogram_draw(const Histogram *histogram, SDL_Renderer *renderer, const SDL_FRect *area, const TextRenderer *text);


/**
 * Classificação da imagem a partir da média de intensidade: "escura", "média"
 * ou "clara". Os limiares adotados e a justificativa estão no README.md.
 */
const char *Histogram_brightness_label(const Histogram *histogram);

/**
 * Classificação do contraste a partir do desvio padrão: "baixo", "médio" ou
 * "alto". Os limiares adotados e a justificativa estão no README.md.
 */
const char *Histogram_contrast_label(const Histogram *histogram);


/**
 * Calcula a tabela de mapeamento de intensidades da equalização de histograma:
 * `mapping[i]` é a nova intensidade dos pixels que hoje têm intensidade `i`.
 *
 * A transformação é a função de distribuição acumulada normalizada:
 *
 *   mapping[i] = round( (cdf[i] - cdf_min) / (total - cdf_min) * 255 )
 *
 * em que `cdf[i]` é a soma das contagens até o nível `i` e `cdf_min` é o
 * primeiro valor acumulado diferente de zero. Subtrair `cdf_min` faz o nível
 * mais escuro presente na imagem ser mapeado para 0, usando toda a faixa
 * disponível.
 *
 * Quando a imagem inteira tem uma só intensidade, não há faixa para espalhar e
 * a tabela devolvida é a identidade, o que deixa a imagem intacta.
 *
 * O cálculo é separado da aplicação de propósito: esta função lida apenas com
 * o histograma, e quem percorre os pixels é o módulo de imagem.
 */
bool Histogram_equalization_mapping(const Histogram *histogram, Uint8 *mapping);

#endif // HISTOGRAM_H
