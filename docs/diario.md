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

---

## 2026-09-21 — Makefile multiplataforma

O `makefile` dos exemplos da disciplina não serve para a entrega como está: o
`SDL_DIR` é um caminho absoluto fixo e as regras `clean` e de cópia das DLLs
usam `del` e `copy`, que só existem no interpretador do Windows. Como o
professor compila o projeto tanto no Windows quanto no WSL Ubuntu, um Makefile
que só funciona em um dos dois é risco de "projeto não compila", que zera a
atividade.

Foi escrito um Makefile único que detecta a plataforma: no Windows usa
`SDL_DIR` (sobrescrevível na linha de comando) e copia as DLLs para `build/`;
no Linux usa `pkg-config` e acrescenta `-lm`.

**Problema encontrado: o `make` não usa sempre o mesmo shell no Windows.**
Medindo o valor de `$(SHELL)`, o `mingw32-make` reportou
`C:/Program Files/Git/usr/bin/sh.exe` quando chamado pelo Git Bash e `sh.exe`
(caindo para o `cmd.exe`) quando chamado pelo PowerShell. Ou seja, o mesmo
Makefile executaria `mkdir -p build` em um terminal e precisaria de
`if not exist build mkdir build` no outro. Solução: forçar `SHELL := cmd.exe`
e `.SHELLFLAGS := /C` no ramo Windows. A definição precisa aparecer antes de
qualquer `$(shell ...)` do arquivo, senão a expansão acontece com o shell
errado.

**Problema encontrado: `-std=c23` não existe antes do gcc 14.** O repositório
da disciplina compila com `-std=c23` e o professor usa gcc 15.x, mas o gcc
13.3 do WSL desta máquina recusa a opção com
`unrecognized command-line option '-std=c23'; did you mean '-std=c2x'?`. O
Makefile passou a consultar `gcc -dumpversion` e a escolher entre `c23` e
`c2x` conforme a versão principal do compilador.

**Validação.** Com um projeto de teste de três arquivos (`main.c`, `util.c`,
`util.h`) que inclui e chama as três bibliotecas:

| Ambiente | Resultado |
| --- | --- |
| Git Bash + gcc 15.2.0 | compila com `-std=c23`, gera `build/imgproc.exe`, executa |
| PowerShell + gcc 15.2.0 | saída idêntica à do Git Bash |
| WSL Ubuntu + gcc 13.3.0 | seleciona `-std=c2x`, alvo sem `.exe`, acrescenta `-lm` |

Também foram verificados: recompilação incremental correta ao alterar um
cabeçalho (via `-MMD -MP`), `make clean`, `make DEBUG=1` alternando os flags e
definindo a macro `DEBUG`, e o override `SDL_DIR=` refletindo tanto nos `-I/-L`
quanto no caminho de cópia das DLLs.

---

## 2026-09-21 — Importação do código-base da disciplina

Entre os exemplos do repositório da disciplina, o escolhido como base foi o
`src/05-filter_image`. É o que mais se aproxima do que o projeto pede: já
carrega a imagem com `IMG_Load` e a converte para `SDL_PIXELFORMAT_RGBA32`,
percorre os pixels com `SDL_LockSurface` / `SDL_GetRGB` / `SDL_MapRGB` (molde
para a conversão em escala de cinza e para a equalização), atualiza a textura a
partir de uma surface processada, restaura a imagem original sem recarregar o
arquivo do disco, e redimensiona e reposiciona a janela usando
`SDL_GetWindowBordersSize` — que é exatamente o que o item 6 do escopo exige.

O arquivo foi copiado **sem nenhuma alteração** (conferido por hash), junto com
a imagem de teste `kodim23.png`. A ideia é que todo o trabalho do grupo apareça
como diferença em relação a este commit, o que torna o histórico do repositório
a resposta direta para a pergunta do relatório sobre o que foi refatorado no
código original.

