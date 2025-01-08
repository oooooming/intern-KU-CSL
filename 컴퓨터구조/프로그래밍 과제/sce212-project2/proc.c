/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   SCE212 Ajou University                                    */
/*   proc.c                                                    */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <malloc.h>

#include "proc.h"
#include "mem.h"
#include "util.h"

/***************************************************************/
/* System (CPU and Memory) info.                                             */
/***************************************************************/
struct MIPS32_proc_t g_processor;


/***************************************************************/
/* Fetch an instruction indicated by PC                        */
/***************************************************************/
int fetch(uint32_t pc)
{
    return mem_read_32(pc);
}

/***************************************************************/
/* TODO: Decode the given encoded 32bit data (word)            */
/***************************************************************/
struct inst_t decode(int word)
{
    struct inst_t inst;

    inst.opcode = (word >> 26) & 0x3F;

    if (inst.opcode == 0) { // R-type
        inst.func_code = word & 0x3F;
        inst.r_t.r_i.rs = (word >> 21) & 0x1F;
        inst.r_t.r_i.rt = (word >> 16) & 0x1F;
        inst.r_t.r_i.r_i.r.rd = (word >> 11) & 0x1F;
        inst.r_t.r_i.r_i.r.shamt = (word >> 6) & 0x1F; 
    }
    else if (inst.opcode == 2 || inst.opcode == 3) { // J-type
        inst.r_t.target = word & 0x03FFFFFF;
    }
    else { // I-type
        inst.r_t.r_i.rs = (word >> 21) & 0x1F;
        inst.r_t.r_i.rt = (word >> 16) & 0x1F;
        inst.r_t.r_i.r_i.imm = word & 0xFFFF;
    }

    inst.value = word;

    return inst;
}

/***************************************************************/
/* TODO: Execute the decoded instruction                       */
/***************************************************************/
void execute(struct inst_t inst)
{
    if (inst.opcode == 0) { // R-type
        switch (inst.func_code) {
            case 0x21: // ADDU
                g_processor.regs[RD(inst)] = g_processor.regs[RS(inst)] + g_processor.regs[RT(inst)];
                break;

            case 0x24: // AND
                g_processor.regs[RD(inst)] = g_processor.regs[RS(inst)] & g_processor.regs[RT(inst)];
                break;

            case 0x08: // JR
                g_processor.pc = g_processor.regs[RS(inst)];
                break;

            case 0x27: // NOR
                g_processor.regs[RD(inst)] = ~(g_processor.regs[RS(inst)] | g_processor.regs[RT(inst)]);
                break;

            case 0x25: // OR
                g_processor.regs[RD(inst)] = g_processor.regs[RS(inst)] | g_processor.regs[RT(inst)];
                break;

            case 0x2B: // SLTU
                g_processor.regs[RD(inst)] = (g_processor.regs[RS(inst)] < g_processor.regs[RT(inst)]) ? 1 : 0;
                break;

            case 0x00: // SLL
                g_processor.regs[RD(inst)] = g_processor.regs[RT(inst)] << SHAMT(inst);
                break;

            case 0x02: // SRL
                g_processor.regs[RD(inst)] = g_processor.regs[RT(inst)] >> SHAMT(inst);
                break;

            case 0x23: //SUBU
                g_processor.regs[RD(inst)] = g_processor.regs[RS(inst)] - g_processor.regs[RT(inst)];
                break;
        }
    }
    else if (inst.opcode == 2 || inst.opcode == 3) { // J-type
        switch (inst.func_code) {
            case 0x02: // J
                g_processor.pc = (g_processor.pc & 0xF0000000) | (TARGET(inst) << 2);
                return;

            case 0x03: //JAL
                g_processor.regs[31] = g_processor.pc + 4;
                g_processor.pc = (g_processor.pc & 0xF0000000) | (TARGET(inst) << 2);
                return;
        }
    }
    else { // I-type
        switch (inst.opcode) {
            case 0x09:  // ADDIU
                g_processor.regs[RT(inst)] = g_processor.regs[RS(inst)] + SIGN_EX(IMM(inst));
                break;

            case 0x0C: // ANDI
                g_processor.regs[RT(inst)] = g_processor.regs[RS(inst)] & (IMM(inst) & 0xFFFF);
                break;

            case 0x04: // BEQ
                if(g_processor.regs[RT(inst)] == g_processor.regs[RS(inst)]) 
                    g_processor.pc += (SIGN_EX(IMM(inst)) << 2);
                break;

            case 0x05: // BNE
                if (g_processor.regs[RT(inst)] != g_processor.regs[RS(inst)])
                    g_processor.pc += (SIGN_EX(IMM(inst)) << 2);
                break;

            case 0x0F: // LUI
                g_processor.regs[RT(inst)] = IMM(inst) << 16;
                break;

            case 0x23: // LW
                g_processor.regs[RT(inst)] = mem_read_32(g_processor.regs[RS(inst)] + SIGN_EX(IMM(inst)));
                break;

            case 0x0D: // ORI
                g_processor.regs[RT(inst)] = g_processor.regs[RS(inst)] | (IMM(inst) & 0xFFFF);
                break;

            case 0x0A: // SLTIU
                g_processor.regs[RT(inst)] = (g_processor.regs[RS(inst)] < SIGN_EX(IMM(inst))) ? 1 : 0;
                break;

            case 0x2B: // SW
                mem_write_32(g_processor.regs[RS(inst)] + SIGN_EX(IMM(inst)), g_processor.regs[RT(inst)]);
                break;
        }
    }

    g_processor.pc += 4;
}

/***************************************************************/
/* Advance a cycle                                             */
/***************************************************************/
void cycle()
{
    int inst_reg;
    struct inst_t inst;

    // 1. fetch
    inst_reg = fetch(g_processor.pc);
    g_processor.pc += BYTES_PER_WORD;

    // 2. decode
    inst = decode(inst_reg);

    // 3. execute
    execute(inst);

    // 4. update stats
    g_processor.num_insts++;
    g_processor.ticks++;
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump() {
    int k;

    printf("\n[INFO] Current register values :\n");
    printf("-------------------------------------\n");
    printf("PC: 0x%08x\n", g_processor.pc);
    printf("Registers:\n");
    for (k = 0; k < MIPS_REGS; k++)
        printf("R%d: 0x%08x\n", k, g_processor.regs[k]);
}



/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate MIPS for n cycles                      */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {
    int i;

    if (g_processor.running == FALSE) {
        printf("[ERROR] Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("[INFO] Simulating for %d cycles...\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
        if (g_processor.running == FALSE) {
            printf("[INFO] Simulator halted\n");
            break;
        }
        cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate MIPS until HALTed                      */
/*                                                             */
/***************************************************************/
void go() {
    if (g_processor.running == FALSE) {
        printf("[ERROR] Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("[INFO] Simulating...\n");
    while (g_processor.running)
        cycle();
    printf("[INFO] Simulator halted\n");
}
