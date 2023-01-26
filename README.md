# Process VM SVEU16
 
| OP | Operation | Mnemonic | Comment |
| ------ | ------ | ------ | ------ |
| 0000 | DEST = MEM[SRC2] | LOD R1,R2,R3|Reading from memory, memory from position SRC2 is written to destination register|
| 0001 | DEST = SRC1 + SRC2 | ADD R1,R2,R3|Addition|
| 0010 | DEST = SRC1 - SRC2 | SUB R1,R2,R3|Subtraction|
| 0011 | DEST = SRC1 & SRC2 | AND R1,R2,R3|Logical conjunction|
| 0100 | DEST = SRC1 \| SRC2 | ORA R1,R2,R3|Logical disjunction|
| 0101 | DEST = SRC1 ^ SRC2 | XOR R1,R2,R3|Exclusive disjunction|
| 0110 | DEST = SRC1 >>> SRC2<sub>0-3</sub> <br> DEST = SRC1 >> SRC2<sub>0-3</sub> <br> DEST = SRC1 rot SRC2<sub>0-3</sub> <br> DEST = SRC1 << SRC2<sub>0-3</sub>| SHR R1,R2,R3|Rotates SRC1 left/right dependent on the bits 0-3, while 4-5 chooses the type of rotation |
| 0111 | DEST = SRC1 * SRC2 | MUL R1,R2,R3|Multiplication|
| 1000 | MEM[SRC2] = SRC1 <br> DEST = SRC1 | STO R1,R2,R3|SRC1 is written to destination register and memory from position SRC2|
| 1001 | IF SRC1 != 0 <br> DEST = SRC2 | MIF R1,R2,R3|SRC2 is written to destination register if SRC1 not 0|
| 1010 | DEST = SRC1 > SRC2 | GTU R1,R2,R3|Destination register is 1 if SRC1>SRC2 as unsigned number, else 0|
| 1011 | DEST = +-SRC1 > +-SRC2 | GTS R1,R2,R3|Destination register is 1 if SRC1>SRC2 as signed number, else 0|
| 1100 | DEST = SRC1 < SRC2 | LTU R1,R2,R3|Destination register is 1 if SRC1<SRC2 as unsigned number, else 0|
| 1101 |  DEST = +-SRC1 < +-SRC2 | LTS R1,R2,R3|Destination register is 1 if SRC1<SRC2 as signed number, else 0|
| 1110 | DEST = SRC1 == SRC2 | EQU R1,R2,R3|Destination register is 1 if SRC1==SRC2, else 0|
| 1111 | DEST = SRC1 <br> PC = SRC2 | MAJ R1,R2,R3|SRC1 is written to destination register and program counter becomes SRC2|
