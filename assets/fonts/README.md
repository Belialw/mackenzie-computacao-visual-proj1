# Fonte usada nos textos do programa

**DejaVu Sans** (arquivo `DejaVuSans.ttf`, versão 2.37).

A fonte é distribuída junto com o código, e não carregada do sistema
operacional, para que o programa funcione da mesma forma em qualquer máquina —
exigência do item 8 do escopo do projeto. Um caminho como
`C:\Windows\Fonts\arial.ttf` não existiria no Linux, e o nome e o local das
fontes do sistema variam entre distribuições.

A escolha considerou:

- **Licença permissiva**, que autoriza a redistribuição junto com o projeto
  (ver `LICENSE-DejaVu.txt`).
- **Cobertura dos caracteres acentuados do português**, já que as mensagens do
  programa usam acentuação (à, á, ã, ç, é, ê, í, ó, õ, ú).
- **Legibilidade em tamanhos pequenos**, necessária para as informações de
  análise exibidas na janela secundária.

O `Makefile` copia o conteúdo de `assets/` para junto do executável, em
`build/assets/`, e o programa monta o caminho da fonte a partir de
`SDL_GetBasePath()`, que devolve o diretório do próprio executável.
