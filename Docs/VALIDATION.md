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

## Windows e fidelidade

- A tentativa de build cruzado falhou porque o JUCE 8.0.12 recusa MinGW
  explicitamente. Não foi desativada essa verificação do framework.
- O SDK Microsoft não pôde ser obtido: o redirecionamento oficial de
  `aka.ms` para `download.visualstudio.microsoft.com` recebeu HTTP 403
  da política de rede. O domínio foi acrescentado ao rascunho das configurações.
- O preset CMake `windows-x64`, o script PowerShell e a workflow GitHub Actions
  foram preparados para MSVC/Visual Studio 2022; **não foram executados no Windows**.
- **Nenhum binário Windows é apresentado como compilado ou testado.**
- Ainda não houve teste em DAW Windows, hardware de áudio real ou comparação
  auditiva com o plugin comercial. O DSP é uma implementação independente.
- As páginas enviadas têm baixa resolução, e o PDF oficial continua inacessível.
  A auditoria integral do manual e dos valores iniciais permanece pendente.

O PNG de interface é uma captura de teste do componente JUCE, não uma captura
de uma DAW Windows.
