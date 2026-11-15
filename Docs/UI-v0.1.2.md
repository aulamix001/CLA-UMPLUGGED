# Interface v0.1.2 — acabamento em relevo

Mantém o menu de presets no topo e as categorias da v0.1.1.

O corpo dos knobs usa uma camada PNG transparente de 1254 × 1254 pixels,
com aro de latão escovado recortado, chanfro elevado, reflexos fixos e sombra
de contato. A face central é cinza fosca texturizada. Um ponteiro branco
vetorial é desenhado separadamente conforme o valor do parâmetro, mantendo
nitidez ao redimensionar a janela e iluminação estável durante o giro.

Os botões de lente amarela/cinza conservam suas bordas metálicas e ganham
deslocamento visual ao pressionar. Os controles continuam ligados aos
parâmetros e à automação da DAW.

`Assets/cla-unplugged-original.png` permanece byte a byte igual ao arquivo
aprovado. O corpo novo fica em `Assets/knob-body.png`.

As alterações funcionais passaram pelas duas suítes locais: DSP/presets e
processador/interface, incluindo carga do VST3, menu completo, modos, estados
luminosos, automação dos knobs e redimensionamento. A publicação Windows
também exige todos os testes e a inspeção PE AMD64.

## Windows aprovado e download conferido

- [Build Windows v0.1.2](https://github.com/aulamix001/CLA-UMPLUGGED/actions/runs/43766092053): concluído com sucesso.
- [Release v0.1.2](https://github.com/aulamix001/CLA-UMPLUGGED/releases/tag/v0.1.2): ZIP, checksum e três capturas de interface publicados.
- O pacote publicado foi baixado e conferido: VST3 DLL e standalone são PE AMD64.
- As capturas Windows do menu e dos pré-delays foram inspecionadas após o download.
- SHA-256 do ZIP: `b6702d96b44032f0aa23be186bd333144a78b82ce8fdf36bfd9a67026ed0b8b0`.
