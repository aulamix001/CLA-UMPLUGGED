# Alinhamento com os XMLs e o manual enviados

O banco contém 185 programas: os índices 0–151 foram mantidos e os 33 registros
do arquivo de presets CLUN foram acrescentados em 152–184, na categoria XML Reference.
`Presets/Reference/parameters.xml` define a ordem dos 71 campos de cada registro;
`factory.xml` conserva os valores de referência. Somente os 21 parâmetros
expostos pelo projeto são mapeados; medidores, placeholders e bypasses de
componentes internos do fabricante não são parâmetros deste DSP independente.
`Tools/ImportReferencePresets.py` gera `Source/ReferencePresets.h` por nome de
parâmetro, sem executar o conteúdo dos anexos. O teste `reference_presets` compara
o exportador compilado com todos os valores correspondentes do XML.

## Controles

- Balance: −6 a +6 dB, passo 0,1; atenua o canal oposto em dB e só atua na entrada estéreo.
- Pré-delay: escolha discreta 0–13; índice 7 = 1/16, botões desligados no Init.
  As divisões seguem a ordem crescente de duração:
  `64T, 64, 32T, 64D, 32, 16T, 32D, 16, 8T, 16D, 8, 4T, 8D, 4`.
  T = tercina; D = pontuada. O XML fixa os índices 0–13 e o padrão 7;
  o manual fixa os extremos 64T–4 e o padrão 16. A ordem intermediária usa
  a convenção crescente de duração, pois a função proprietária DisplayPreDelay
  não foi fornecida. Não é uma tabela extraída de código do fabricante.
- O tempo efetivo segue o BPM da sessão; sem BPM, usa 120. O buffer cobre
  até uma semínima a 20 BPM (3 segundos), sem alocação durante processamento.
- Os seis seletores mantêm OFF e as três cores/nomenclaturas do manual.
- Ao selecionar um preset XML, Sensitivity e Output são preservados, conforme
  IgnoreOnPresetLoad. Os demais controles são aplicados; Main Mute = 0 significa Direct ligado.
- O preset Full Reset do XML mantém as cores verdes, conforme seus valores;
  ele é diferente do programa legado que desliga todos os módulos.

## Compatibilidade

O estado salvo passa à versão 2. Estados versão 1 com pré-delays em ms são
aproximados para a divisão musical mais próxima em 120 BPM; Balance é limitado
à faixa nova. Os presets legados também convertem seus tempos em ms para a
nota mais próxima nessa referência. Portanto, os índices são preservados,
mas os tempos antigos não são reproduzidos exatamente. Automações antigas da
DAW precisam de revisão: pré-delay passou de float em ms a escolha de nota,
e o intervalo normalizado de Balance mudou. Os IDs continuam os mesmos.
O JSON exportado passa a schemaVersion 2, com pré-delay expresso em índices.

## Limites

O manual e os XMLs descrevem controles, presets e uma cadeia de componentes,
não os algoritmos completos desses componentes. O EQ, compressor e reverbs
continuam implementações independentes. O Slap permanece a aproximação de
120 ms já existente: os arquivos não estabelecem sua fórmula completa.
Não há afirmação de equivalência sonora, reprodução do processamento fixo,
ou implementação da toolbar proprietária completa. O recurso 1100 recebido
é um arquivo de ícones Windows, não código DSP.

## Validação

A suíte DSP mede a resposta de Balance em dB e a diferença de início da
reverberação nas 14 divisões, em 20/60/120/240 BPM, incluindo tempos acima
do antigo limite de 160 ms. Também testa áudio finito em todos os 185 presets,
em 44,1/48/96 kHz, bypass, Direct, EQ, compressor e ausência de alocações.
A suíte JUCE cobre mapeamento de parâmetros, estado antigo, ganhos preservados,
interface, menu completo e carregamento/processamento do VST3 compilado.

Resultados nesta instância Linux: 2/2 testes CTest no build DSP normal,
2/2 com ASan/UBSan e 3/3 no build completo (DSP, comparação XML e processador).
O build completo produziu VST3 e standalone. O teste do host JUCE 8.0.12 em
Debug termina com código 0 e todas as verificações funcionais aprovadas,
mas reporta retenção de VST3HostContextHeadless/RunLoop/AsyncUpdater no
encerramento. A fábrica VST3 do JUCE mantém referências ao contexto do host
em setHostContext; o aviso não foi resolvido nem foi tratado como prova de
ausência de vazamentos do host. O build Windows atualizado não foi executado.
