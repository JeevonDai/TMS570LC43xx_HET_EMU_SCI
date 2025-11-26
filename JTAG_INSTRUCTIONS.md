Instructions Accepted by the ICEPick TAP
The ICEPick-C TAP has a 6-bit instruction register (IR) to hold the current instruction. This TAP can execute the instructions shown in Table 1. Before using the ROUTER instruction, the debugger must use the CONNECT instruction to put the ICEPick module in the connected state. Otherwise, the ICEPick
module is in the disconnected state, and the ROUTER instruction is executed as a BYPASS instruction.
Instructions other than the ROUTER instruction operate normally regardless of whether the ICEPick module is in the connected state. All reserved instructions act as bypass, but should not be used.

Table 1. Instructions Accepted by the ICEPick TAP
Opcode in IR Instruction Execution
000000b Reserved No
000001b Reserved (acts as BYPASS) No
000010b ROUTER Yes
000011b Reserved (acts as BYPASS) No
000100b IDCODE No
000101b ICEPICKCODE No
000110b Reserved (acts as BYPASS) No
000111b CONNECT No
001000b-111110b Reserved (do not use) No
111111b BYPASS No

While some instructions are straight-forward public instructions, we will elaborate on the private instructions: CONNECT and ROUTER.


2.1.3 CONNECT Instruction

The CONNECT instruction is used to read or modify the 4 LSBs of the debug connect register (see Table 2). The four bits must be changed together, not individually. When the 4 LSBs are 1001b, the ICEPick module is in the connected state. If the 4 LSBs are any other value, the ICEPick module is in the
disconnected state, and the ICEPick TAP interprets the ROUTER opcode as a BYPASS opcode. The CONNECT instruction can be thought of as a protection key for accessing the mapped registers using the ROUTER instruction. For illustration on how the CONNECT register is accessed and the CONNECTKEY
is used to protect the connection to the mapped registers (see Figure 2). A power-on reset or a test-logic reset forces the 4 LSBs to 0110b, which selects the disconnected state.

Table 2. Debug Connect Register (DCON) Field Descriptions
Bit Field Value Description
7 WRITEENAB 0,1 Write Enable to CONNECTKEY field. A '1' in WRITEENABLE field opens the write to the LE CONNECTKEY field during Update-DR state.
6:4 Reserved 0 Reserved bits return 0s when read.
3:0 CONNECTK 0000b- Debug connect key. Write 1001b to put the ICEPick module in the connected state (ROUTER opcode EY 1111b enabled). Any other value written to this register puts the ICEPick module in the disconnected state (ROUTER opcode treated as BYPASS opcode)


2.1.3.1 Reading the Debug Connect Register
To read the debug connect register:
1. Move the ICEPick TAP to the Shift-IR state, shift in the 6-bit CONNECT instruction (000111b) to the IR register for six cycles
2. Move to the Update-IR state, the CONNECT instruction becomes the current instruction
3. Move the ICEPick TAP to the Shift-DR state, while in Shift-DR state shift in the 8 bits of register access information LSB first. Since the operation is to perform a read, bit 7 must be 0 and the rest of bits are don't care.
4. Once the TAP advances to the Update-DR state, the content of the Data Shift Register value is updated (written in parallel) to the DCON register. Since bit 7 indicating a read is written to the DCON register, bits[6:0] of the DCON register aren't affected in Update-DR state
5. Move the TAP to the Capture-DR state, the content of the DCON register is captured to the data shift register.
6. Move the TAP to the Shift-DR state, the content of the shift register which contains the value of the DCON register is shifted out.


2.1.3.2 Writing the Debug Connect Register
To write the debug connect register:
1. Move the ICEPick TAP to the Shift-IR state, shift in the 6-bit CONNECT instruction (000111b) to the IR register for six cycles
2. Move to the Update-IR state, the CONNECT instruction becomes the current instruction
3. Move the ICEPick TAP to the Shift-DR state, while in Shift-DR state shift in the 8 bits of register access information LSB first. Since the operation is to perform a write to the CONNECTKEY field, bit 7 must be 1. In order to open access to the mapped registers, a value of 1001b should be scanned into bits[3:0] of the DCON register. Bits[6:4] are don't care.
4. Once the TAP advances to the Update-DR state, the Data Shift Register value is updated (written in parallel) to the DCON register. If the ICEPick module is in the disconnected state, the ROUTER opcode is interpreted as a BYPASS opcode

2.1.4 ROUTER Instruction
There are four 24-bit mapped registers in ICEPICK-C implemented for the Hercules devices (see Figure 2 and Table 3). These registers can only be accessed when the ROUTER instruction (000010b) is the current IR instruction. To access the ROUTER instruction, the ICEPick module must first be in the connect state by following the instructions in Section 2.1.3.2.
For the ROUTER instruction, all 32 bits of the ICEPick data shift register are placed between TDI and TDO in the SELECT DR state. In the SHIFT DR state, the 32 bits shifted in must have the format shown in Figure 3. When a value is scanned in, bit 31 indicates whether the register is to be read or written, bits 30–24 indicate which register is to be accessed, and bits 23–0 contain the data (if any). When a value is scanned out, bit 31 indicates whether the previous write succeeded (0) or failed (1), bits 30–24 indicate which register was read, and bits 23–0 contain the data.

