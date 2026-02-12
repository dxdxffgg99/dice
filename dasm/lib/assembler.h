#ifndef dasm_assembler_h
#define dasm_assembler_h

#include "parser.h"
#include <dasm.h>


DICECALL libdice_word_t dasm_assemble_programme(libdice_word_t rdwr_programme[], const libdice_word_t c_programme_len, 
					const struct dasm_parsed_line rd_parsed_lines[], const libdice_word_t c_parsed_lines_len);




#endif /* dasm_assembler_h */
