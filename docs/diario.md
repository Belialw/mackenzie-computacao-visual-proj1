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

---

## 2026-09-21 — Remoção do filtro de média e adaptação do cabeçalho

O exemplo `05-filter_image` foi escolhido como base pela infraestrutura de
carregamento e manipulação de pixels, não pelo filtro em si. O filtro de média
é a funcionalidade de demonstração do exemplo e não aparece em nenhum item do
escopo obrigatório do projeto.

Manter esse código até as próximas refatorações significaria adaptá-lo à nova
estrutura de módulos e à remoção de variáveis globais para só então apagá-lo.
Por isso a remoção veio antes: `MyImage_blur()` e tudo que existia apenas para
atendê-lo saiu do arquivo, a saber, a superfície de trabalho `surfaceFilter`,
os dois cursores de mouse (que existiam só para indicar o processamento em
andamento durante a filtragem), as teclas `1` a `9` e as respectivas limpezas
no `shutdown()`.

O que foi preservado é justamente o que o projeto vai usar: `load_rgba32()`,
`MyImage_update_texture_with_surface()`, `MyImage_restore_texture()` e a lógica
de redimensionar e reposicionar a janela com `SDL_GetWindowBordersSize()`. As
teclas `0` e `R`, que restauram a imagem original a partir da surface em
memória, também ficaram: é exatamente o mecanismo que o item 5 do escopo exige
para reverter a equalização sem recarregar o arquivo.

O cabeçalho do arquivo, que descrevia o exemplo, foi substituído pela descrição
do projeto e pela identificação do grupo. A linha de copyright e o
identificador de licença do autor original foram mantidos, com a indicação
explícita de qual exemplo deu origem ao código.

Resultado: de 667 para 511 linhas, build sem nenhum aviso e execução
verificada.

---

## 2026-09-21 — Refatoração 1/4: separação em headers e arquivos .c

Segunda das quatro refatorações listadas no cabeçalho do arquivo original. O
programa inteiro vivia em um único `main.c`, e o próprio autor aponta o uso de
headers e arquivos `.c` separados como o primeiro item a mudar em um projeto
real.

A divisão adotada:

| Arquivo | Conteúdo |
| --- | --- |
| `src/window.h` / `.c` | `MyWindow` e o ciclo de vida da janela e do renderizador |
| `src/image.h` / `.c` | `MyImage`, carregamento do arquivo e manutenção da textura |
| `src/main.c` | estado do programa, validação dos argumentos, ciclo de vida da aplicação e loop de eventos |

O critério da separação foi a responsabilidade: `window` cuida do que aparece na
tela como janela, `image` cuida do que é a imagem em memória, e `main` costura
os dois. Os módulos `window` e `image` não se conhecem — só o `main.c` inclui
os dois —, o que evita dependência circular e permite compilar e testar cada um
isoladamente.

As funções movidas deixaram de ser `static`: antes os protótipos eram
declarados `static` no topo do arquivo e as definições apareciam sem o
qualificador, o que só funcionava por estarem na mesma unidade de compilação.
Agora os protótipos estão nos headers, com documentação de cada função, e as
definições continuam nos `.c`.

Para não introduzir erro de transcrição, os corpos das funções foram extraídos
do arquivo original por faixa de linhas em vez de copiados manualmente. O
comportamento do programa é idêntico ao anterior, verificado nos casos de erro
e no caminho normal.

Distribuição depois da separação: `main.c` caiu de 511 para 286 linhas, contra
50 em `window.c` e 169 em `image.c`. O Makefile não precisou de alteração — o
`$(wildcard $(SRC_DIR)/*.c)` já compila e linka os novos arquivos
automaticamente, e o rastreio de dependências por `-MMD -MP` passou a
recompilar os `.c` corretos quando um header muda.

---

## 2026-09-22 — Refatoração 2/4: remoção das variáveis globais

Terceira das quatro refatorações listadas no cabeçalho do arquivo original, que
marca as globais com o comentário `// Globals (argh!)`.

As duas globais, `g_window` e `g_image`, foram substituídas por uma struct
`App` que agrega as duas e é criada como variável local de `main()`. As funções
`initialize()`, `shutdown()`, `render()`, `loop()` e `reset_image()` passaram a
receber `App *` por parâmetro. A `render()` recebe `const App *`, já que só lê o
estado — o que o compilador agora garante.

