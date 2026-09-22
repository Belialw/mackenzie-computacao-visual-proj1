# Projeto 1 — Análise Final (Etapa 2)

Universidade Presbiteriana Mackenzie — Faculdade de Computação e Informática
Ciência da Computação — **Computação Visual** — Prof. André Kishimoto

> **Rascunho.** Conferir os campos marcados com `[...]` antes de exportar para
> PDF, e apagar este aviso.

---

## 1. Integrantes

| Nome completo | RA |
| --- | --- |
| Enzo Ponte Gamberi | 10389931 |
| Luís Henrique Ribeiro Fernandes | 10420079 |
| Raphael Grizante da Silva | 10416979 |
| Vinícius Brait Lorimier | 10420046 |

> Conferir os RAs. O relatório da Etapa 1 trazia o mesmo número para dois
> integrantes, e o valor corrigido precisa aparecer aqui.

---

## 2. Repositório

https://github.com/Belialw/mackenzie-computacao-visual-proj1

---

## 3. Fonte escolhida para os textos

**DejaVu Sans**, versão 2.37, distribuída junto com o programa em
`assets/fonts/DejaVuSans.ttf`.

Os critérios da escolha foram três:

- **Licença permissiva**, que autoriza redistribuir a fonte junto com o
  projeto. O arquivo de licença acompanha a fonte no repositório.
- **Cobertura dos caracteres acentuados do português**, já que as mensagens e
  os rótulos do programa usam acentuação.
- **Legibilidade em tamanhos pequenos**, necessária para as informações de
  análise exibidas na janela secundária.

A fonte é distribuída com o programa em vez de ser carregada do sistema
operacional porque o item 8 do escopo exige que ela funcione independentemente
do sistema. Um caminho como `C:\Windows\Fonts\arial.ttf` não existe no Linux, e
o nome e a localização das fontes do sistema variam entre distribuições.

O caminho é montado em tempo de execução a partir de `SDL_GetBasePath()`, que
devolve o diretório do próprio executável, e não com um caminho relativo ao
diretório de trabalho — este só funcionaria se o programa fosse chamado da raiz
do projeto. O `Makefile` copia a pasta `assets/` para junto do binário. O
comportamento foi verificado executando o programa da raiz do projeto e de
`C:\`, com o mesmo caminho resolvido nos dois casos.

---

## 4. Refatorações do código-fonte original

O projeto partiu do exemplo `src/05-filter_image` do repositório da disciplina.
Ele foi escolhido por já conter a infraestrutura de que o projeto precisava:
carregamento com `IMG_Load` e conversão para `SDL_PIXELFORMAT_RGBA32`,
varredura de pixels com `SDL_LockSurface`/`SDL_GetRGB`/`SDL_MapRGB`, atualização
da textura a partir de uma superfície processada, restauração da imagem original
sem reler o arquivo, e redimensionamento e reposicionamento da janela com
`SDL_GetWindowBordersSize`.

O arquivo foi importado **sem nenhuma alteração** em um commit separado, de modo
que todo o trabalho do grupo aparecesse como diferença em relação a ele.

O próprio cabeçalho do arquivo original lista as refatorações que o autor
considera necessárias em um projeto real, e as quatro foram feitas, cada uma em
seu commit:

**1. Caminho da imagem por parâmetro.** A constante `IMAGE_FILENAME` foi
removida e o programa passou a ler o caminho de `argv[1]`, como o enunciado
define. Isso também era o item 1 do escopo. Foram acrescentadas a validação do
argumento e a verificação do caminho antes de qualquer inicialização da SDL.

**2. Separação em headers e arquivos `.c`.** O programa inteiro vivia em um
único `main.c`. Passou a ser dividido por responsabilidade: `window` cuida da
janela e do renderizador, `image` da imagem em memória, `histogram` da análise,
`button` e `text` dos elementos de interface, e `main` costura tudo. Os módulos
não se conhecem entre si — apenas o `main` inclui todos —, o que evita
dependência circular e permite compilar e testar cada um isoladamente.

**3. Remoção das variáveis globais.** As duas globais, que o autor marcava com
o comentário `// Globals (argh!)`, foram substituídas por uma struct `App`
criada como variável local de `main()` e recebida por parâmetro. A função de
renderização recebe um ponteiro constante, já que apenas lê o estado, o que o
compilador passou a garantir.