O código compilou pelo Makefile do projeto sem erros. Os dois únicos avisos são
`unused parameter 'argc'` e `unused parameter 'argv'`, ou seja, o próprio
compilador aponta a refatoração que o professor sugere no cabeçalho do arquivo:
receber o caminho da imagem por parâmetro no lugar da constante
`IMAGE_FILENAME`.

Execução verificada: o programa carregou `kodim23.png`, redimensionou a janela
de 640x480 para 768x512 e entrou no loop de eventos. Como o nome do arquivo
ainda é uma constante relativa ao diretório atual, o programa só encontra a
imagem se for executado de dentro de `samples/`. Isso deixa de valer no próximo
commit.

### Refatorações previstas

O cabeçalho do arquivo original lista as refatorações que o próprio autor
considera necessárias em um projeto real, e elas serão feitas em commits
separados:

1. separar o código em headers `.h` e arquivos `.c`;
2. remover as variáveis globais;
3. reduzir os logs, desativando-os na build de release;
4. receber o arquivo de imagem por `argv` no lugar da constante.

---

## 2026-09-21 — Refatoração 4/4: caminho da imagem por argv

Primeira das quatro refatorações listadas no cabeçalho do arquivo original, e
também o item 1 do escopo obrigatório. A constante `IMAGE_FILENAME` foi
removida e o programa passou a receber o caminho da imagem como argumento,
conforme a chamada definida no enunciado.

Foram acrescentadas duas funções: `print_usage()`, que mostra como chamar o
programa, e `check_image_path()`, que valida o caminho antes de qualquer
inicialização da SDL. A validação usa `SDL_GetPathInfo()` para distinguir
"arquivo não existe" de "o caminho é um diretório", o que permite mensagens de
erro mais específicas. Se o arquivo existe mas não é uma imagem em formato
suportado, quem detecta é o `IMG_Load()`, e a mensagem de erro correspondente
em `load_rgba32()` foi reescrita para deixar isso claro.

A validação dos argumentos acontece antes do `atexit(shutdown)` e antes do
`SDL_Init()`: não faz sentido inicializar subsistema de vídeo para descobrir em
seguida que o argumento estava errado.

**Bug encontrado no código-base.** Ao recompilar, o gcc acusou:

```
src/main.c:275:32: warning: '%s' directive argument is null [-Wformat-overflow=]
```

Dentro do bloco `if (!filename)` de `load_rgba32()`, a variável `filename` é
comprovadamente `NULL` e ainda assim era passada para um `%s` do `SDL_Log()`.
Passar `NULL` para `%s` é comportamento indefinido em C. O mesmo vale para o
log de entrada da função, que acontecia antes da verificação. A correção foi
mover a verificação de `NULL` para antes do log de entrada e usar texto literal
nas mensagens desse caminho de erro. A build voltou a ficar sem nenhum aviso
com `-Wall -Wextra -Wpedantic`.

**Bug encontrado no nosso Makefile.** O alvo `run` falhava com
`'build' não é reconhecido como um comando interno ou externo`. O `cmd.exe`
interpreta a barra normal no início de um comando como início de uma opção,
então `build/imgproc.exe` não é reconhecido como executável. Foi acrescentada a
variável `RUN_BIN`, que converte o caminho para barras invertidas no Windows.
Só apareceu porque o alvo foi realmente executado — vale testar cada alvo do
Makefile, não só o `all`.

**Testes dos caminhos de erro** (todos com mensagem no terminal e saída
diferente de zero):

| Entrada | Resultado |
| --- | --- |
| sem argumento | erro + modo de uso |
| dois argumentos | erro + modo de uso |
| `nao_existe.png` | "arquivo não encontrado" |
| `samples` (diretório) | "é um diretório, não um arquivo de imagem" |
| arquivo de texto renomeado para `.png` | "formato de imagem inválido ou não suportado" |
| `samples/kodim23.png` | carrega e exibe corretamente |

No último caso de erro o `shutdown()` roda normalmente, liberando o que já
tinha sido alocado antes da falha.