**Ponto delicado: o `atexit()`.** O código original registrava
`atexit(shutdown)`, e `atexit()` só aceita função sem parâmetros. Havia duas
saídas: manter um ponteiro estático para o `App` só para o `shutdown()`
enxergar, ou abandonar o `atexit()` e chamar a limpeza explicitamente. A
primeira opção reintroduziria pela porta dos fundos exatamente o estado global
que a refatoração queria eliminar, então foi adotada a segunda. Hoje o
`shutdown(&app)` é chamado nos três pontos de saída depois da inicialização.

Isso é seguro mesmo quando a inicialização falha no meio: o `App` é criado com
todos os ponteiros nulos, e tanto `MyWindow_destroy()` quanto
`MyImage_destroy()` tratam ponteiro nulo, assim como o `SDL_DestroyWindow()` e
o `SDL_DestroySurface()` da própria SDL. Verificado com um arquivo inválido: a
falha acontece depois do `SDL_Init()`, o `shutdown()` roda e encerra a SDL
normalmente.

Aproveitando a passagem, os códigos de retorno de `main()` foram uniformizados.
O código original devolvia `SDL_APP_FAILURE` — um valor de enum da SDL, pensado
para o retorno das callbacks de `SDL_AppInit`/`SDL_AppIterate`, não para o
código de saída de um processo. Agora todas as saídas usam `EXIT_SUCCESS` ou
`EXIT_FAILURE`. Na prática, a saída por imagem inválida passou de 2 para 1.

---

## 2026-09-22 — Refatoração 3/4: redução dos logs

Última das quatro refatorações listadas no cabeçalho do arquivo original, que
pede a "redução de logs (ou melhor, seriam desativados na build release)".

O ponto de partida eram 70 chamadas de `SDL_Log()` espalhadas pelos três
arquivos, registrando a entrada e a saída de praticamente toda função. Apagar
tudo seria perder uma ferramenta de depuração útil; manter tudo é ruído para
quem usa o programa. Pior: o enunciado exige que mensagens específicas apareçam
no terminal — se a imagem é colorida ou está em escala de cinza, e o resultado
do salvamento — e elas se perderiam no meio do rastreamento.

A solução foi separar os dois tipos de mensagem em `src/log.h`:

| Macro | Build de debug | Build de release | Uso |
| --- | --- | --- | --- |
| `LOG_DEBUG` | aparece | não aparece | rastreamento interno |
| `LOG_INFO` | aparece | aparece | informação ao usuário |
| `LOG_ERROR` | aparece | aparece | erro ao usuário |

Distribuição das 70 chamadas: 47 viraram `LOG_DEBUG`, 2 `LOG_INFO` (o modo de
uso) e 21 `LOG_ERROR`.

**Detalhe do `LOG_DEBUG` na build de release.** A definição não é um
`((void)0)` puro, e sim `do { if (0) SDL_Log(__VA_ARGS__); } while (0)`. Assim
o compilador continua analisando a chamada — verifica a string de formato
contra os argumentos e continua considerando as variáveis como usadas — mas
descarta o código na otimização. Com `((void)0)`, um erro de formato só
apareceria em quem compilasse com `DEBUG=1`, e variáveis usadas apenas em log
passariam a gerar aviso de não utilizadas.

**Limpeza das mensagens de erro.** O `LOG_ERROR` usa `SDL_LogError()`, e a SDL
já prefixa a linha com `ERROR:`. Como as mensagens do código-base começavam com
`*** Erro:`, a saída ficava `ERROR: *** Erro: ...`. O prefixo redundante foi
removido das 18 mensagens, junto com os `\t` de indentação que só faziam
sentido no rastreamento hierárquico.

Resultado, com a mesma imagem e o mesmo binário de origem:

| Situação | Release | Debug |
| --- | --- | --- |
| imagem válida | nenhuma linha | 18 linhas de rastreamento |
| arquivo inexistente | `ERROR: Arquivo não encontrado: nada.png` | idem + rastreamento |
| formato inválido | `ERROR: Formato de imagem inválido ou não suportado em ...` | idem + rastreamento |

Com isso a Fase 1 se encerra: as quatro refatorações que o autor do código-base
listou como necessárias em um projeto real estão feitas, cada uma em seu
próprio commit.

---

## 2026-09-22 — Item 2: detecção e conversão para escala de cinza

Primeira funcionalidade nova do projeto, e o primeiro item do escopo que não
vinha de graça com o código-base. Duas funções novas no módulo `image`:

