#ifndef libdasm_assembler_h
#define libdasm_assembler_h

#include "parser.h"



libdice_word_t libdasm_assemble_programme(libdice_word_t rdwr_programme[], const libdice_word_t c_programme_len, 
					const struct libdasm_parsed_line rd_parsed_lines[], const libdice_word_t parsed_line_len);




#endif /* libdasm_assembler_h */