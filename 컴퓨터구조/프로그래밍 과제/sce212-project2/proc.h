#ifndef __PROC_H__
#define __PROC_H__

#include <stdint.h>

/* Basic Information */
#define MIPS_REGS      32
#define BYTES_PER_WORD 4


struct MIPS32_proc_t {
    uint32_t pc;                /* Program counter */
    uint32_t regs[MIPS_REGS];    /* Register file */

    uint32_t num_insts;         /* Number of executed instructions */
    uint32_t ticks;             /* Clock ticks (cycles) */

    int      running;           /* Runnning or halted */
    int      input_insts;       /* Size of input instructiosn */
};

/* You should decode your instructions from the
 * ASCII-binary format to this structured format */
struct inst_t {
    short opcode;

    /*R-type*/
    short func_code;

    union {
        /* R-type or I-type: */
        struct {
	    unsigned char rs;
	    unsigned char rt;

	    union {
	        short imm;

	        struct {
		    unsigned char rd;
		    unsigned char shamt;
		} r;
	    } r_i;
	} r_i;
        /* J-type: */
        uint32_t target;
    } r_t;

    uint32_t value;
};


void		    rdump();
void		    run(int num_cycles);
void		    go();

/* key functions */
void            execute(struct inst_t);
struct inst_t   decode(int inst_word);
int             fetch(uint32_t pc);
void		    cycle();

#define OPCODE(INST)		(INST).opcode
#define FUNC(INST)		    (INST).func_code
#define RS(INST)		    (INST).r_t.r_i.rs
#define RT(INST)		    (INST).r_t.r_i.rt
#define RD(INST)		    (INST).r_t.r_i.r_i.r.rd
#define FS(INST)		    RD(INST)
#define FT(INST)		    RT(INST)
#define SHAMT(INST)		    (INST).r_t.r_i.r_i.r.shamt
#define IMM(INST)		    (INST).r_t.r_i.r_i.imm
#define BASE(INST)		    RS(INST)
#define IOFFSET(INST)		IMM(INST)
#define SET_IOFFSET(INST, VAL)	SET_IMM(INST, VAL)
#define IDISP(INST)		(SIGN_EX (IOFFSET (INST) << 2))

#define COND(INST)		RS(INST)
#define CC(INST)		(RT(INST) >> 2)
#define ND(INST)		((RT(INST) & 0x2) >> 1)
#define TF(INST)		(RT(INST) & 0x1)

#define TARGET(INST)		(INST).r_t.target
#define ENCODING(INST)		(INST).encoding
#define EXPR(INST)		(INST).expr
#define SOURCE(INST)		(INST).source_line

/* Sign Extension */
#define SIGN_EX(X) (((X) & 0x8000) ? ((X) | 0xffff0000) : (X))

#define COND_UN		0x1
#define COND_EQ		0x2
#define COND_LT		0x4
#define COND_IN		0x8


/* Minimum and maximum values that fit in instruction's imm field */
#define IMM_MIN		0xffff8000
#define IMM_MAX 	0x00007fff

#define UIMM_MIN  	(unsigned)0
#define UIMM_MAX  	((unsigned)((1<<16)-1))

#define BRANCH_INST(TEST, TARGET, NULLIFY)	\
{						\
    if (TEST)					\
    {						\
	uint32_t target = (TARGET);		\
	JUMP_INST(target)			\
    }						\
}


#define JUMP_INST(TARGET)			\
{						\
    g_processor.pc = (TARGET);		\
}

#define LOAD_INST(DEST_A, LD, MASK)		\
{						\
    LOAD_INST_BASE (DEST_A, (LD & (MASK)))	\
}

#define LOAD_INST_BASE(DEST_A, VALUE)		\
{						\
    *(DEST_A) = (VALUE);			\
}

#endif
