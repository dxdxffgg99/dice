#ifndef libdasm_tokenizer_h
#define libdasm_tokenizer_h

#include <libdice/type.h>

#define LIBDASM_TOKEN_MAX_LEN 64
#define LIBDASM_TOKEN_MAX_CNT_PER_LINE 12

enum e_libdasm_token_type {
	LIBDASM_TOKEN_TYPE_IDENT = 0,	/* opcode */
	LIBDASM_TOKEN_TYPE_NUMBER,	/* immediate value, address*/
	LIBDASM_TOKEN_TYPE_LABEL,	/* :label */		
	LIBDASM_TOKEN_TYPE_STRING,	/* "string" */
	LIBDASM_TOKEN_TYPE_CHAR,	/* 'char' */
	LIBDASM_TOKEN_TYPE_OPERATOR,	/* asterisk*/
	LIBDASM_TOKEN_TYPE_EOL,		/* '\n' */
	LIBDASM_TOKEN_TYPE_EOP		/* '\0' */
};
 


struct libdasm_token {
	char m_token[LIBDASM_TOKEN_MAX_LEN];
	enum e_libdasm_token_type m_token_type;
};

struct libdasm_token_line {
	struct libdasm_token m_token_line[LIBDASM_TOKEN_MAX_CNT_PER_LINE];
	libdice_word_t m_token_cnt;
};

static libdice_word_t libdasm_tokenize_programme(struct libdasm_token_line rdwr_token_table[], const libdice_word_t c_token_table_len, const char *rd_src);


#endif /* libdasm_tokenizer_h */