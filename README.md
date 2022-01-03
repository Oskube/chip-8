# Chip-8 emulator
Chip-8 emulator with own assembler and disassembler. GUI for this emulator is implemented with [raylib](https://github.com/raysan5/raylib). Raylib version 4.0.0 should work.

## Building project
### Dependecies
At the moment of writing, raylib which is used in for GUI implementation is unavailable from (official) Ubuntu package repositories. See guides in [raylib wiki](https://github.com/raysan5/raylib/wiki) to install it on your system.

### Compiling
Run ```make``` to compile this project.

## Usage
Compiling a chip-8 program and running the binaries:
```
./assembler chip8_res/button-test.asm button-test.ch8
./emulator button-test.ch8 
```

Disassembling a chip-8 binary:
```
./assembler button-test.ch8 button-test.asm.2 -d
```

### Emulator key bindings
```ESC``` will quit the emulator.

Input keys:
```
1-2-3-4
q-w-e-r
a-s-d-f
z-x-c-v
```

## List of supported opcodes
x, y = Number of register V used  
nnn = 12-bit value  
nn  = 8-bit  value  
n   = 4-bit  value  

| Opcode | Mnemonic       | Description                  |
| ------ | -------------- | ---------------------------- |
| 00E0   | CLS            | Clear the display            |
| 00EE   | RET            | Return from a subroutine     |
| 1nnn   | JMP  nnn       | Jump to a location nnn       |
| 2nnn   | CALL nnn       | Call subroutine at nnn       |
| 3xnn   | SE   Vx, nn    | Skip next instruction if Vx == nn |
| 4xnn   | SNE  Vx, nn    | Skip next instruction if Vx != nn |
| 5xy0   | SE   Vx, Vy    | Skip next instruction if Vx == Vy |
| 6xnn   | MOV  Vx, nn    | Vx  = nn |
| 7xnn   | ADD  Vx, nn    | Vx += nn |
| 8xy0   | MOV  Vx, Vy    | Vx  = Vy |
| 8xy1   | OR   Vx, Vy    | Vx |= Vy |
| 8xy2   | AND  Vx, Vy    | Vx &= Vy |
| 8xy3   | XOR  Vx, Vy    | Vx ^= Vy |
| 8xy4   | ADD  Vx, Vy    | Vx += Vy, if overflow  Vf = 1     |
| 8xy5   | SUB  Vx, Vy    | Vx -= Vy, if no borrow Vf = 1     |
| 8xy6   | SHR  Vx, Vy    | Vx >>= 1, if LSB is 1 set Vf = 1  |
| 8xy7   | SUBN Vx, Vy    | Vx = Vy - Vx, if no borrow Vf = 1 |
| 8xyE   | SHL  Vx, Vy    | Vx <<= 1, if MSB is 1 set Vf = 1  |
| 9xy0   | JNE  Vx, Vy    | Vx != Vy, skip next instr    |
| Annn   | MOV  nnn       | I = nnn                      |
| Bnnn   | JMP  Vx, nnn   | Jump to Vx + nnn             |
| Cxnn   | RND  Vx, nn    | Vx = RND & nn                |
| Dxyn   | DRW  Vx, Vy, n | Draw sprite with height n from RAM[I] to position Vx,Vy |
| Ex9E   | KE   Vx        | Skip next instruction if key Vx pressed     |
| ExA1   | KNE  Vx        | Skip next instruction if key Vx not pressed |
| Fx07   | MOV  Vx, DT    | Vx = delay timer         |
| Fx0A   | MOV  Vx, K     | Vx = Key press, blocking |
| Fx15   | MOV  DT, Vx    | DT = Vx                  |
| Fx18   | MOV  ST, Vx    | ST = Vx                  |
| Fx1E   | ADD  I, Vx     | I += Vx                  |
| Fx29   | MOV  F, Vx     | I = location of sprite for digit Vx |
| Fx33   | MOV  B, Vx     | Store BCD representation of Vx to memory starting from RAM[I] |
| Fx55   | MOV  [I], Vx   | RAM[I...] = V0 ... Vx |
| Fx65   | MOV  Vx, [I]   | Vx = RAM[I]           |
