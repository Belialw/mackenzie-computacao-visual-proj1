# Projeto 1 — Processamento de imagens em C com SDL3

Universidade Presbiteriana Mackenzie — Faculdade de Computação e Informática
Ciência da Computação — **Computação Visual** — Prof. André Kishimoto

---

## Integrantes

| Nome completo | RA |
| --- | --- |
| Enzo Ponte Gamberi | 10389931 |
| Luís Henrique Ribeiro Fernandes | 10420079 |
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

### Dependências

O projeto depende de **SDL3** e **SDL3_image**, que são as bibliotecas exigidas
pelo enunciado, e opcionalmente de **SDL3_ttf** para os textos da interface.

#### Windows com MinGW (método adotado pelo grupo)

É o mesmo processo descrito no material da disciplina: instalação manual dos
pacotes de desenvolvimento, sem CMake.

1. Baixe os pacotes `mingw` dos releases oficiais:
   - [SDL3-devel-3.4.16-mingw.zip](https://github.com/libsdl-org/SDL/releases/tag/release-3.4.16)
   - [SDL3_image-devel-3.4.6-mingw.zip](https://github.com/libsdl-org/SDL_image/releases/tag/release-3.4.6)
   - [SDL3_ttf-devel-3.2.2-mingw.zip](https://github.com/libsdl-org/SDL_ttf/releases/tag/release-3.2.2)

2. De cada arquivo, extraia **apenas** a pasta `x86_64-w64-mingw32` (a pasta
   `i686-w64-mingw32` é a variante de 32 bits e não é usada).

3. Mescle o conteúdo das três pastas em um único diretório, de forma que as
   três bibliotecas compartilhem os mesmos `include/`, `lib/` e `bin/`:

   ```
   D:\dev\compvis\libs\SDL3
   ├── bin\        SDL3.dll, SDL3_image.dll, SDL3_ttf.dll
   ├── include\    SDL3\, SDL3_image\, SDL3_ttf\
   ├── lib\        libSDL3.dll.a, libSDL3_image.dll.a, libSDL3_ttf.dll.a
   └── share\
   ```

   O caminho `D:\dev\compvis\libs\SDL3` é o padrão do Makefile por ser o mesmo
   usado no material da disciplina. Para instalar em outro lugar, informe o
   caminho na linha de comando:

   ```
   mingw32-make SDL_DIR=c:/libs/SDL3
   ```

#### Linux / WSL

Instale os pacotes de desenvolvimento das três bibliotecas de modo que o
`pkg-config` as localize. Verifique com:

```
pkg-config --modversion sdl3 sdl3-image sdl3-ttf
```

### Compilação

| Comando | Efeito |
| --- | --- |
| `mingw32-make` | Compila em modo release (`-O2 -DNDEBUG`) |
| `mingw32-make DEBUG=1` | Compila com símbolos de depuração e logs habilitados |
| `mingw32-make run IMG=caminho/imagem.png` | Compila e executa |
| `mingw32-make USE_SDL_TTF=0` | Compila sem a SDL_ttf (ver a seção seguinte) |
| `mingw32-make clean` | Remove a pasta `build/` |

No Linux/WSL o comando é `make` em vez de `mingw32-make`.

O executável e as DLLs necessárias são gerados em `build/`. Essa pasta é
ignorada pelo controle de versão.

### Execução

```
build\imgproc.exe caminho_da_imagem.ext     # Windows
./build/imgproc caminho_da_imagem.ext       # Linux/WSL
```


### SDL_ttf é opcional

O enunciado exige SDL3 e SDL_image, e apenas sugere a SDL_ttf. Caso ela não
esteja disponível na máquina, o projeto compila sem a dependência:

```
mingw32-make USE_SDL_TTF=0
```

A interface fica sem os textos — histograma, botões, processamento e gravação
continuam funcionando normalmente. O mesmo vale em tempo de execução: se o
arquivo da fonte não for encontrado, o programa avisa no terminal e segue, em
vez de encerrar.

### Como usar

O programa abre duas janelas: a principal, com a imagem sendo processada, e a
secundária, com o histograma, a análise e os controles.

| Ação | Como |
| --- | --- |
| Equalizar o histograma | botão **Equalizar histograma** |
| Voltar à imagem em escala de cinza | botão **Ver original** |
| Exibir na resolução original da imagem | botão **Resolução original** |
| Voltar para 1024x768 | botão **1024x768** |
| Salvar a imagem exibida | tecla **S** |
| Encerrar | fechar qualquer uma das janelas |

O terminal informa se a imagem de entrada era colorida ou já estava em escala
de cinza, e o resultado de cada gravação de `output_image.png`.

### Notas sobre o Makefile

Dois detalhes que o Makefile resolve automaticamente:

- **Padrão da linguagem.** O `gcc` só passou a aceitar o nome `c23` na versão
  14; em versões anteriores o mesmo padrão se chama `c2x`. O Makefile consulta
  a versão do compilador e escolhe o nome correto, evitando falha de build em
  `gcc` mais antigo.

- **Shell das regras.** No Windows o GNU make usa `sh.exe` ou `cmd.exe`
  conforme o terminal que iniciou a build, o que mudaria a sintaxe válida dos
  comandos das regras. O Makefile força `cmd.exe`, tornando a compilação
  idêntica a partir do PowerShell, do `cmd` ou do Git Bash.

---

## Estrutura do repositório

```
.
├── src/
│   ├── main.c          estado da aplicação, ciclo de vida, eventos e layout
│   ├── window.h/.c     janela da SDL e o renderizador associado a ela
│   ├── image.h/.c      carregamento, escala de cinza, equalização e gravação
│   ├── histogram.h/.c  cálculo, desenho, estatísticas e tabela de equalização
│   ├── button.h/.c     botão em primitivas da SDL, com três estados visuais
│   ├── text.h/.c       carregamento da fonte e desenho de texto
│   └── log.h           separa log de diagnóstico de mensagem ao usuário
├── assets/fonts/       fonte distribuída com o programa, e sua licença
├── samples/            imagens de teste
├── docs/diario.md      registro de decisões e problemas do desenvolvimento
└── Makefile            build para Windows (MinGW) e Linux/WSL
```

A divisão é por responsabilidade: `window` cuida do que aparece como janela,
`image` do que é a imagem em memória, `histogram` da análise, `button` e `text`
dos elementos de interface, e `main` costura tudo. Os módulos não se conhecem
entre si — apenas `main` inclui todos —, o que evita dependência circular e
permite compilar e testar cada um isoladamente.

### Imagens de teste

| Arquivo | Para que serve |
| --- | --- |
| `kodim23.png` | Fotografia colorida, a mesma usada nos exemplos da disciplina. |
| `gray_gradient.png` | 256x128 já em escala de cinza, com a mesma quantidade de pixels em cada um dos 256 níveis. Exercita a detecção de imagem que não precisa ser convertida, e tem média e desvio padrão previsíveis analiticamente (127,50 e 73,90). |
| `large_gradient.png` | 4000x2600, maior que a área útil da maioria dos monitores. Exercita o posicionamento da janela principal quando ela não cabe na tela. |

## Decisões de implementação

### Limiares de classificação do histograma

O enunciado pede que a imagem seja classificada como "clara", "média" ou
"escura" a partir da média de intensidade, e que o contraste seja classificado
como "alto", "médio" ou "baixo" a partir do desvio padrão, mas não define os
valores de corte. Os adotados pelo grupo são:

| Classificação | Critério |
| --- | --- |
| Imagem escura | média < 85 |
| Imagem média | 85 ≤ média < 170 |
| Imagem clara | média ≥ 170 |
| Contraste baixo | desvio padrão < 40 |
| Contraste médio | 40 ≤ desvio padrão < 70 |
| Contraste alto | desvio padrão ≥ 70 |

**Brilho.** A faixa de intensidades vai de 0 a 255 e foi dividida em três
partes iguais, o que coloca os cortes em 85 e 170.

**Contraste.** O desvio padrão não tem uma divisão igualmente óbvia, então os
cortes foram ancorados em dois valores de referência:

- O **máximo possível** é 127,5, obtido por uma imagem com metade dos pixels em
  0 e metade em 255. É um caso degenerado, que não ocorre em imagens reais.
- Uma imagem que usa **toda a faixa tonal de maneira uniforme** tem desvio
  padrão de `raiz((256² − 1) / 12)`, aproximadamente **73,9**.

O corte de contraste alto ficou em 70, logo abaixo dessa segunda referência:
uma imagem que percorre toda a faixa tonal é classificada como de contraste
alto. O corte de contraste baixo ficou em 40, abaixo do qual a distribuição se
concentra em uma faixa estreita e a imagem aparenta estar "lavada".

Os valores numéricos da média e do desvio padrão são exibidos na janela
secundária junto das classificações, de modo que a análise possa ser conferida,
e não apenas lida.

### Fonte dos textos

**DejaVu Sans 2.37**, distribuída com o projeto em `assets/fonts/`. Ver
[assets/fonts/README.md](assets/fonts/README.md) para os critérios da escolha.

### Exibição da imagem

A imagem é desenhada na maior escala que couber na janela sem distorcer as
proporções, centralizada, com o espaço restante como margem. A alternativa
seria esticá-la até as dimensões exatas da janela, o que deformaria qualquer
imagem cuja proporção fosse diferente da proporção da janela.


### Imagem salva

A tecla `S` grava `output_image.png` no diretório de onde o programa foi
chamado, sobrescrevendo um arquivo existente e informando no terminal se ele
foi criado ou sobrescrito.

O arquivo contém a imagem que está sendo exibida — em escala de cinza ou
equalizada, conforme o estado do botão — sempre na **resolução original da
imagem de entrada**, e não no tamanho da janela. O enunciado diz "a imagem
atualmente exibida na janela principal", o que poderia ser lido como o conteúdo
da janela; a interpretação adotada é que o que muda é *qual* imagem, e não a
escala com que ela aparece na tela. Salvar o conteúdo da janela introduziria a
perda de qualidade do redimensionamento em um arquivo que se espera ser o
resultado do processamento.

---

## Contribuições

O trabalho foi dividido nas frentes abaixo. O histórico do repositório
(`git log --format='%an - %s'`) e o `docs/diario.md` registram em detalhe as
decisões e os problemas de cada uma.

| Frente | Integrante | O que envolveu |
| --- | --- | --- |
| Configuração do repositório | Enzo Ponte Gamberi | `.gitignore`, para que nenhum arquivo gerado na compilação fosse versionado, e `.gitattributes`, para que o checkout use sempre LF e um Makefile com CRLF não quebre a build no Linux. |
| Instalação do ambiente | Enzo Ponte Gamberi | Instalação da SDL3, SDL3_image e SDL3_ttf pelo método adotado na disciplina, com a extração da variante de 64 bits e a mesclagem das três em um único diretório; verificação das versões e da compilação. |
| Sistema de build | Enzo Ponte Gamberi | Makefile único para Windows e Linux, com detecção de plataforma, escolha do nome do padrão da linguagem conforme a versão do `gcc`, builds de depuração e de release separadas, cópia das DLLs e dos recursos para junto do executável, e a opção de compilar sem a SDL_ttf. |
| Importação e refatoração do código-base | Luís Henrique Ribeiro Fernandes | Importação do exemplo `05-filter_image` sem alterações, seguida das quatro refatorações que o próprio autor aponta no cabeçalho do arquivo: caminho da imagem por argumento, separação em headers e arquivos `.c`, remoção das variáveis globais em favor de uma struct de estado, e separação entre log de diagnóstico e mensagem ao usuário. |
| Carregamento de imagem (item 1) | Luís Henrique Ribeiro Fernandes | Leitura do caminho da imagem por argumento; validação anterior à inicialização da SDL, distinguindo arquivo inexistente de diretório; mensagens de erro no terminal para arquivo ausente, formato inválido e uso incorreto do programa. |
| Escala de cinza (item 2) | Luís Henrique Ribeiro Fernandes | Detecção de imagem colorida percorrendo os pixels; conversão pela fórmula do enunciado, preservando o canal alpha; mensagem no terminal indicando qual era o caso da imagem de entrada. |
| Janelas (item 3) | Raphael Grizante da Silva | Janela principal em 1024x768 centralizada no monitor principal; janela secundária filha, de tamanho fixo, encostada no canto superior esquerdo da tela; roteamento dos eventos entre as duas janelas pelo identificador de origem. |
| Cálculo do histograma (item 4) | Raphael Grizante da Silva | Contagem de pixels por nível de intensidade, com varredura pelo `pitch` da superfície; média de intensidade e desvio padrão; classificação de brilho e contraste, com a definição e a justificativa dos limiares adotados. |
| Exibição do histograma (item 4) | Raphael Grizante da Silva | Desenho do gráfico com um pixel por nível, normalizado pelo nível mais frequente para que a altura seja proporcional; eixo e rótulos; apresentação das informações de análise junto dos valores numéricos. |
| Equalização (item 5) | Vinícius Brait Lorimier | Tabela de mapeamento a partir da função de distribuição acumulada; aplicação sobre os pixels em uma superfície separada, mantendo a imagem em escala de cinza intacta em memória; alternância entre as duas versões sem recarregar o arquivo do disco. |
| Botões e resolução (itens 5 e 6) | Vinícius Brait Lorimier | Widget de botão desenhado com primitivas da SDL, com os três estados visuais e a troca de rótulo conforme a ação; tratamento dos eventos de mouse; alternância da janela principal entre a resolução original da imagem e 1024x768, com o reposicionamento correspondente. |
| Textos e gravação (itens 7 e 8) | Vinícius Brait Lorimier | Carregamento da fonte distribuída com o programa, a partir do diretório do executável; desenho e medição de texto; gravação da imagem exibida em `output_image.png` pela tecla `S`, informando no terminal se o arquivo foi criado ou sobrescrito. |

A documentação — `README.md`, o diário de desenvolvimento e os relatórios das
duas etapas — e os testes de cada funcionalidade foram trabalho compartilhado
entre os integrantes.