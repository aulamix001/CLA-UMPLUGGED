# Validação — 15/11/2026

## Executado com sucesso no ambiente Linux x86-64

- Build C++17: DSP, VST3 JUCE, standalone e ferramentas.
- `ctest --test-dir build-linux --output-on-failure`: **2/2 executáveis** aprovados.
- DSP: **12 casos** cobrindo estéreo, unity gain, bypass, Direct, ganhos em dB,
  tempo do delay, pré-delay, EQ, compressão, balance, silêncio, valores não
  finitos e ausência de alocações no processamento.
- Banco: **151 presets × 44,1 / 48 / 96 kHz**; parâmetros dentro das faixas,
  nomes únicos e saída finita nos blocos de áudio exercitados.
- DSP em Debug com **AddressSanitizer e UndefinedBehaviorSanitizer**, aprovado.
- Processador/interface: **6 verificações**: recuperação de estado/programa,
  mapeamento dos 21 parâmetros de todos os presets, mono→estéreo e bypass,
  criação/renderização da interface, ligação bidirecional entre fader e parâmetro,
  e carregamento do VST3 compilado por um host JUCE.
- O teste de host examina o VST3, instancia-o, verifica parâmetros/programas,
  processa um impulso e grava/restaura seu estado.
- Integridade da imagem: SHA-256 comparado com o arquivo aprovado; o CMake
  também verifica o hash antes de construir o plugin.

O primeiro teste com sanitizadores identificou um índice circular no limite
de arredondamento de ponto flutuante. O cálculo foi corrigido e a suíte completa
passou novamente, incluindo os 151 presets nas três taxas.

## Windows x64 — compilado, testado e publicado

- [Execução Windows x64 #1](https://github.com/aulamix001/CLA-UMPLUGGED/actions/runs/43760055832):
  concluída com sucesso, usando Visual Studio 2022 no runner Windows.
- O script `build-windows.ps1` configurou e compilou os destinos, executou as
  duas suítes de teste, verificou os binários PE AMD64 e gerou o ZIP e o SHA-256.
- O job de publicação conferiu o checksum e criou a
  [release v0.1.0](https://github.com/aulamix001/CLA-UMPLUGGED/releases/tag/v0.1.0).
- Commit compilado: `7f1e0838167e3263fad991c8d98342a79354db6d`.
- Depois da publicação, o ZIP foi baixado do endereço público do GitHub,
  descompactado e verificado novamente: VST3 DLL e standalone são Windows AMD64.
- A inspeção do VST3 confirmou os exports `GetPluginFactory`, `InitDll` e
  `ExitDll`, além da dependência do runtime Microsoft Visual C++ v14 x64.
- SHA-256 de `Unplugged-Study-Windows-x64.zip`:
  `5556e0d3864d7be5ef2c03a0122de86b6523a8300342245c62bdda46c0363723`.

## Diagnóstico local anterior e limites de fidelidade

- A tentativa de build cruzado falhou porque o JUCE 8.0.12 recusa MinGW
  explicitamente. Não foi desativada essa verificação do framework.
- O SDK Microsoft não pôde ser obtido: o redirecionamento oficial de
  `aka.ms` para `download.visualstudio.microsoft.com` recebeu HTTP 403
  da política de rede. O domínio foi acrescentado ao rascunho das configurações.
- Esses bloqueios da máquina Linux foram resolvidos para a entrega usando
  o runner Windows do GitHub Actions, conforme o resultado acima.
- Ainda não houve teste em DAW Windows, hardware de áudio real ou comparação
  auditiva com o plugin comercial. O DSP é uma implementação independente.
- As páginas enviadas têm baixa resolução, e o PDF oficial continua inacessível.
  A auditoria integral do manual e dos valores iniciais permanece pendente.

O PNG de interface é uma captura de teste do componente JUCE, não uma captura
de uma DAW Windows.
