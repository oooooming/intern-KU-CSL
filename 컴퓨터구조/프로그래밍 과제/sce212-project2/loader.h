#ifndef __LOADER_H__
#define __LOADER_H__

#include "proc.h"

extern struct MIPS32_proc_t g_processor;


void        load_inst_to_mem(const char *buffer, const int index);
void        load_data_to_mem(const char *buffer, const int index);
void        load_program(char *program_filename);

#endif
