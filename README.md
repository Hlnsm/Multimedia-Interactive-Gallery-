# Immersive Gallery

Projeto em openFrameworks para uma galeria 3D de imagens e videos.

## Requisitos

- Visual Studio com suporte para C++.
- openFrameworks para Visual Studio.
- Addons:
  - `ofxXmlSettings`
  - `ofxOpenCv`

## Como correr

1. Colocar a pasta `ImmersiveGallery` em:

```text
openFrameworks/apps/myApps/
```

2. Abrir `ImmersiveGallery.sln` no Visual Studio.
3. Selecionar `Debug` e `x64`.
4. Correr com `Local Windows Debugger`.

## Media

Colocar imagens e videos em:

```text
bin/data/media
```

A metadata XML e criada automaticamente em:

```text
bin/data/metadata
```

Para recarregar novos ficheiros durante a execucao, carregar em `R`.

Formatos suportados:

- Imagens: `jpg`, `jpeg`, `png`, `gif`
- Videos: `mp4`

## Controlos

| Controlo | Acao |
| --- | --- |
| `WASD` / setas | Mover camara |
| `Q` / `E` | Mover camara na vertical |
| Rato | Orbitar / zoom |
| Clique no media | Abrir em ecra grande |
| `V` | Voltar a galeria |
| `M` | Mostrar/esconder metadata |
| `R` | Recarregar media |
| `Espaco` na galeria | Pausar/continuar movimento dos objetos |
| `Espaco` em ecra grande | Play/pause do video |
| `C` | Escolher numero de clusters |
| `Enter` | Confirmar clusters |
| `Esc` | Cancelar escolha de clusters |
| `G` | Voltar a grelha |
| `F` | Fullscreen |
| `H` | Ajuda |
