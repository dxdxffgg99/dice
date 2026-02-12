#ifndef dasm_tokenizer_h
#define dasm_tokenizer_h

#include <libdice/type.h>
#include <dasm.h>

#define DASM_TOKEN_MAX_LEN 64	
#define DASM_TOKEN_MAX_CNT_PER_LINE 12

enum DASM_TOKEN_TYPE_ {
	DASM_TOKEN_TYPE_IDENT = 0,	/* opcode, jmp *label */	
	DASM_TOKEN_TYPE_NUMBER,		/* immediate value, address*/
	DASM_TOKEN_TYPE_LABEL,		/* label: */
	DASM_TOKEN_TYPE_STRING,		/* "string" */
	DASM_TOKEN_TYPE_CHAR,		/* 'char' */
	DASM_TOKEN_TYPE_OPERATOR,	/* asterisk*/
	DASM_TOKEN_TYPE_EOL,		/* '\n' */
	DASM_TOKEN_TYPE_EOP		/* '\0' */
};
 

enum DASM_TK_ERR_ {
	DASM_TK_ERR_OK,
	DASM_TK_ERR_MEM_INSUF,
	DASM_TK_ERR_NO_TERM,
	DASM_TK_ERR_
};
struct dasm_token {
	char m_text[DASM_TOKEN_MAX_LEN];
	enum DASM_TOKEN_TYPE_ m_token_type;
};

struct dasm_token_line {
	struct dasm_token m_tokens[DASM_TOKEN_MAX_CNT_PER_LINE];
	libdice_word_t m_token_cnt;
};

DICECALL libdice_word_t dasm_tokenize_programme(struct dasm_token_line rdwr_token_lines[], const libdice_word_t c_token_lines_len, const char rd_src[], const libdice_word_t c_src_len);
libdice_word_t dasm_get_token_line_word_len(const struct dasm_token_line *rd_token_line);


#endif /* dasm_tokenizer_h */