2.1.4.1 Reading a Mapped Register
To read a mapped register through the ICEPick TAP:
1. Make sure that CONNECT instruction has been issued to put the ICEPick in a connect state as depicted in Section 2.1.3.2.
2. Move the ICEPick TAP to Shift-IR state, shift in the ROUTER instruction (000010b) for six cycles
3. Move to the Update-IR state, the ROUTER instruction becomes the current instruction.
4. Move the ICEPick TAP to the Shift-DR state, while in the Shift-DR state, shift in the 32 bits of register access information. Bit 31 must be 0 for a read. For example, to read the SDTAP0 Reg for DAP, the value to be scanned in would be 0x20xxxxxx
5. Move the ICEPick TAP to the Update-DR state, and the mapped register to be selected for accessed is decoded according to the value of REG (bits[30-24] of the Data Shift register). and bits 23–0 are don't care bits.
6. Move the TAP to the Capture-DR state, the content of the selected register (SDTAP0 Reg) is updated (written in parallel) to the Data Shift register.
7. Move the TAP to the Shift-DR state, the content of the shift register which contains the value of the selected mapped register (SDTAP0 Reg) is shifted out. While the value is being shifted out, new register-access information (for a read or a write) can be shifted in.
8. Perform another DR scan to capture and shift out the content of the register.
Multiple registers can be read in sequence without the need to scan in another ROUTER instruction.


2.1.4.2 Writing to a Mapped Register
To load a mapped register through the ICEPick TAP:
1. Make sure that CONNECT instruction has been issued to put the ICEPick in a connect state as depicted in Section 2.1.3.2.
2. Move the ICEPick TAP to Shift-IR state, shift in the ROUTER instruction (000010b) for six cycles.
3. Move to the Update-IR state, the ROUTER instruction becomes the current instruction.
4. Move the ICEPick TAP to the Shift-DR state, while in the Shift-DR state, shift in the 32 bits of register access information. Bit 31 must be 1 for a write. For example, to write the SYS_CNTL Reg, the value to be scanned in would be 0x81xxxxxx where xxxxxx is the value to be written to SYS_CNTL register.
5. When the TAP advances to the UPDATE DR state, the register-access information according to the REG field in bits[30:24] is decoded and the data is written to the register.

Multiple registers can be loaded in sequence without the need to scan in another ROUTER instruction. In this case, after each value is written, the output of the subsequent scan shows the value written 


2.1.5.3 Selecting DAP TAP
As mentioned in the beginning, the ICEPick TAP is the only TAP in the JTAG scan chain after reset. To select other TAP such as the TAP for the Debug Access Port (DAP) the tools must write to the SDTAP0 register. In addition, the device must not be in a secured state (see Section 2.2). Details of selecting DAP TAP can be found in [4]. Follow the steps below:
1. Move the ICEPick TAP to the Shift-IR state, shift in the 6-bit CONNECT instruction (000111b) to the IR
register for six cycles
2. Move to the Update-IR state, the CONNECT instruction becomes the current instruction
3. Move the ICEPick TAP to the Shift-DR state, while in Shift-DR state shift in the 8-bit value 10001001b with LSB first.
4. Move the ICEPick TAP to the Update-DR state, this operation will put the ICEPick in connect state.
5. Move the ICEPick TAP to the Shift-IR state again, shift in the 6-bit ROUTER instruction (000010b) to the IR register for six cycles
6. Move to the Update-IR state, the ROUTER instruction becomes the current instruction
7. Move the ICEPick TAP to the Shift-DR state, while in Shift-DR state shift in the 32-bit value a0002108h with LSB first.
• bit[31] = 1 indicates a write operation
• bit[30:24] = 0100000b selects SDTAP0
• bit[23:0] = 0x2108 is the value to be written to SDTAP0
8. Move the ICEPick TAP to the Update-DR state. The value 2108h is written to SDTAP0. The value of 2108h will:
• Bit[13]: Enable the debug logic associated with SDTAP0
• Bit[8]: Select SDTAP0 to be on the JTAG scan chain
• Bit[3]: Force active power and clock to the logic associated with SDTAP0. This bit may not be necessary since Hercules family is always powered for DAP and the CPU core.
9. Move the ICEPick TAP to the Shift-IR state again, shift in the 6-bit BYPASS instruction (111111b) to the IR register for six cycles.
10. Wait for a minimum of 10 TCLK cycles.
11. Starting from here the secondary debug TAP 0 is on the JTAG scan chain 


Once the SDTAP0 becomes part of the scan chain, the external tool needs to take into account the JTAG post-amble counts during IR scan and DR scans. The post-amble IR count is 6 bits to account for the ICEPick instruction register and the post-amble DR count is 1 bit to account for the ICEPick 1-bit bypass register. During post-amble IR scan, the external tool should keep other TAPs (the ICEPick) in bypass state by shifting in all '1'. The subsequent sections of this document describing the DAP operations assumes you have included the post-amble counts to your scan operations.