Esta foi a refatoração com o ponto mais delicado: o código original registrava
a limpeza com `atexit()`, que só aceita função sem parâmetros. Manter um
ponteiro estático para o `App`, só para a função de encerramento enxergá-lo,
reintroduziria pela porta dos fundos exatamente o estado global que a
refatoração eliminava. A limpeza passou a ser chamada explicitamente nos pontos
de saída.

**4. Redução dos logs.** Eram 70 chamadas de log registrando a entrada e a
saída de praticamente toda função. Apagar tudo custaria uma ferramenta de
depuração útil; manter tudo afogaria as mensagens que o enunciado exige no
terminal. Foram separadas em três macros: uma que só produz saída na build de
depuração e duas que existem nas duas builds, por serem dirigidas a quem usa o
programa. Das 70 chamadas, 47 viraram diagnóstico, 2 informação e 21 erro. Com
uma imagem válida, a build de release passou a não imprimir nada além do que o
enunciado pede.

Além dessas quatro, foram feitas outras duas alterações no código original:

**Remoção do filtro de média.** Era a funcionalidade de demonstração do exemplo
e não consta de nenhum item do escopo. Saiu antes da separação em módulos, para
não adaptar código que seria apagado em seguida, e com ele saíram a superfície
de trabalho, os dois cursores de mouse e as teclas de 1 a 9 que só existiam para
atendê-lo. O arquivo passou de 667 para 511 linhas.

**Correção de um comportamento indefinido.** Ao recompilar o código original com
`-Wall -Wextra -Wpedantic`, o compilador acusou
`'%s' directive argument is null`. Dentro de um bloco que só executa quando o
nome do arquivo é nulo, a variável era mesmo assim passada para um `%s` — o que
é comportamento indefinido em C. A verificação foi movida para antes do log de
entrada da função.

---

## 5. Problemas encontrados e como foram solucionados

**O `make` não usa sempre o mesmo shell no Windows.** Medindo o valor de
`$(SHELL)`, o `mingw32-make` reportou o `sh.exe` do Git quando chamado pelo Git
Bash, e caiu no `cmd.exe` quando chamado pelo PowerShell. O mesmo Makefile
precisaria de `mkdir -p build` em um terminal e de `if not exist build mkdir
build` no outro. Solução: forçar o interpretador no ramo Windows, o que tornou a
build idêntica a partir de qualquer terminal. A definição precisa aparecer antes
de qualquer expansão de comando do arquivo, senão a expansão já acontece com o
shell errado.

**`-std=c23` não existe antes do gcc 14.** O repositório da disciplina compila
com essa opção e o professor usa gcc 15, mas o gcc 13.3 disponível no WSL da
máquina de desenvolvimento a recusa, sugerindo `-std=c2x` — o mesmo padrão com
outro nome. O Makefile passou a consultar a versão do compilador e escolher o
nome correto.

**Posicionar uma janela em (0,0) corta a barra de título.** A janela secundária
foi posicionada na coordenada (0,0) conforme o enunciado, e uma captura de tela
mostrou a janela sem barra de título: `SDL_SetWindowPosition()` posiciona a área
de cliente, então a decoração fica acima da borda da tela. Na prática a janela
ficava sem título visível e sem como ser arrastada ou fechada pelo botão. A
correção consulta a espessura da borda e desloca a janela por ela, encostando a
janela inteira no canto. O próprio exemplo da disciplina faz esse ajuste, com um
comentário explicando o mesmo problema — só que para a janela principal.