`MyImage_is_grayscale()` percorre os pixels e devolve falso no primeiro em que
R, G e B não sejam iguais. O canal alpha não entra na comparação: uma imagem
cinza com transparência continua sendo cinza.

`MyImage_to_grayscale()` aplica `Y = 0.2125*R + 0.7154*G + 0.0721*B` — a
fórmula exata do enunciado, e não a 0.299/0.587/0.114 que aparece na maioria
dos materiais sobre o assunto. Os três pesos somam exatamente 1,0, então o
resultado nunca passa de 255 e não precisa ser limitado. O arredondamento usa
`SDL_roundf()`; truncar puxaria a imagem inteira meio nível para baixo.

**Conversão no lugar, e não em uma segunda surface.** A conversão altera a
própria `image->surface`. Assim a imagem em escala de cinza passa a ser a base
de tudo que vem depois, como o enunciado pede, e `MyImage_restore_texture()`
passa a devolver exatamente a imagem em escala de cinza — que é o
comportamento exigido no item 5 para desfazer a equalização sem recarregar o
arquivo do disco.

**Varredura por pitch.** O código-base calcula o índice do pixel como
`row * surface->w + col`, o que só vale se cada linha ocupar exatamente
`w * 4` bytes. A SDL pode alinhar as linhas e inserir bytes de preenchimento no
fim de cada uma, caso em que esse cálculo lê o pixel errado. As duas funções
novas usam `surface->pitch`, que é a distância real em bytes entre linhas. Para
o formato RGBA32 os dois cálculos coincidem na prática, mas o segundo não
depende disso.

**Alpha preservado.** A conversão usa `SDL_GetRGBA`/`SDL_MapRGBA` em vez de
`SDL_GetRGB`/`SDL_MapRGB`, mantendo a transparência original de cada pixel.

### Verificação numérica

Foi escrito um teste que chama `MyImage_to_grayscale()` com cores conhecidas e
compara com o valor esperado pela fórmula:

| Entrada | Esperado | Obtido |
| --- | --- | --- |
| vermelho puro (255, 0, 0) | 54 | 54 |
| verde puro (0, 255, 0) | 182 | 182 |
| azul puro (0, 0, 255) | 18 | 18 |
| branco (255, 255, 255) | 255 | 255 |
| preto (0, 0, 0) | 0 | 0 |
| cinza médio (128, 128, 128) | 128 | 128 |
| (10, 20, 30) com alpha 128 | 19, alpha 128 | 19, alpha 128 |

A detecção também foi verificada nos dois sentidos: a mesma imagem é reportada
como colorida antes da conversão e como cinza depois.

Para exercitar o caminho da imagem que já chega em escala de cinza, foi gerado
`samples/gray_gradient.png`, um gradiente 256x128 com R = G = B em todos os
pixels. Com ele o programa informa que a imagem já está em escala de cinza e
não converte nada.

---

## 2026-09-22 — Item 3 (parte 1): janela principal 1024x768 centralizada

A janela do código-base tinha 640x480 e, logo depois de carregar a imagem, era
redimensionada para o tamanho da imagem e jogada no canto superior esquerdo. O
enunciado pede o contrário: a janela principal deve **iniciar** com 1024x768 e
centralizada no monitor principal. O redimensionamento para o tamanho da imagem
não desapareceu do projeto — ele passa a ser um dos dois estados do botão do
item 6, e volta lá.

**Evitando o salto da janela.** A janela é criada com `SDL_WINDOW_HIDDEN`,
posicionada e só então exibida com `SDL_ShowWindow()`. Criada visível, ela
apareceria na posição padrão do sistema e saltaria para o centro no quadro
seguinte.

**Monitor principal, não o primeiro da lista.** A centralização usa
`SDL_WINDOWPOS_CENTERED_DISPLAY(SDL_GetPrimaryDisplay())`. Um
`SDL_WINDOWPOS_CENTERED` simples equivale ao display de índice 0, que nem
sempre é o monitor principal em um sistema com mais de um monitor.

**Desenho da imagem.** O código-base desenhava a textura com o retângulo de
origem igual ao de destino, ou seja, tamanho natural no canto da janela. Agora
`compute_image_destination()` calcula a maior escala que cabe na janela sem
distorcer as proporções e centraliza o resultado, e o que sobra fica como
margem. A interpretação adotada para "exibir a imagem na resolução 1024x768" é
essa: a imagem ocupa a maior área possível da janela mantendo as proporções,
em vez de ser esticada até 1024x768 exatos, o que a deformaria sempre que a
proporção da imagem fosse diferente de 4:3.

