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