**O alvo `run` do Makefile falhava.** O `cmd.exe` interpreta a barra normal no
início de um comando como início de uma opção, e por isso não reconhecia
`build/imgproc.exe` como executável. Foi acrescentada uma variável que converte
o caminho para barras invertidas no Windows. O problema só apareceu porque o
alvo foi realmente executado, o que ensinou a testar cada alvo do Makefile e não
apenas a compilação.

**Uma atribuição que nunca entrou no arquivo.** A gravação da imagem falhava com
"não há imagem em exibição para salvar". A causa não estava na lógica: a linha
que registrava qual superfície estava sendo exibida nunca chegou a ser inserida,
porque a edição automatizada usada para escrevê-la não casou com o texto do
arquivo. O código compilou normalmente, já que um ponteiro nulo é perfeitamente
válido para o compilador. A lição foi conferir o resultado de uma edição em vez
de supor que ela foi aplicada.

**Um caminho que a máquina de desenvolvimento nunca exercitava.** O item 6 exige
posicionar a janela principal no canto da tela quando ela excede a resolução do
monitor. O monitor usado tem 3440 pixels de largura útil, e nenhuma imagem comum
dispara essa condição — o caminho seria implementado e nunca testado. Foi gerada
uma imagem de 4000x2600 especificamente para exercitá-lo.

**Detalhes do sistema que atrapalham testar interface por automação.** Dois
comportamentos do Windows apareceram ao automatizar os testes de clique e
teclado: o primeiro clique enviado a uma janela sem foco é consumido pelo
sistema para ativá-la e não chega ao programa, e um evento de teclado enviado
sem o código de varredura da tecla também não chega. Nenhum dos dois é
comportamento do programa.

---

## 6. Itens que a análise inicial considerou resolvíveis apenas com o material da disciplina

Na Etapa 1, o grupo avaliou que os itens **1 (carregamento de imagem)**,
**2 (escala de cinza)**, **3 (interface com duas janelas)** e
**8 (exibição de textos)** seriam resolvidos consultando apenas o material da
disciplina e a documentação da SDL. A avaliação se mostrou correta para dois
deles e errada para os outros dois.

**Itens 1 e 2 — a avaliação se confirmou.** O exemplo da disciplina já
carregava a imagem e já percorria os pixels, e a fórmula da conversão para
escala de cinza está no próprio enunciado. O que exigiu consulta à documentação
da SDL foi apenas a função usada para distinguir "arquivo não existe" de "o
caminho é um diretório", o que permitiu mensagens de erro mais específicas.

**Item 3 — exigiu bem mais do que o previsto.** Todos os exemplos da disciplina
usam uma única janela. Não havia referência para criar uma segunda janela como
filha da primeira, para encaminhar eventos entre duas janelas pelo identificador
de origem, nem para o problema da barra de título descrito na questão anterior.
Foi necessário consultar a documentação da SDL3 e recorrer a IA generativa.

**Item 8 — exigiu mais do que o previsto.** Uma busca por "ttf" e "font" em todo
o repositório da disciplina não retorna nenhuma ocorrência: não há exemplo nem
instruções de instalação da SDL_ttf. Foi preciso descobrir a instalação da
biblioteca, a API de carregamento e desenho de texto, e a função que devolve o
diretório do executável — sem a qual a fonte só seria encontrada se o programa
fosse chamado da pasta certa.

---

## 7. Itens que a análise inicial reconheceu que precisariam de pesquisa

Na Etapa 1, o grupo reconheceu que os itens **4 (histograma)**,
**5 (equalização)**, **6 (exibição da imagem)** e **7 (salvar imagem)**
exigiriam pesquisa adicional ou ajuda de IA generativa. Aqui a avaliação também
foi parcialmente correta.

**Item 5 — foi onde a pesquisa mais rendeu.** A equalização por função de
distribuição acumulada era o conceito mais desconhecido do projeto, e entendê-la
mudou a forma de pensar o restante. Ficou claro por que a transformação subtrai
o primeiro valor acumulado não nulo, por que a tabela resultante nunca decresce,
e por que uma imagem já equalizada é mapeada para ela mesma. Esse último ponto
virou inclusive um teste: equalizar uma imagem de histograma uniforme devolve a
identidade exata nos 256 níveis.

