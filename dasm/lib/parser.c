#include "parser.h"
#include <string.h>
#include <dasm/keys.h>
#include <dasm.h>
#include <stdlib.h>


/* TODO : Checks whether the label appears only in the first token of line*/

#define PARSER_MAX_LABEL_CNT 100

struct libdasm_label {
	char m_text[LIBDASM_TOKEN_MAX_LEN];
	libdice_word_t m_address;
};

struct libdasm_label_table {
	struct libdasm_label m_labels[PARSER_MAX_LABEL_CNT];
	libdice_word_t m_label_cnt;
};

struct libdasm_opcode_define {
	const char m_mnemonic[LIBDASM_TOKEN_MAX_LEN];
	const enum LIPDICE_OPCODE_ m_opcode;
	const libdice_word_t m_operand_cnt;
};

static const struct libdasm_opcode_define s_opcode_define_table[LIBDICE_OPCODE_CNT] = {
	{LIBDASM_OPCODE_TXT_SET, LIBDICE_OPCODE_SET, 2},
	{LIBDASM_OPCODE_TXT_NOP, LIBDICE_OPCODE_NOP, 0},
	{LIBDASM_OPCODE_TXT_EOP, LIBDICE_OPCODE_EOP, 0},
	{LIBDASM_OPCODE_TXT_IADD, LIBDICE_OPCODE_IADD, 5},
	{LIBDASM_OPCODE_TXT_ISUB, LIBDICE_OPCODE_ISUB, 5},
	{LIBDASM_OPCODE_TXT_PUTI, LIBDICE_OPCODE_PUTI, 2},
	{LIBDASM_OPCODE_TXT_JMP, LIBDICE_OPCODE_JMP, 2},
	{LIBDASM_OPCODE_TXT_JMPZ, LIBDICE_OPCODE_JMPZ, 4},
};

static void libdasm_init_label_table(struct libdasm_label_table *rdwr_label_table)
{
	rdwr_label_table->m_label_cnt = 0;
	memset(rdwr_label_table->m_labels, 0, sizeof(struct libdasm_label) * PARSER_MAX_LABEL_CNT);
}

static bool libdasm_get_label_address(const struct libdasm_label_table *rd_label_table, const char c_label[], libdice_word_t *addr)
{
	libdice_word_t i = 0;

	if (rd_label_table->m_label_cnt == 0) {
		return false;
	}

	for (i=0; i<rd_label_table->m_label_cnt; i++) {
		if (strcmp(rd_label_table->m_labels[i].m_text, c_label) == 0) {
			*addr = rd_label_table->m_labels[i].m_address;
			return true;
		}
	}

	return false;
}

static bool libdasm_insert_label(struct libdasm_label_table *rdwr_label_table, const char c_label[], const libdice_word_t c_addr)
{
	if (rdwr_label_table->m_label_cnt == PARSER_MAX_LABEL_CNT) {
		return false;
	}

	strcpy(rdwr_label_table->m_labels[rdwr_label_table->m_label_cnt].m_text, c_label);
	rdwr_label_table->m_labels[rdwr_label_table->m_label_cnt].m_address = c_addr;
	rdwr_label_table->m_label_cnt++;

	return true;	
}

static void libdasm_label_programme(struct libdasm_label_table *rdwr_label_table, const struct libdasm_token_line rd_token_lines[], const libdice_word_t c_token_lines_len)
{
	libdice_word_t line_idx = 0;
	libdice_word_t tmp = 0;
	libdice_word_t pc = 0;
	const struct libdasm_token_line *rd_token_line = NULL;

	for (line_idx=0; line_idx<c_token_lines_len; line_idx++) {
		rd_token_line = &(rd_token_lines[line_idx]);

		assert(rd_token_line->m_token_cnt > 0);
		if (rd_token_line->m_tokens[0].m_token_type == LIBDASM_TOKEN_TYPE_LABEL) {
			assert(!libdasm_get_label_address(rdwr_label_table, rd_token_line->m_tokens[0].m_token, &tmp));	/* label must be defined only once */
			libdasm_insert_label(rdwr_label_table, rd_token_line->m_tokens[0].m_token, pc);
		}
		pc += libdasm_get_token_line_len(rd_token_line);

		if (rd_token_line->m_tokens[rd_token_line->m_token_cnt-1].m_token_type == LIBDASM_TOKEN_TYPE_EOP) {
			break;
		}
	}
}

static bool libdasm_get_opcode_table_idx(const char c_mnemonic[], libdice_word_t *idx)
{

	libdice_word_t i = 0;
	for (i=0; i<LIBDICE_OPCODE_CNT; i++) {
		if (strcmp(s_opcode_define_table[i].m_mnemonic, c_mnemonic) == 0) {
			*idx = i;
			return true;;
		}
	}

	return false;
}

static void libdasm_init_parsed_line(struct libdasm_parsed_line *rdwr_parsed_line)
{
	rdwr_parsed_line->m_opcode = 0;
	memset(rdwr_parsed_line->m_operands, 0, sizeof(struct libdasm_operand) * LIBDICE_OPERAND_MAX_CNT);
	rdwr_parsed_line->m_operand_cnt = 0;
}

