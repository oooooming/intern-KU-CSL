/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   SCE212 Ajou University                                    */
/*   sce212sim.c                                               */
/*                                                             */
/***************************************************************/

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/*          DO NOT MODIFY THIS FILE!                            */
/*          You should only the proc.c file!                    */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <getopt.h>

#include "proc.h"
#include "mem.h"
#include "loader.h"
#include "util.h"


/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename) {
    init_memory();
    load_program(program_filename);
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
    int debug_set = 0;
    char** tokens;
    int i = 100;		//for loop

    int num_inst_set = 0;
    int num_inst = 0;

    int mem_dump_set = 0;
    int start_addr = 0;
    int end_addr = 0;

    int opt;
    extern int optind;

    while ((opt=getopt(argc, argv, "m:dn:")) != -1) {
        switch (opt) {
            case 'm':
                mem_dump_set = 1;
                tokens = str_split(optarg,':');
                start_addr = (int)strtol(*(tokens), NULL, 16);
                end_addr = (int)strtol(*(tokens+1), NULL, 16);
                break;

            case 'd':
                debug_set = 1;
                break;

            case 'n':
                num_inst_set = 1;
                num_inst = atoi(optarg);
                break;
        }
    }


    if (optind+1 > argc) {
        fprintf(stderr,  "Usage: %s [-m start_addr:end_addr] [-d] [-n num_instr] MIPS_binary\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    initialize(argv[argc-1]);

    if (num_inst_set)
        i = num_inst;
    
    if (debug_set) {
        printf("[DEBUG] Simulating for %d cycles...\n\n", i);

        for(; i > 0; i--){
            cycle();
            rdump();

            if (mem_dump_set)
                mdump(start_addr, end_addr);
        }

    } else {
        run(i);
        rdump();

        if (mem_dump_set)
            mdump(start_addr, end_addr);
    }

    exit(EXIT_SUCCESS);
}
