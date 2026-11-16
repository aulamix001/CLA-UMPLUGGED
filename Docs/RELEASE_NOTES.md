Plugin VST3 e aplicativo standalone **Windows x64 — v0.1.3**.

- Balance corrigido para −6 a +6 dB.
- Pré-delays com 14 divisões musicais sincronizadas ao BPM; padrão 1/16, desligados no Init.
- 33 presets dos XMLs acrescentados: 185 programas, mantendo os índices 0–151.
- Sensitivity e Output preservados ao carregar os presets XML.
- Migração dos estados antigos de pré-delay para a nota mais próxima em 120 BPM.

Extraia o ZIP e copie a pasta completa `VST3/Unplugged Study.vst3` para
`C:\Program Files\Common Files\VST3\`. Atualize a lista de plugins na DAW.
O aplicativo separado fica em `Standalone`. Requer Windows 10/11 x64 e
Microsoft Visual C++ Runtime x64: https://aka.ms/vs/17/release/vc_redist.x64.exe

**Compatibilidade:** revise automações antigas de pré-delay e Balance, pois suas
faixas foram corrigidas. Os tempos antigos em ms são aproximados para notas.

O workflow exige build, testes no Windows, cabeçalhos PE AMD64 e checksum antes
da publicação. O arquivo `.sha256` permite conferir a integridade do download.
O DSP é independente: não há equivalência sonora comprovada com o Waves.
A ordem intermediária das divisões de pré-delay segue a convenção crescente de
duração; a função proprietária de exibição não foi fornecida. Slap continua
como aproximação de 120 ms. Consulte `Docs/XML-ALIGNMENT.md`.