**Item 4 — o cálculo foi mais simples do que se esperava, mas trouxe uma
decisão inesperada.** Montar o histograma e calcular média e desvio padrão é
direto. O que não era óbvio é que o enunciado pede as classificações de brilho e
contraste sem definir os valores de corte, o que obrigou o grupo a escolhê-los e
justificá-los. Para o brilho a divisão da faixa em três partes iguais resolve;
para o contraste foi preciso encontrar referências — o desvio máximo possível, e
o desvio de uma imagem que usa toda a faixa tonal uniformemente — para que os
limiares não fossem arbitrários.

**Item 6 — já estava quase pronto no exemplo.** O redimensionamento e o
reposicionamento da janela, com o ajuste da borda, são exatamente o que o
exemplo `05-filter_image` faz ao carregar uma imagem maior que a janela. A
pesquisa prevista não foi necessária.

**Item 7 — mais simples do que o previsto.** A gravação é uma única chamada da
SDL_image. O único detalhe que exigiu atenção não estava na gravação e sim na
ordem: a existência do arquivo precisa ser consultada **antes** de gravar, já
que depois ele existe nos dois casos e a distinção entre criar e sobrescrever se
perde.

---

## 8. Como a IA generativa ajudou no desenvolvimento

`[Conferir e ajustar conforme o uso de cada integrante.]`

A IA generativa foi usada ao longo de todo o desenvolvimento da Etapa 2, de
quatro formas distintas:

**Consulta à API da SDL3.** A SDL3 é recente e boa parte do material disponível
na internet se refere à SDL2, cujas assinaturas mudaram. Em vez de confiar nessa
memória, a prática adotada foi conferir cada função diretamente nos cabeçalhos
da versão instalada antes de usá-la. Isso evitou erros em pontos onde a API
mudou, como a função de desenho de texto, que na SDL3 recebe o comprimento da
string como parâmetro, e as funções de conversão de cor, que ganharam um
parâmetro a mais.

**Escrita de testes com resultado previsível.** Foi a contribuição mais útil.
Em vez de verificar o processamento "no olho", foram escritos programas que
chamam as funções do projeto com entradas cujo resultado é conhecido
analiticamente: a conversão para escala de cinza de cores puras, o histograma de
um gradiente uniforme, a tabela de equalização de uma imagem já equalizada. Isso
transformou "parece certo" em "confere com a conta".

**Investigação de falhas.** Vários dos problemas descritos na questão 5 foram
encontrados assim: reproduzir a falha, formular uma hipótese, medir. O caso do
shell do `make`, por exemplo, foi resolvido medindo o valor da variável nos dois
terminais em vez de supor qual deles estava em uso.

**Redação da documentação.** O `README.md` e o diário de desenvolvimento em
`docs/diario.md` foram escritos com apoio de IA, a partir das decisões tomadas
durante o trabalho.

Uma ressalva vale ser registrada: a IA também errou. Em dois momentos, edições
automatizadas de arquivo não foram aplicadas e o erro só apareceu em tempo de
execução, porque o código continuava compilando. Isso reforçou a necessidade de
conferir o resultado de cada alteração em vez de aceitá-la como feita.

---

## 9. Assuntos que o grupo precisa estudar e praticar mais

**Formato e disposição de pixels na memória.** O ponto que mais se repetiu no
projeto foi a diferença entre a largura de uma imagem e o número de bytes que
uma linha ocupa na memória. O código-base calcula o endereço de um pixel
supondo que os dois coincidem, o que vale para o formato usado, mas não em
geral. Entender por que a biblioteca pode inserir bytes de preenchimento no fim
de cada linha, e quando isso acontece, é um tema de fundo que aparece em
qualquer processamento de imagem.

**Programação orientada a eventos.** O projeto desenha em resposta a eventos, e
não em um laço contínuo. Decidir o que dispara um redesenho, e garantir que todo
caminho que altera o estado também o dispare, exigiu mais cuidado do que
parecia. Foi por isso que as teclas herdadas do exemplo foram removidas: elas
eram um segundo caminho para o mesmo estado, capaz de deixar a imagem revertida
com o rótulo do botão desatualizado.

