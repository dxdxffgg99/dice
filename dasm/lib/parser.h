#ifndef libdasm_parser_h
#define libdasm_parser_h

#include <libdice/type.h>
#include <libdice/opcode.h>
#include <ae2f/c90/StdBool.h>
#include "tokenizer.h"


struct libdasm_operand {
	char m_text[LIBDASM_TOKEN_MAX_LEN];
};

struct libdasm_parsed_line {
	enum LIBDICE_OPCODE_ m_opcode;
	struct libdasm_operand m_operands[LIBDICE_OPERAND_MAX_CNT];
	libdice_word_t m_operand_cnt;
};



libdice_word_t libdasm_parse_programme(struct libdasm_parsed_line rdwr_parsed_lines[], const libdice_word_t c_parsed_lines_len, 
					const struct libdasm_token_line rd_token_lines[], const libdice_word_t c_token_lines_len );

#endif /* libdasm_parser_h */