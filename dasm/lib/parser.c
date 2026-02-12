#include "parser.h"
#include <string.h>
#include <dasm/keys.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>


/* TODO : Checks whether the label appears only in the first token of line*/

#define PARSER_MAX_LABEL_CNT 1000

struct dasm_label {
	char m_text[DASM_TOKEN_MAX_LEN];
	libdice_word_t m_address;
};

struct dasm_label_table {
	struct dasm_label m_labels[PARSER_MAX_LABEL_CNT];
	libdice_word_t m_label_cnt;
};

struct dasm_opcode_define {
	const char m_mnemonic[DASM_TOKEN_MAX_LEN];
	const enum LIBDICE_OPCODE_ m_opcode;
	const libdice_word_t m_operand_cnt;
};

static const struct dasm_opcode_define s_opcode_define_table[LIBDICE_OPCODE_CNT] = {
	{DASM_OPCODE_TXT_NOP,        LIBDICE_OPCODE_NOP,        0},

	{DASM_OPCODE_TXT_IADD,       LIBDICE_OPCODE_IADD,       5},
	{DASM_OPCODE_TXT_ISUB,       LIBDICE_OPCODE_ISUB,       5},
	{DASM_OPCODE_TXT_IMUL,       LIBDICE_OPCODE_IMUL,       5},
	{DASM_OPCODE_TXT_IDIV,       LIBDICE_OPCODE_IDIV,       5},
	{DASM_OPCODE_TXT_IREM,       LIBDICE_OPCODE_IREM,       5},

	{DASM_OPCODE_TXT_UMUL,       LIBDICE_OPCODE_UMUL,       5},
	{DASM_OPCODE_TXT_UDIV,       LIBDICE_OPCODE_UDIV,       5},
	{DASM_OPCODE_TXT_UREM,       LIBDICE_OPCODE_UREM,       5},

	{DASM_OPCODE_TXT_FADD,       LIBDICE_OPCODE_FADD,       5},
	{DASM_OPCODE_TXT_FSUB,       LIBDICE_OPCODE_FSUB,       5},
	{DASM_OPCODE_TXT_FMUL,       LIBDICE_OPCODE_FMUL,       5},
	{DASM_OPCODE_TXT_FDIV,       LIBDICE_OPCODE_FDIV,       5},

	{DASM_OPCODE_TXT_INEG,       LIBDICE_OPCODE_INEG,       3},
	{DASM_OPCODE_TXT_FNEG,       LIBDICE_OPCODE_FNEG,       3},

	{DASM_OPCODE_TXT_JMP,        LIBDICE_OPCODE_JMP,        2},
	{DASM_OPCODE_TXT_JMPA,       LIBDICE_OPCODE_JMPA,       2},
	{DASM_OPCODE_TXT_JMPN,       LIBDICE_OPCODE_JMPN,       2},

	{DASM_OPCODE_TXT_JMPZ,       LIBDICE_OPCODE_JMPZ,       4},
	{DASM_OPCODE_TXT_JMPZA,      LIBDICE_OPCODE_JMPZA,      4},
	{DASM_OPCODE_TXT_JMPZN,      LIBDICE_OPCODE_JMPZN,      4},

	{DASM_OPCODE_TXT_BAND,       LIBDICE_OPCODE_BAND,       5},
	{DASM_OPCODE_TXT_BOR,        LIBDICE_OPCODE_BOR,        5},
	{DASM_OPCODE_TXT_BXOR,       LIBDICE_OPCODE_BXOR,       5},
	{DASM_OPCODE_TXT_BLSHIFT,    LIBDICE_OPCODE_BLSHIFT,    5},
	{DASM_OPCODE_TXT_BRSHIFT,    LIBDICE_OPCODE_BRSHIFT,    5},
	{DASM_OPCODE_TXT_LRSHIFT,    LIBDICE_OPCODE_LRSHIFT,    5},
	{DASM_OPCODE_TXT_BNOT,       LIBDICE_OPCODE_BNOT,       3},

	{DASM_OPCODE_TXT_LAND,       LIBDICE_OPCODE_LAND,       5},
	{DASM_OPCODE_TXT_LOR,        LIBDICE_OPCODE_LOR,        5},
	{DASM_OPCODE_TXT_LNOT,       LIBDICE_OPCODE_LNOT,       3},

	{DASM_OPCODE_TXT_TOBIT,      LIBDICE_OPCODE_TOBIT,      3},

	{DASM_OPCODE_TXT_EQ,         LIBDICE_OPCODE_EQ,         5},
	{DASM_OPCODE_TXT_NEQ,        LIBDICE_OPCODE_NEQ,        5},

	{DASM_OPCODE_TXT_SET,        LIBDICE_OPCODE_SET,        2},
	{DASM_OPCODE_TXT_MSET,       LIBDICE_OPCODE_MSET,       4},
	{DASM_OPCODE_TXT_MOV,        LIBDICE_OPCODE_MOV,        4},

	{DASM_OPCODE_TXT_ITOF,       LIBDICE_OPCODE_ITOF,       3},
	{DASM_OPCODE_TXT_FTOI,       LIBDICE_OPCODE_FTOI,       3},

	{DASM_OPCODE_TXT_IGT,        LIBDICE_OPCODE_IGT,        5},
	{DASM_OPCODE_TXT_FGT,        LIBDICE_OPCODE_FGT,        5},
	{DASM_OPCODE_TXT_ILT,        LIBDICE_OPCODE_ILT,        5},
	{DASM_OPCODE_TXT_FLT,        LIBDICE_OPCODE_FLT,        5},

	{DASM_OPCODE_TXT_PUTS,       LIBDICE_OPCODE_PUTS,       1},
	{DASM_OPCODE_TXT_PUTI,       LIBDICE_OPCODE_PUTI,       2},
	{DASM_OPCODE_TXT_PUTU,       LIBDICE_OPCODE_PUTU,       2},
	{DASM_OPCODE_TXT_PUTC,       LIBDICE_OPCODE_PUTC,       2},
	{DASM_OPCODE_TXT_PUTF,       LIBDICE_OPCODE_PUTF,       2},

	{DASM_OPCODE_TXT_DEF,        LIBDICE_OPCODE_DEF,        1},
	{DASM_OPCODE_TXT_UNDEF,      LIBDICE_OPCODE_UNDEF,      1},

	{DASM_OPCODE_TXT_RAND,       LIBDICE_OPCODE_RAND,	2},


	{DASM_OPCODE_TXT_EOP,        LIBDICE_OPCODE_EOP,        0},
};