static bool libdasm_parse_operand(struct libdasm_operand *rdwr_operand, const struct libdasm_token *rd_token, const struct libdasm_label_table *rd_label_table)
{
	switch (rd_token->m_token_type) 
		{
		case LIBDASM_TOKEN_TYPE_IDENT:
		{
			libdice_word_t label_addr = 0;
			if (libdasm_get_label_address(rd_label_table, rd_token->m_token, &label_addr)) {
				/* label */
				snprintf(rdwr_operand->m_text, LIBDASM_TOKEN_MAX_LEN, "%u", label_addr);
				return true;
			}/* opcode */
			strcpy(rdwr_operand->m_text, rd_token->m_token);
			return true;
		}
		case LIBDASM_TOKEN_TYPE_NUMBER:
		{
			strcpy(rdwr_operand->m_text, rd_token->m_token);
			return true;
		}
		case LIBDASM_TOKEN_TYPE_LABEL:
		{
			/* label must be handled by  libdasm_parse_line. This means the label was placed in the middle of the line, not at the beginning*/
			assert(0);
			return false;
		}
		case LIBDASM_TOKEN_TYPE_STRING:
		{
			assert(0); 	/* Doesn't support string yet*/
			return false;
		}
		case LIBDASM_TOKEN_TYPE_CHAR:
		{
			snprintf(rdwr_operand->m_text, LIBDASM_TOKEN_MAX_LEN, "%u", (unsigned)(unsigned char)rd_token->m_token[1]);
			return true;
		}
		case LIBDASM_TOKEN_TYPE_OPERATOR:
		{
			snprintf(rdwr_operand->m_text, LIBDASM_TOKEN_MAX_LEN, "%u", (unsigned)strlen(rd_token->m_token));
			return true;
		}
		case LIBDASM_TOKEN_TYPE_EOL:
		{
			return false;
		}
		case LIBDASM_TOKEN_TYPE_EOP:
		{
			return false;
		}
		default:
		{
			assert(0);
			return false;
		}
		
		}
}

/**
 * @return Indicates whether the line is meaningful
 * * */
static bool libdasm_parse_line(struct libdasm_parsed_line *rdwr_parsed_line, const struct libdasm_token_line *rd_token_line, const struct libdasm_label_table *rd_label_table)
{
	libdice_word_t token_idx = 0;
	libdice_word_t opcode_table_idx = 0;
	struct libdasm_token *token = NULL;
	libdice_word_t tmp_operand_cnt = 0;

	libdasm_init_parsed_line(rdwr_parsed_line);

	if (rd_token_line->m_token_cnt == 0) {
		return false;	/* Shouldn't reach here*/
	}

	if (rd_token_line->m_token_cnt==1 && (rd_token_line->m_tokens[0].m_token_type==LIBDASM_TOKEN_TYPE_EOL || rd_token_line->m_tokens[0].m_token_type==LIBDASM_TOKEN_TYPE_EOP)) {
		return false;
	}

	if (rd_token_line->m_tokens[0].m_token_type==LIBDASM_TOKEN_TYPE_LABEL) {
		token_idx++;
	}

	token = &(rd_token_line->m_tokens[token_idx]);
	token_idx++;

	if (token->m_token_type==LIBDASM_TOKEN_TYPE_EOL || token->m_token_type==LIBDASM_TOKEN_TYPE_EOP) {
		return false;
	}
	
	/* opcode must be found in opcode_table */
	if (!libdasm_get_opcode_table_idx(token->m_token, &opcode_table_idx)) {
		assert(0);
	}
	rdwr_parsed_line->m_opcode = s_opcode_define_table[opcode_table_idx].m_opcode;
	rdwr_parsed_line->m_operand_cnt = s_opcode_define_table[opcode_table_idx].m_operand_cnt;
	
	tmp_operand_cnt = 0;
	for (; token_idx<rd_token_line->m_token_cnt; token_idx++) {
		token = &(rd_token_line->m_tokens[token_idx]);
		assert(tmp_operand_cnt <= LIBDICE_OPERAND_MAX_CNT);
		if (libdasm_parse_operand(&(rdwr_parsed_line->m_operands[tmp_operand_cnt]), token, rd_label_table)) {
			tmp_operand_cnt++;
		}
	}
	assert(tmp_operand_cnt == rdwr_parsed_line->m_operand_cnt);
	return true;
} 


libdice_word_t libdasm_parse_programme(struct libdasm_parsed_line rdwr_parsed_lines[], const libdice_word_t c_parsed_lines_len, const struct libdasm_token_line rd_token_lines[], const libdice_word_t c_token_line_cnt)
{
	libdice_word_t i = 0;
	libdice_word_t parsed_line_cnt = 0;
	struct libdasm_label_table label_table;
	libdasm_init_label_table(&label_table);
	libdasm_label_programme(&label_table, rd_token_lines, c_token_line_cnt);

	for (i=0; i<c_token_line_cnt; i++) {
		assert(parsed_line_cnt+1 <= c_parsed_lines_len);
		if (libdasm_parse_line(&rdwr_parsed_lines[parsed_line_cnt], &rd_token_lines[i], &label_table)) {
			parsed_line_cnt++;
		}
 	}

	return parsed_line_cnt;
	


}