**Gerenciamento de memória com recursos de biblioteca.** Manter o controle de
quem cria e quem libera cada superfície e cada textura, especialmente quando uma
função recebe uma superfície que pode ou não ser a mesma que o objeto já
guardava, é um tipo de cuidado que não aparece em exercícios menores.

**Portabilidade de build.** Boa parte dos problemas do projeto não estava no
código C, e sim em torno dele: qual interpretador o `make` usa, qual nome o
compilador dá ao padrão da linguagem, onde as bibliotecas estão instaladas. São
assuntos que raramente aparecem em disciplina de programação e que decidem se o
projeto compila na máquina de outra pessoa.

**Fundamentos de processamento de imagens.** A equalização foi o primeiro
contato do grupo com uma transformação de intensidade baseada na distribuição
dos pixels. Operações de vizinhança, filtragem no domínio da frequência e
segmentação continuam sendo território desconhecido.

---

## 10. Funcionalidades que seriam importantes do ponto de vista do usuário

O software desenvolvido é um protótipo. Da perspectiva de quem o usaria de
fato, as funcionalidades abaixo seriam as mais relevantes.

**1. Abrir imagem pela própria interface.** Hoje o caminho da imagem só pode ser
informado na linha de comando, o que obriga o usuário a abrir um terminal e
digitar um caminho. Um menu ou botão que abra o seletor de arquivos do sistema,
e a possibilidade de trocar de imagem sem reiniciar o programa, eliminariam a
barreira de entrada mais óbvia.

**2. Arrastar e soltar a imagem na janela.** Complementa a anterior e é o gesto
que a maioria dos usuários tenta primeiro ao querer abrir um arquivo.

**3. Escolher onde e com que nome salvar.** O nome `output_image.png` é fixo e o
arquivo vai para o diretório de onde o programa foi chamado. Salvar duas imagens
sem sobrescrever a primeira exige renomear o arquivo manualmente entre uma
gravação e outra. Um diálogo de gravação, com escolha de pasta, nome e formato,
resolveria.

**4. Desfazer e refazer.** Hoje só existe uma operação reversível, e apenas para
o estado imediatamente anterior. Um histórico de operações, com desfazer e
refazer, é o que permite experimentar sem medo — e é o que diferencia uma
ferramenta de um demonstrador.

**5. Comparar antes e depois.** Avaliar o efeito da equalização exige alternar
entre os dois estados e confiar na memória visual. Exibir as duas versões lado a
lado, ou uma divisória arrastável sobre a imagem, tornaria a comparação direta.

**6. Zoom e deslocamento da imagem.** Com uma imagem grande, hoje só é possível
vê-la inteira reduzida ou em tamanho original dentro de uma janela maior que a
tela. Ampliar uma região e percorrer a imagem é básico em qualquer visualizador.

**7. Ajuste manual de brilho e contraste.** A equalização é automática e não
admite ajuste. Controles deslizantes, com o resultado atualizando enquanto o
usuário arrasta e o histograma acompanhando, dariam controle sobre o resultado
em vez de oferecer uma única transformação fixa.

**8. Informação do pixel sob o cursor.** Exibir a coordenada e a intensidade do
pixel apontado, e destacar a posição correspondente no histograma, conectaria as
duas janelas e ajudaria a entender o que o gráfico representa.

**9. Atalhos de teclado para todas as ações, e uma forma de descobri-los.** Hoje
só a gravação tem atalho, e ele não é anunciado em lugar nenhum da interface:
quem não ler a documentação não descobre que a tecla existe.

**10. Indicação de progresso em imagens grandes.** A conversão e a equalização
de uma imagem de vários megapixels levam tempo perceptível, durante o qual a
janela simplesmente não responde. Uma barra de progresso, ou ao menos a mudança
do cursor, evitaria a impressão de que o programa travou.
