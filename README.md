# Unplugged Study — C++ / JUCE / Windows x64

**Download Windows x64:** [Unplugged Study — VST3 + Standalone, última versão](https://github.com/aulamix001/CLA-UMPLUGGED/releases/latest/download/Unplugged-Study-Windows-x64.zip).
[Release e checksum](https://github.com/aulamix001/CLA-UMPLUGGED/releases/latest)
· [Build e testes no Windows](https://github.com/aulamix001/CLA-UMPLUGGED/actions/workflows/windows.yml).

Implementação independente de um processador no fluxo do **CLA Unplugged**,
com interface baseada na imagem aprovada, VST3 e aplicativo standalone.
O modelo foi confirmado pelo usuário como Unplugged, substituindo o pedido
inicial de CLA Vocals. Não há Pitch, Wide ou Spreader.

O manual descreve a operação, não os algoritmos proprietários da Waves.
O DSP deste projeto é próprio: não constitui uma cópia sonora comprovada do
produto comercial. Os presets são pontos de partida, não correção automática
ou classificação do áudio recebido.

## Usar no Windows 64 bits

Requisitos de execução: Windows 10/11 x64 e
[Microsoft Visual C++ Runtime x64](https://aka.ms/vs/17/release/vc_redist.x64.exe).
O binário publicado importa MSVCP140 e VCRUNTIME140; instale o runtime se ele
ainda não estiver disponível na máquina.

Baixe e extraia o ZIP da release no início desta página, ou compile o projeto
conforme as instruções abaixo:

1. Localize a pasta `VST3` no ZIP extraído ou dentro dos artefatos Release.
2. Copie a **pasta inteira** `Unplugged Study.vst3` para
   `C:\Program Files\Common Files\VST3\`.
3. Atualize a lista de plugins na DAW e insira **Unplugged Study** em uma
   pista mono ou estéreo. A saída é estéreo nos dois casos.
4. Abra o seletor **A:** na barra superior e escolha uma categoria e um preset.
   Todos os controles são
   mapeados e carregados automaticamente. O tempo do delay vem da DAW;
   sem andamento disponível, o fallback é 120 BPM, também usado pelos pré-delays.

O standalone permite selecionar a interface de áudio nas opções do wrapper
JUCE. Use fones para evitar realimentação entre microfone e caixas.

## Controles

| Parâmetro | Faixa / comportamento |
| --- | --- |
| Input sensitivity / Output | −10 a +10 dB; passo de 0,1 |
| Bass / Treble / Compress / Reverb 1 / Reverb 2 / Delay | −10 a +10; passo de 0,1 |
| Bass colour | OFF, SUB, LOWER, UPPER |
| Treble colour | OFF, BITE, TOP, ROOF |
| Compress colour | OFF, PUSH, SPANK, WALL |
| Reverb 1 colour | OFF, ROOM, HALL, CHAMBER |
| Reverb 2 colour | OFF, TIGHT, LARGE, CANYON |
| Delay colour | OFF, SLAP, EIGHT, QUARTER |
| Pré-delay de cada reverb | 14 divisões de 64T a 4, sincronizadas ao BPM; padrão 16, botão desligado |
| Balance | −6 a +6 dB, passo de 0,1; somente entrada estéreo; duplo clique centraliza |
| Direct | Ligado: sinal direto + efeitos; desligado: retorno somente dos efeitos |
| Bypass | Sinal original; preserva a conversão mono→estéreo |

As setas alternam OFF→verde→azul→vermelho. Duplo clique nos faders retorna
a zero; nos pré-delays retorna a 1/16 de nota. O pico de saída é retido até clicar
no medidor de saída. A automação e o estado são geridos pelo JUCE APVTS:
o projeto da DAW recupera todos os parâmetros e o índice de preset.

O controle de compressão é uma quantidade macro, não dB de redução de ganho;
os controles de reverb/delay também são quantidades macro. As curvas internas,
frequências, razões, tempos de decaimento e o slap de 120 ms foram definidos
para esta implementação e não extraídos do produto original.

## Presets e mapeamento automático

O banco possui **185 programas**: Init, 50 perfis com versões principal,
Dry e Ambient, **Default / all modules bypassed** e os 33 programas XML. Os 152 índices existentes
foram preservados; 33 presets dos XMLs enviados foram acrescentados na categoria XML Reference.
Inclui vozes femininas, masculinas, lead, backing e coros,
podcast, narração, locução, caixa, bumbo, toms, overheads, sala, bus paralelo,
pandeiro, shaker, congas, cajón, violão aço/nylon, guitarra base/solo, baixo,
violino, cello, cordas, piano, Rhodes, Wurlitzer, synth lead, arpejos, pads,
Hammond, sax, trompete, trombone, gaita e acordeon.

Selecionar um programa aplica os controles abaixo. Nos presets XML,
Sensitivity e Output são preservados conforme IgnoreOnPresetLoad. Os mesmos parâmetros
aparecem para automação da DAW, com IDs estáveis. O mapeamento não depende
do nome da pista, de MIDI Learn ou de detecção do instrumento.

| IDs | Tipo |
| --- | --- |
| `input`, `output`, `balance` | Contínuos |
| `bass`, `treble`, `compress`, `reverb1`, `reverb2`, `delay` | Macros contínuas |
| `bassColour`, `trebleColour`, `compressColour`, `reverb1Colour`, `reverb2Colour`, `delayColour` | Enum 0–3 |
| `predelay1`, `predelay2` | Escolha 0–13 de divisão musical (padrão 7 = 16) |
| `predelay1On`, `predelay2On`, `direct`, `bypass` | Booleanos |

São **21 parâmetros** no total (3 + 6 + 6 + 2 + 4).
O catálogo completo, com todos os valores, está em
`Presets/factory-presets.json`. Para regenerá-lo: execute `preset_export`.

O preset de bus paralelo é para uma pista auxiliar misturada com a bateria
original; comece com o fader do auxiliar baixo. Os presets não fazem análise
de loudness e não garantem ausência de clipping: ajuste Input/Output pelo
material. Não há limiter oculto.

## Imagem preservada

`Assets/cla-unplugged-original.png` é uma cópia byte a byte da imagem aprovada:

```text
SHA-256 1179a8a2e3db3da08a19e2cb31396130cd84ed0928a6b606129ef62ab0a75ba3
```

O PNG não é editado. Faders, LEDs, rótulos de modos e medidores são desenhados
em camadas durante a execução para responder ao áudio e à automação. O
seletor de presets e o bypass ficam em uma barra fina acima da arte principal.
O menu segue a referência: Female Vocals, Male Vocals, Lead Vocals,
Backing & Choir, Speech, Drums, Percussion, Guitars & Bass, Strings,
Keys & Synths e Brass & Winds. Os knobs têm aro de latão escovado em relevo,
textura de alta resolução e indicador branco vetorial; os botões possuem relevo e estados
aceso/apagado ligados aos parâmetros da DAW.

## Compilar no Windows

Requisitos: Visual Studio 2022 com Desktop development with C++, Windows SDK,
CMake ≥3.22 e Git. Na pasta deste projeto:

Você pode executar `./build-windows.ps1` no PowerShell para configurar,
compilar, testar e gerar o ZIP automaticamente. Alternativamente:

```powershell
cmake --preset windows-x64
cmake --build --preset windows-x64 --parallel 3
ctest --preset windows-x64
```

JUCE **8.0.12** é obtido pelo commit fixo
`29396c22c93392d6738e021b83196283d6e4d850`.
Um checkout já existente pode ser usado com `-DJUCE_DIR=C:/caminho/JUCE`.
Os artefatos ficam em `build/windows-x64/UnpluggedStudy_artefacts/Release/`.
A workflow `.github/workflows/windows.yml` automatiza build, testes e pacote
de artefatos no Windows. Tags `v*` também publicam o ZIP e seu checksum em
GitHub Releases, somente depois dos testes e da verificação PE AMD64.
Pull requests e execução manual produzem artefatos sem criar release.

Para testar apenas o DSP em Linux/macOS/Windows, sem JUCE ou hardware:

```sh
cmake -S . -B build-dsp -DUNPLUGGED_BUILD_PLUGIN=OFF
cmake --build build-dsp --parallel 3
ctest --test-dir build-dsp --output-on-failure
```

Para diagnóstico de memória em GCC/Clang, acrescente
`-DUNPLUGGED_SANITIZERS=ON`.

O JUCE 8.0.12 rejeita MinGW; a tentativa de compilação cruzada foi interrompida
por essa restrição explícita. O destino Windows usa MSVC x64. A máquina atual
é Linux, e o acesso ao SDK Microsoft também retornou bloqueio de rede:
`download.visualstudio.microsoft.com` foi incluído no rascunho da configuração.
O build Windows foi concluído posteriormente no GitHub Actions, com Visual
Studio 2022, testes automáticos e publicação da versão v0.1.0. O ZIP publicado
foi baixado novamente e teve checksum e cabeçalhos PE AMD64 verificados.

## Estado da fidelidade e validação

Os XMLs e o PDF fornecidos foram usados para conferir os parâmetros expostos,
padrões e os 33 presets da categoria XML Reference. Os índices de pré-delay
são divisões musicais, não milissegundos. Consulte `Docs/XML-ALIGNMENT.md`
para o mapeamento, compatibilidade e limites da reprodução sonora.
Teste em DAW Windows, hardware e comparação auditiva com o original continuam pendentes.

O código identifica-se como **Unplugged Study**, com IDs próprios; não substitui
o binário ou o identificador VST3 do produto Waves. A licença do JUCE e os
direitos da arte/marcas devem ser considerados para qualquer distribuição.