**Verificação.** Com um monitor de área útil 3440x1392, a janela foi reportada
como 1024x768 na posição (1208, 312) — exatamente `(3440-1024)/2` e
`(1392-768)/2`. Uma captura de tela confirmou a imagem em escala de cinza,
escalada para 1024 de largura, centralizada verticalmente e com as margens
ocupando o resto.

Foi acrescentada a macro `DEBUG_ENABLED` em `log.h`, que permite escrever
blocos de diagnóstico sem espalhar `#ifdef` pelo meio do código: o compilador
analisa o bloco nas duas builds e o descarta na otimização quando o valor é
falso.

---

## 2026-09-22 — Item 3 (parte 2): janela secundária filha, em (0,0)

Esta é a parte do projeto sem nenhum exemplo no material da disciplina: todos
os exemplos do professor usam uma única janela.

A `struct App` passou a ter `main_window` e `secondary_window`, e o vínculo de
parentesco é feito com `SDL_SetWindowParent()`. Além de atender à exigência,
isso faz a secundária acompanhar a principal ao minimizar e permanecer à frente
dela.

**Tamanho escolhido: 420x560.** O histograma tem 256 níveis e será desenhado a
um pixel por nível, então a área do gráfico tem 256 pixels de largura, o que
evita barras de espessura irregular por arredondamento. Com margem dos dois
lados sobra espaço para as linhas de informação da análise e para os dois
botões, um abaixo do outro.

**O (0,0) literal não funciona.** Posicionar a janela em (0, 0) e capturar a
tela mostrou a janela sem barra de título: `SDL_SetWindowPosition()` posiciona
a área de cliente, então a decoração fica acima da borda da tela. Na prática a
janela ficava sem título visível e sem como ser arrastada ou fechada pelo
botão. A correção consulta `SDL_GetWindowBordersSize()` e desloca a janela pela
espessura da borda, que neste sistema é 3 à esquerda e 26 no topo. A janela
inteira passa a encostar no canto superior esquerdo, que é o que a exigência
descreve. O próprio exemplo `05-filter_image` faz esse ajuste, com um
comentário explicando o mesmo problema — só que para a janela principal.

Vale registrar que `SDL_GetWindowBordersSize()` devolveu valores corretos mesmo
com a janela ainda oculta, o que permitiu posicionar antes de exibir e evitar
que a janela aparecesse no lugar errado e saltasse em seguida.

**Detalhe da documentação da SDL.** Uma janela filha oculta junto com o pai é
reexibida automaticamente quando o pai reaparece, mas isso só vale para janelas
que não tiveram o estado oculto definido explicitamente. Como a secundária é
criada com `SDL_WINDOW_HIDDEN` de propósito, ela precisa do seu próprio
`SDL_ShowWindow()`.

**Loop de eventos.** Com duas janelas, cada evento passou a ser encaminhado
pela janela de origem, identificada por `event.window.windowID`. O `render()`
foi dividido em `render_main()` e `render_secondary()`, e `SDL_EVENT_WINDOW_EXPOSED`
redesenha apenas a janela que precisou ser reexibida. Essa separação foi feita
agora de propósito: fazê-la depois, com histograma e botões já implementados,
custaria muito mais.

Fechar qualquer uma das duas janelas encerra o programa, já que a secundária
concentra os controles e deixá-la fechada tornaria a equalização e a troca de
resolução inacessíveis.

**Verificação.** Janela principal 1024x768 em (1208, 312) e secundária 420x560
em (3, 26). Uma captura de tela do canto superior esquerdo confirmou a janela
secundária inteira, com barra de título, botões de minimizar, maximizar e
fechar, e a área reservada ao gráfico do histograma delimitada.

---

## 2026-09-22 — Item 8: carregamento da fonte

O item 8 do escopo pede que "o código garanta que a fonte usada pelos textos do
programa é carregada e usada corretamente, independentemente do sistema
operacional em que o programa for executado". São duas exigências embutidas
nisso, e as duas têm a mesma solução.

