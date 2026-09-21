# Projeto 1 — Processamento de imagens em C com SDL3

Universidade Presbiteriana Mackenzie — Faculdade de Computação e Informática
Ciência da Computação — **Computação Visual** — Prof. André Kishimoto

---

## Integrantes

| Nome completo | RA |
| --- | --- |
| Enzo Ponte Gamberi | 10389931 |
| Luís Henrique Ribeiro Fernandes | 10420046 |
| Raphael Grizante da Silva | 10416979 |
| Vinícius Brait Lorimier | 10420046 |

---

## Sobre o projeto

Programa de linha de comando que carrega uma imagem, converte para escala de
cinza e permite analisá-la e processá-la por meio de uma interface gráfica de
duas janelas construída com a biblioteca SDL3.

```
imgproc caminho_da_imagem.ext
```

Funcionalidades previstas:

1. Carregamento de imagem com tratamento de erros (`SDL3_image`).
2. Detecção de imagem colorida e conversão para escala de cinza.
3. Interface gráfica com janela principal e janela secundária (filha).
4. Cálculo e exibição do histograma, com média de intensidade e desvio padrão.
5. Equalização do histograma, reversível para a imagem original.
6. Alternância entre a resolução original da imagem e 1024x768.
7. Salvamento da imagem exibida em `output_image.png` (tecla `S`).
8. Exibição de textos com fonte embarcada no projeto.

O projeto parte do código-base disponibilizado no repositório da disciplina
([profkishimoto/CompVis262](https://github.com/profkishimoto/CompVis262)),
especificamente do exemplo `src/05-filter_image`.

---

## Ambiente de desenvolvimento

| Item | Versão |
| --- | --- |
| Sistema operacional | Windows 11 Pro (10.0.26200) |
| Compilador | gcc 15.2.0 (MSYS2 UCRT64) |
| Editor | Visual Studio Code |
| SDL3 | 3.4.16 |
| SDL3_image | 3.4.6 |
| SDL3_ttf | 3.2.2 |

---

## Compilação e execução

_A ser detalhado no commit do Makefile._

---

## Estrutura do repositório

```
.
├── src/          código-fonte C
├── assets/       recursos embarcados (fonte usada nos textos)
├── docs/         documentação e diário de desenvolvimento
└── Makefile      build para Windows (MinGW) e Linux/WSL
```

---

## Decisões de implementação

_A ser preenchido: limiares de classificação de brilho e contraste, fonte
escolhida, formato da imagem salva._

---

## Contribuições

_A ser preenchido._