static void dasm_init_label_table(struct dasm_label_table rdwr_label_table[])
{
	rdwr_label_table->m_label_cnt = 0;
	memset(rdwr_label_table->m_labels, 0, sizeof(struct dasm_label) * PARSER_MAX_LABEL_CNT);
}

static bool dasm_get_label_address(const struct dasm_label_table *rd_label_table, const char c_label[], libdice_word_t *addr)
{
	libdice_word_t i = 0;

	for (i=0; i<rd_label_table->m_label_cnt; i++) {
		if (strcmp(rd_label_table->m_labels[i].m_text, c_label) == 0) {
			*addr = rd_label_table->m_labels[i].m_address;
			return true;
		}
	}

	return false;
}

static bool dasm_insert_label(struct dasm_label_table *rdwr_label_table, const char c_label[], const libdice_word_t c_addr)
{
	if (rdwr_label_table->m_label_cnt == PARSER_MAX_LABEL_CNT) {
		return false;
	}

	strncpy(rdwr_label_table->m_labels[rdwr_label_table->m_label_cnt].m_text, c_label, DASM_TOKEN_MAX_LEN);
	rdwr_label_table->m_labels[rdwr_label_table->m_label_cnt].m_address = c_addr;
	rdwr_label_table->m_label_cnt++;

	return true;	
}

static bool dasm_label_programme(struct dasm_label_table *rdwr_label_table, const struct dasm_token_line rd_token_lines[], const libdice_word_t c_token_lines_len)
{
	libdice_word_t token_lines_idx = 0;
	libdice_word_t tmp = 0;
	libdice_word_t pc = 0;
	
	dasm_init_label_table(rdwr_label_table);

	for (token_lines_idx=0; token_lines_idx<c_token_lines_len; token_lines_idx++) {
		const struct dasm_token_line *rd_token_line = &(rd_token_lines[token_lines_idx]);

		assert(rd_token_line->m_token_cnt > 0);
		if (rd_token_line->m_tokens[0].m_token_type == DASM_TOKEN_TYPE_LABEL) {
			/* label must be defined only once */
			if (dasm_get_label_address(rdwr_label_table, rd_token_line->m_tokens[0].m_text, &tmp)) {
				return false;
			}
			dasm_insert_label(rdwr_label_table, rd_token_line->m_tokens[0].m_text, pc);
		}
		pc += dasm_get_token_line_word_len(rd_token_line);

		if (rd_token_line->m_tokens[rd_token_line->m_token_cnt-1].m_token_type == DASM_TOKEN_TYPE_EOP) {
			return true;
		}
	}

	/* Couldn't find EOP */
	return false;
}

