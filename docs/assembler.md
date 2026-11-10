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
| Caractere    | `'A'`   | 65    |

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

        ORG 128          ; dados a partir do endereço 128
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