**A fonte não pode vir do sistema operacional.** Um caminho como
`C:\Windows\Fonts\arial.ttf` simplesmente não existe no Linux, e o nome e a
localização das fontes variam entre distribuições. A fonte escolhida é
distribuída junto com o código, em `assets/fonts/`.

**O caminho não pode ser relativo ao diretório de trabalho.** Um
`"assets/fonts/DejaVuSans.ttf"` puro só funciona se o programa for chamado da
raiz do projeto. O caminho é montado com `SDL_GetBasePath()`, que devolve o
diretório do próprio executável, e o `Makefile` ganhou um alvo `assets` que
copia a pasta para junto do binário, da mesma forma que já copiava as DLLs.

**Fonte escolhida: DejaVu Sans 2.37.** Os critérios foram licença permissiva
que autoriza a redistribuição junto com o projeto (o arquivo de licença está em
`assets/fonts/LICENSE-DejaVu.txt`), cobertura dos caracteres acentuados do
português, já que as mensagens do programa usam acentuação, e legibilidade em
tamanhos pequenos, necessária para as informações de análise da janela
secundária. O arquivo tem 740 KB.

**Estado da fonte dentro do `App`.** O módulo `text` poderia guardar a fonte em
uma variável estática, que é o caminho mais curto. Preferiu-se uma struct
`TextRenderer` guardada no `App` e recebida por parâmetro, pelo mesmo motivo
que levou à remoção das variáveis globais do código original — não faria
sentido eliminá-las e reintroduzir uma logo em seguida.

**Detalhes da API.** No SDL3_ttf, `TTF_RenderText_Blended()` recebe o
comprimento da string como parâmetro, e o valor 0 indica que a string termina
em `\0`. O `TTF_Quit()` só é chamado se o `TTF_Init()` tiver sido bem-sucedido,
para não desequilibrar a contagem interna da biblioteca quando a inicialização
falha antes disso.

