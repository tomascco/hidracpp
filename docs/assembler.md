# Assembler do Hidra: notas de uso (foco na Neander)

Este documento reúne o que o assembler do Hidra aceita, com foco na máquina
Neander. Ele cobre recursos que já existiam, mas não estavam documentados em
lugar fácil de achar, e os recursos adicionados depois.

## Sintaxe básica

```
; comentário até o fim da linha
label:  MNEMÔNICO operando   ; comentário
```

- Mnemônicos, diretivas e labels **não diferenciam maiúsculas de minúsculas**.
- Um label pode ficar sozinho numa linha. Nesse caso ele aponta para o próximo
  byte emitido:
  ```
  loop:
          LDA x
  ```
- Labels começam com letra ou `_` e depois podem ter letras, dígitos e `_`.

### Comentários
- Tudo que vem depois de `;` é comentário.
- Um `;` dentro de uma string entre aspas simples não inicia comentário:
  `DAB 'a;b'` grava três bytes.

### Números
| Forma        | Exemplo | Valor |
|--------------|---------|-------|
| Decimal      | `128`   | 128   |
| Decimal negativo | `-1` | 255 (em um byte) |
| Hexadecimal (estilo Hidra/Daedalus) | `h80` | 128 |
| Hexadecimal (estilo C) | `0x80`, `-0x01` | 128, 255 |
| Binário      | `0b00001010` | 10 |
| Caractere    | `'A'`   | 65    |

Os prefixos `0x` e `0b` podem ser escritos em maiúsculas ou minúsculas e
funcionam em qualquer lugar onde um número é aceito: `ORG 0x80`, `DB 0b1010`,
`LDA 0x80` e `LDA x+0x10`.

### Label com deslocamento
`label+n` e `label-n` referenciam o endereço do label somado ao deslocamento.
Isso permite apontar para o **operando** de uma instrução de 2 bytes, útil em
código automodificável:

```
ptr:    LDA 128      ; ptr = endereço da instrução, ptr+1 = operando (128)
        ...
        LDA ptr+1
        ADD um
        STA ptr+1    ; agora a instrução em ptr lê o endereço 129
```

### Label do operando: `label.op`
Todo label colocado numa instrução que tem operando (2 bytes ou mais) define
automaticamente um segundo label, `label.op`, que aponta para o byte do operando.
É o mesmo que `label+1`, mas deixa a intenção explícita:

```
ptr:    LDA 128      ; define ptr (instrução) e ptr.op (operando)
        ...
        LDA ptr.op
        ADD um
        STA ptr.op
```

- Também funciona com o label sozinho na linha anterior à instrução.
- Aceita deslocamento: `ptr.op+1`.
- Labels em instruções de 1 byte (`NOT`, `NOP`, `HLT`) e em diretivas (`DB`…)
  **não** geram `.op`. Usar `x.op` nesse caso dá erro de montagem.
- O nome `x.op` aparece na coluna *Label* da tabela de memória.
- Como `.` não é permitido em labels definidos pelo usuário, um `x.op` nunca
  conflita com um label definido à mão.

## Diretivas

| Diretiva | Uso | Efeito |
|---|---|---|
| `ORG` | `ORG 128` | Os próximos bytes passam a ser escritos a partir do endereço dado. |
| `DB`  | `x: DB 5` | Um byte com o valor indicado. Sem argumento, vale 0. |
| `DW`  | `w: DW 1000` | Uma palavra de 2 bytes. |
| `DAB` | `DAB 1, 2, 3` ou `DAB 'texto'` | Um vetor de bytes. Uma string vira um byte por caractere. |
| `DAB [n]` | `buf: DAB [10]` | Reserva `n` bytes sem inicializar. Eles ficam com 0. |
| `DAW` | `DAW 1, 2` / `DAW [n]` | O mesmo que `DAB`, mas com palavras de 2 bytes. |

Exemplo de organização típica para a Neander:

```
        ORG 0
        LDA x
        ADD y
        STA r
        HLT

        ORG 0x80         ; dados a partir do endereço 128
x:      DB 5
y:      DB 7
r:      DB 0
```

Memória que nenhuma linha do programa define fica com **0** depois da montagem.

## Neander: conjunto de instruções no Hidra

| Opcode (hex) | Mnemônico | Bytes | Efeito | Flags |
|---|---|---|---|---|
| `00` | `NOP`   | 1 | nada | – |
| `10` | `STA a` | 2 | `mem[a] ← AC` | – |
| `20` | `LDA a` | 2 | `AC ← mem[a]` | N, Z |
| `30` | `ADD a` | 2 | `AC ← AC + mem[a]` | N, Z |
| `40` | `OR a`  | 2 | `AC ← AC or mem[a]` | N, Z |
| `50` | `AND a` | 2 | `AC ← AC and mem[a]` | N, Z |
| `60` | `NOT`   | 1 | `AC ← not AC` | N, Z |
| `70` | `SUB a` | 2 | `AC ← AC − mem[a]` | N, Z |
| `80` | `JMP a` | 2 | `PC ← a` | – |
| `90` | `JN a`  | 2 | se N, `PC ← a` | – |
| `A0` | `JZ a`  | 2 | se Z, `PC ← a` | – |
| `F0` | `HLT`   | 1 | para a execução | – |

O nibble baixo do opcode é ignorado na execução. Por exemplo, `05` também
executa como `NOP`.

### Sobre o `SUB` na Neander
- A Neander original **não tem `SUB`**. Ele é uma extensão do Hidra que usa o
  mesmo opcode do Ahmes (`0111` → `70h`).
- O resultado é `(AC − mem[a]) mod 256`. Como a Neander só tem as flags N e Z,
  o `SUB` atualiza apenas essas duas. Carry, borrow e overflow não existem nesta
  máquina.
- Programas que usam `SUB` não rodam no WNeander original.

## Arquivos de memória

O menu de importar e exportar memória trabalha com dois formatos.

| Formato | Extensão | Importa | Exporta |
|---|---|---|---|
| Binário do Hidra/Daedalus | `.mem` | sim | sim (filtro *Memória do Hidra*) |
| Texto Verilog `$readmemh` / `$writememh` | `.hex` (também `.mem`, `.txt`) | sim | sim (filtro *Verilog $writememh*) |

Na **importação**, o formato é detectado pelo conteúdo: o binário do Hidra
sempre contém bytes nulos, e um arquivo de texto nunca contém. Por isso um
`.mem` gerado por uma ferramenta Verilog também abre.

### Formato `$readmemh` aceito
```
// comentário de linha
/* comentário
   de bloco */
@10          // próximo valor vai para o endereço 0x10
3A 0b        // um byte por token, em hexadecimal
f_f          // '_' é ignorado dentro do número
```
- Cada valor ocupa um byte (`00`–`FF`) e é escrito no endereço atual, que
  depois avança uma posição.
- `@endereço` (em hexadecimal) muda o endereço atual.
- **Endereços que não aparecem no arquivo ficam com 0.** Na importação
  parcial, isso vale só para a faixa de destino.
- Se o arquivo tiver qualquer erro (token não hexadecimal, valor maior que
  `FF`, endereço fora da memória ou comentário `/*` sem fechamento), nada é
  alterado e aparece a mensagem "Formato de arquivo inválido".
- Não há suporte a `x`/`z` nem a valores com mais de um byte por token.

### Formato gravado na exportação
Um byte por linha, em hexadecimal minúsculo, com um comentário de endereço a
cada 16 bytes. O arquivo pode ser lido diretamente em Verilog/SystemVerilog:

```systemverilog
logic [7:0] mem [0:255];
initial $readmemh("programa.hex", mem);
```
