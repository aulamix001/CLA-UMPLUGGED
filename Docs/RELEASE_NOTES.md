Plugin VST3 e aplicativo standalone para **Windows x64**, em C++17/JUCE 8.0.12.

- Seletor de presets na barra superior, com submenus por categoria como na referência.
- Knobs com aro dourado serrilhado, face escura texturizada e indicador branco.
- Botões com moldura metálica, relevo e estados amarelo aceso/cinza apagado.
- 152 presets: preserva os 151 anteriores e acrescenta Default / all modules bypassed.
- Menu com categorias separadas para Guitars & Bass e Strings.
- Carregamento automático dos 21 parâmetros, automação e recuperação do estado na DAW.
- Bass, Treble, Compress, dois reverbs com pré-delay, delay sincronizado e Direct.
- Componentes estéreo e mono→estéreo. A imagem aprovada permanece intacta.

Extraia o ZIP e copie a pasta completa `VST3/Unplugged Study.vst3` para
`C:\Program Files\Common Files\VST3\`. Depois atualize a lista de plugins na DAW.
O executável separado está na pasta `Standalone`.

O processo de publicação exige build e testes no runner Windows, verificação dos
cabeçalhos PE AMD64 e do checksum do pacote. O arquivo `.sha256` permite conferir
a integridade do download.

Esta é uma implementação independente, identificada como **Unplugged Study**.
O processamento não foi comprovado como sonoramente idêntico ao produto Waves,
e ainda não houve comparação em uma DAW com o original.