static bool dasm_get_opcode_table_idx(const char rd_mnemonic[], libdice_word_t *rdwr_idx)
{
	libdice_word_t idx = 0;
	for (idx=0; idx<LIBDICE_OPCODE_CNT; ++idx) {
		if (strcmp(s_opcode_define_table[idx].m_mnemonic, rd_mnemonic) == 0) {
			*rdwr_idx = idx;
			return true;;
		}
	}
	return false;
}

static void dasm_init_parsed_line(struct dasm_parsed_line *rdwr_parsed_line)
{
	rdwr_parsed_line->m_opcode = 0;
	memset(rdwr_parsed_line->m_operands, 0, sizeof(struct dasm_operand) * LIBDICE_OPERAND_MAX_CNT);
	rdwr_parsed_line->m_operand_cnt = 0;
}

/**
 * @return  1==success parsing, 0==failed parsing, DASM_ERR_RET==error
 */
static libdice_word_t dasm_parse_operand(struct dasm_operand *rdwr_operand, const struct dasm_token *rd_token, 
				const struct dasm_label_table *rd_label_table)
{
	switch (rd_token->m_token_type) 
		{
			case DASM_TOKEN_TYPE_IDENT:
			{
				libdice_word_t label_addr = 0;
				
				if (dasm_get_label_address(rd_label_table, rd_token->m_text, &label_addr)) {
					/* This token is label */
					int tmp = 0;
					tmp = snprintf(rdwr_operand->m_text, DASM_TOKEN_MAX_LEN, "%u", label_addr);
					if (tmp >= DASM_TOKEN_MAX_LEN || tmp<0) {
						return DASM_ERR_RET;
					}
					return 1;
				}
				/* This token is opcode */
				strncpy(rdwr_operand->m_text, rd_token->m_text, DASM_TOKEN_MAX_LEN);
				return 1;
			}
			case DASM_TOKEN_TYPE_NUMBER:
				strncpy(rdwr_operand->m_text, rd_token->m_text, DASM_TOKEN_MAX_LEN);
				return 1;
			case DASM_TOKEN_TYPE_LABEL:
				/* label must be handled by  dasm_parse_line. This means the label was placed in the middle of the line, not at the beginning*/
				return DASM_ERR_RET;
			case DASM_TOKEN_TYPE_STRING:
				/* Doesn't support string yet*/
				return DASM_ERR_RET;
			case DASM_TOKEN_TYPE_CHAR:
			{
				int tmp = 0;
				tmp = snprintf(rdwr_operand->m_text, DASM_TOKEN_MAX_LEN, "%u", (unsigned)(unsigned char)rd_token->m_text[1]);
				if (tmp >= DASM_TOKEN_MAX_LEN || tmp<0) {
					return DASM_ERR_RET;
				}
				return 1;
			}
			case DASM_TOKEN_TYPE_OPERATOR:
			{
				int tmp = 0;
				if (rdwr_operand->m_text[0] == '#') {
					if (strlen(rdwr_operand->m_text) == 1) {
						tmp = snprintf(rdwr_operand->m_text, DASM_TOKEN_MAX_LEN, "%u", (unsigned)0);
						if (tmp >= DASM_TOKEN_MAX_LEN || tmp<0) {
							return DASM_ERR_RET;
						}
						return 1;
					} else {
						return DASM_ERR_RET;
					}
				}
				tmp = snprintf(rdwr_operand->m_text, DASM_TOKEN_MAX_LEN, "%u", (unsigned)strlen(rd_token->m_text));
				if (tmp >= DASM_TOKEN_MAX_LEN || tmp<0) {
					return DASM_ERR_RET;
				}
				return 1;
			}
			case DASM_TOKEN_TYPE_EOL:
				return 0;
			case DASM_TOKEN_TYPE_EOP:
				return 0;
			default:
				assert(0);
				return DASM_ERR_RET;
		}
}

