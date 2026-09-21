# Diário de desenvolvimento

Registro contínuo dos problemas encontrados, das decisões tomadas e das
consultas a referências externas durante o desenvolvimento do Projeto 1.

Este arquivo existe porque o relatório da Etapa 2 pergunta, entre outras coisas:

- quais problemas foram encontrados e como foram solucionados;
- se foi necessário consultar referências extras ou IA generativa em itens que a
  análise inicial classificou como "resolvíveis apenas com o material da
  disciplina";
- como a IA generativa ajudou no desenvolvimento;
- quais assuntos precisam ser mais estudados.

Reconstruir essas respostas no final, de memória, sai pior do que anotar no dia.

---

## 2026-09-20 — Preparação do ambiente

**Instalação das bibliotecas SDL3.**
Adotado o método descrito no material da disciplina (`docs/hello/README.md` do
repositório do professor): download dos pacotes `*-devel-*-mingw.zip` dos
releases oficiais, extração apenas da variante `x86_64-w64-mingw32` e mesclagem
das três bibliotecas em uma única pasta.

Versões instaladas (mais recentes estáveis no momento):

| Biblioteca | Versão da disciplina | Versão instalada |
| --- | --- | --- |
| SDL3 | 3.4.0 | 3.4.16 |
| SDL3_image | — | 3.4.6 |
| SDL3_ttf | — | 3.2.2 |

Local da instalação: `D:\dev\compvis\libs\SDL3`, com `include/`, `lib/`, `bin/`
e `share/` das três bibliotecas mesclados. Foi escolhido exatamente o mesmo
caminho usado pelo professor (visível no `c_cpp_properties.json` do repositório
da disciplina) para que o `makefile` dos exemplos funcione sem alteração.

**Validação.** Um programa de teste que inclui e inicializa as três bibliotecas
compilou com `-std=c23 -Wall -Wextra -Wpedantic` sem nenhum warning e executou
corretamente, reportando as versões em runtime, `SDL_Init(SDL_INIT_VIDEO)` e
`TTF_Init()` bem-sucedidos. O exemplo `05-filter_image` do professor também
compilou e linkou sem alterações.

**Problema encontrado: SDL3_ttf não faz parte do material da disciplina.**
Uma busca por "ttf" e "font" em todo o repositório do professor não retorna
nenhuma ocorrência — não há exemplo nem instruções de instalação. O enunciado
apenas *sugere* a biblioteca e lista como obrigatórias somente SDL3 e
SDL_image. Como "projeto não compila" zera a atividade, a instalação do
SDL3_ttf precisa estar documentada em detalhe no README, e está prevista uma
build alternativa sem SDL3_ttf como proteção.

**Problema encontrado: o `make` escolhe shells diferentes no Windows.**
Ao testar o build, o `mingw32-make` reportou `SHELL=C:/Program Files/Git/usr/bin/sh.exe`
quando invocado pelo Git Bash e `SHELL=sh.exe` (caindo para o `cmd.exe`) quando
invocado pelo PowerShell. Isso significa que comandos das regras do Makefile
(`mkdir`, `copy`, `rm`) teriam sintaxe válida em um terminal e inválida em
outro. Solução adotada: forçar `SHELL := cmd.exe` e `.SHELLFLAGS := /C` no ramo
Windows do Makefile, tornando a build determinística independentemente do
terminal usado. Verificado funcionando a partir dos dois terminais.

**Observação sobre testes.** O monitor de desenvolvimento tem 3440x1392 de área
útil. O item 6 do escopo exige posicionar a janela principal em (0,0) quando a
imagem *excede* a resolução da tela — com essa largura, o caminho só é
exercitado por imagens muito grandes. É preciso separar uma imagem de teste
acima de 3440 pixels de largura para validar esse requisito.