**Verificação.** Executando o programa a partir da raiz do projeto e a partir
de `C:\`, o caminho resolvido foi idêntico nos dois casos:

```
Carregando a fonte C:\...\build\assets/fonts/DejaVuSans.ttf...
```

Uma captura de tela da janela secundária confirmou os textos desenhados, com os
acentos renderizando corretamente em "Informações da imagem".

Com isso a Fase 3 se encerra: janela principal, janela secundária e textos.

---

## 2026-09-22 — Item 4 (parte 1): cálculo do histograma

Começo do item de maior peso do escopo. Novo módulo `histogram`, com uma struct
que guarda a contagem de pixels por nível de intensidade, a maior dessas
contagens e o total.

A `max_count` existe para o desenho: o gráfico precisa ser proporcional à
altura disponível, e a referência para essa proporção é o nível mais
frequente. Calcular na hora de desenhar significaria varrer os 256 níveis a
cada quadro, então o valor é obtido junto com a contagem.

Como a imagem já está em escala de cinza quando o histograma é calculado, os
três canais de cor têm o mesmo valor e a intensidade de cada pixel é lida de um
deles. A varredura usa `surface->pitch`, pelo mesmo motivo das funções de
escala de cinza.

### Verificação

Foram usadas três imagens sintéticas de contagem previsível:

| Imagem | Esperado | Obtido |
| --- | --- | --- |
| gradiente 256x128, intensidade = coluna | 128 pixels em cada um dos 256 níveis, total 32768 | exato, nenhum bin divergente |
| sólida de intensidade 128, 100x50 | 5000 pixels no nível 128 e zero nos demais | exato, nenhum bin divergente |
| gradiente 257x3 (largura ímpar) | total 771 | exato |

O terceiro caso existe para exercitar o cálculo do endereço de cada linha: uma
largura que não é múltiplo de valores "redondos" é a situação em que a SDL pode
inserir bytes de preenchimento no fim das linhas, e em que a varredura por
`w * 4` leria o pixel errado.

Nas imagens reais do projeto os totais também conferem: 32768 pixels para
`gray_gradient.png` (256x128) e 393216 para `kodim23.png` (768x512).

---

## 2026-09-22 — Item 4 (parte 2): desenho do histograma

O critério de avaliação exige que o histograma seja exibido "de forma clara e
proporcional". A função de desenho ficou no próprio módulo `histogram`, que
passa a ser dono tanto do cálculo quanto da apresentação dos seus dados — o
alternativo seria engordar `main.c`, que já concentra bastante coisa.

**Proporcionalidade.** A altura de cada barra é a razão entre a contagem do
nível e a maior contagem do histograma, multiplicada pela altura da área. Assim
a barra mais alta sempre ocupa a altura inteira, independentemente do tamanho
da imagem: um histograma de uma foto de 12 megapixels e o de uma miniatura
ocupam o mesmo espaço e são igualmente legíveis. Normalizar pelo total de
pixels, em vez de pelo máximo, achataria o gráfico até a ilegibilidade.

**Um pixel por nível.** A área reservada tem exatamente 256 pixels de largura,
escolhida lá no dimensionamento da janela secundária justamente para isto: cada
nível ocupa uma coluna de um pixel e nenhuma barra fica mais grossa que a
vizinha por arredondamento, que é o que aconteceria ao espremer 256 níveis em
uma largura arbitrária.

**Clareza.** O fundo da área do gráfico é mais escuro que o da janela, de modo
que os intervalos sem nenhum pixel continuam visíveis como parte do gráfico em
vez de sumirem no fundo. Há moldura em volta e rótulos 0, 128 e 255 alinhados
às posições que representam no eixo.

### Verificação

As duas imagens de teste produzem exatamente o gráfico previsto:

- `gray_gradient.png`, em que os 256 níveis têm a mesma contagem, desenha um
  bloco sólido preenchendo toda a área — todas as barras na altura máxima, que
  é a confirmação visual direta da normalização pelo nível mais frequente.
- `kodim23.png` desenha uma distribuição real, concentrada em torno do nível
  90 e com um segundo agrupamento perto do 190, condizente com uma fotografia
  de tons predominantemente médios.

---

## 2026-09-22 — Item 4 (parte 3): média, desvio padrão e classificações

Fecha o item de maior peso do escopo. A struct do histograma ganhou a média das
intensidades e o desvio padrão em relação a ela, calculados na mesma passagem
em que o histograma é montado.

A acumulação usa `double` em vez de `float`. A soma ponderada de uma imagem de
alguns megapixels chega à casa dos bilhões, faixa em que `float` já perdeu
precisão suficiente para deslocar o resultado.

**Os limiares de classificação.** O enunciado pede as classificações mas não
define os valores de corte, então eles foram escolhidos pelo grupo e estão
justificados no README. Para o brilho, a faixa 0–255 dividida em três partes
iguais, com cortes em 85 e 170. Para o contraste não existe divisão igualmente
óbvia, então os cortes foram ancorados em dois valores de referência: o máximo
possível, 127,5, obtido por uma imagem com metade dos pixels em 0 e metade em
255, e o desvio de uma imagem que usa toda a faixa tonal uniformemente,
`raiz((256² − 1) / 12)`, cerca de 73,9. O corte de contraste alto ficou em 70,
logo abaixo dessa segunda referência, e o de contraste baixo em 40.

Os valores numéricos aparecem na janela secundária junto das classificações. A
ideia é que quem avalia possa conferir a conta, em vez de ter que confiar no
rótulo.

### Verificação

A média e o desvio padrão têm valores previsíveis analiticamente para as
imagens de teste, e os dois conferiram exatamente:

| Imagem | Média esperada | Obtida | Desvio esperado | Obtido |
| --- | --- | --- | --- | --- |
| `gray_gradient.png` (uniforme 0–255) | 127,50 | 127,50 | 73,90 | 73,90 |
| `kodim23.png` | — | 109,71 | — | 47,45 |

O desvio esperado do gradiente vem da fórmula do desvio padrão de uma
distribuição uniforme discreta sobre 0–255.

As classificações foram verificadas com histogramas sintéticos, já que as duas
imagens de teste só exercitam "média" e os contrastes "médio" e "alto":

| Histograma | Média | Desvio | Brilho | Contraste |
| --- | --- | --- | --- | --- |
| sólido no nível 40 | 40,00 | 0,00 | escura | baixo |
| sólido no nível 128 | 128,00 | 0,00 | média | baixo |
| sólido no nível 200 | 200,00 | 0,00 | clara | baixo |
| metade em 0, metade em 255 | 127,50 | 127,50 | média | alto |
| sólido no nível 84 | 84,00 | 0,00 | escura | baixo |

O quarto caso confirma o máximo teórico de 127,5 para o desvio padrão, e o
quinto confirma o comportamento junto ao limiar de 85.

Com isso o item 4 se encerra, e com ele os 2,50 pontos de maior peso do escopo.