/**
 * @return 1==success parsing, 0==failed parsing, DASM_ERR_RET==error
 * * */ 
static libdice_word_t dasm_parse_line(struct dasm_parsed_line *rdwr_parsed_line, const struct dasm_token_line *rd_token_line, 
					const struct dasm_label_table *rd_label_table)
{
	libdice_word_t token_line_idx = 0;
	libdice_word_t opcode_table_idx = 0;
	const struct dasm_token *rd_token = NULL;
	libdice_word_t tmp_operand_cnt = 0;

	dasm_init_parsed_line(rdwr_parsed_line);

	if (rd_token_line->m_token_cnt == 0) {
		assert(0);
		return DASM_ERR_RET;	/* Shouldn't reach here*/
	}

	if (rd_token_line->m_token_cnt==1
		&& (rd_token_line->m_tokens[0].m_token_type==DASM_TOKEN_TYPE_EOL 
		|| rd_token_line->m_tokens[0].m_token_type==DASM_TOKEN_TYPE_EOP)) {
		return 0;
	}

	/* Skip label token */
	if (rd_token_line->m_tokens[0].m_token_type == DASM_TOKEN_TYPE_LABEL) {
		token_line_idx++;
	}

	rd_token = &(rd_token_line->m_tokens[token_line_idx]);
	token_line_idx++;

	/* Check whether the line contains only a label */
	if (rd_token->m_token_type==DASM_TOKEN_TYPE_EOL || rd_token->m_token_type==DASM_TOKEN_TYPE_EOP) {
		return 0;
	}
	
	/* opcode must be found in opcode_table */
	if (!dasm_get_opcode_table_idx(rd_token->m_text, &opcode_table_idx)) {
		return DASM_ERR_RET;
	}
	rdwr_parsed_line->m_opcode = s_opcode_define_table[opcode_table_idx].m_opcode;
	rdwr_parsed_line->m_operand_cnt = s_opcode_define_table[opcode_table_idx].m_operand_cnt;
	
	tmp_operand_cnt = 0;
	for (; token_line_idx<rd_token_line->m_token_cnt; token_line_idx++) {
		libdice_word_t tmp = 0;
		rd_token = &(rd_token_line->m_tokens[token_line_idx]);
		/* boundary check */

		tmp = dasm_parse_operand(&(rdwr_parsed_line->m_operands[tmp_operand_cnt]), rd_token, rd_label_table);
		if (tmp == DASM_ERR_RET) {
			return DASM_ERR_RET;
		}
		if (tmp) {
			if (tmp_operand_cnt+1 > LIBDICE_OPERAND_MAX_CNT) {
				return DASM_ERR_RET;
			}
			tmp_operand_cnt++;
		}
	}
	if (tmp_operand_cnt != rdwr_parsed_line->m_operand_cnt) {
		return DASM_ERR_RET;
	}
	return 1;
} 


DICEIMPL libdice_word_t dasm_parse_programme(struct dasm_parsed_line rdwr_parsed_lines[], const libdice_word_t c_parsed_lines_len, 
					const struct dasm_token_line rd_token_lines[], const libdice_word_t c_token_lines_len)
{
	libdice_word_t token_line_cnt = 0;
	libdice_word_t parsed_line_cnt = 0;
	struct dasm_label_table label_table;

	dasm_init_label_table(&label_table);
	if (!dasm_label_programme(&label_table, rd_token_lines, c_token_lines_len)) {
		return DASM_ERR_RET;
	}

	for (token_line_cnt=0; token_line_cnt<c_token_lines_len; token_line_cnt++) {
		libdice_word_t tmp = 0;
		/* boundary check */
		if (parsed_line_cnt+1 > c_parsed_lines_len) {
			return DASM_ERR_RET;
		}
		tmp = dasm_parse_line(&rdwr_parsed_lines[parsed_line_cnt], &rd_token_lines[token_line_cnt], &label_table);
		if (tmp == DASM_ERR_RET) {
			return DASM_ERR_RET;
		}
		if (tmp) {
			parsed_line_cnt++;
		}
 	}

	return parsed_line_cnt;
	


}
