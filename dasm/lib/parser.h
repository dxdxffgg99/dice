#ifndef dasm_parser_h
#define dasm_parser_h

#include <dasm.h>
#include <libdice/type.h>
#include <libdice/opcode.h>
#include <ae2f/c90/StdBool.h>
#include "tokenizer.h"


struct dasm_operand {
	char m_text[DASM_TOKEN_MAX_LEN];
};

struct dasm_parsed_line {
	enum LIBDICE_OPCODE_ m_opcode;
	struct dasm_operand m_operands[LIBDICE_OPERAND_MAX_CNT];
	libdice_word_t m_operand_cnt;
};

DICECALL libdice_word_t dasm_parse_programme(struct dasm_parsed_line rdwr_parsed_lines[], const libdice_word_t c_parsed_lines_len, 
					const struct dasm_token_line rd_token_lines[], const libdice_word_t c_token_lines_len );

#endif /* dasm_parser_h */
