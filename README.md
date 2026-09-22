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

### Dependências

O projeto depende de **SDL3**, **SDL3_image** e **SDL3_ttf**.

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
| `mingw32-make clean` | Remove a pasta `build/` |

No Linux/WSL o comando é `make` em vez de `mingw32-make`.

O executável e as DLLs necessárias são gerados em `build/`. Essa pasta é
ignorada pelo controle de versão.

### Execução

```
build\imgproc.exe caminho_da_imagem.ext     # Windows
./build/imgproc caminho_da_imagem.ext       # Linux/WSL
```

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
