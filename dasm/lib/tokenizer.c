#include "tokenizer.h"
#include <string.h>
#include <stdlib.h>
#include <ae2f/c90/StdBool.h>


enum e_tokenizer_state {
	TOKENIZER_STATE_IDLE = 0,
	TOKENIZER_STATE_IDENT,
	TOKENIZER_STATE_NUMBER,
	TOKENIZER_STATE_STRING,
	TOKENIZER_STATE_CHAR,
	TOKENIZER_STATE_OPERATOR
};

static bool is_alphabet(const char c_c)
{
	return c_c>='a' && c_c<='z' || c_c>='A' && c_c<='Z';
}

static bool is_number(const char c_c)
{
	return c_c>='0' && c_c<='9';
}

static void libdasm_init_token_line(struct libdasm_token_line *rdwr_token_line)
{
	rdwr_token_line->m_token_cnt = 0;
	memset(rdwr_token_line->m_tokens, 0, sizeof(struct libdasm_token) * LIBDASM_TOKEN_MAX_CNT_PER_LINE);	/* not essential */
}

static bool libdasm_create_new_token(struct libdasm_token_line *rdwr_token_line)
{
	if (rdwr_token_line->m_token_cnt == LIBDASM_TOKEN_MAX_CNT_PER_LINE) {
		return false;
	}

	memset(rdwr_token_line->m_tokens[rdwr_token_line->m_token_cnt].m_token, 0, LIBDASM_TOKEN_MAX_LEN);
	rdwr_token_line->m_token_cnt++;

	return true;
}

/* Append c_c last token's back. Doesn't create new token */
static inline void libdasm_insert_token_char(struct libdasm_token_line *rdwr_token_line, const char c_c)
{
	libdice_word_t i = 0;

	assert(rdwr_token_line->m_token_cnt > 0);

	for (i=0; i<LIBDASM_TOKEN_MAX_LEN; i++) {
		if (rdwr_token_line->m_tokens[rdwr_token_line->m_token_cnt-1].m_token[i] == '\0') {
			assert(i+1 < LIBDASM_TOKEN_MAX_LEN);
			rdwr_token_line->m_tokens[rdwr_token_line->m_token_cnt-1].m_token[i] = c_c;
			rdwr_token_line->m_tokens[rdwr_token_line->m_token_cnt-1].m_token[i+1] = '\0';
			break;
		}
	}
}

/* Set last token's token type. Doesn't create new token */
static inline void libdasm_set_token_type(struct libdasm_token_line *rdwr_token_line, const enum LIBDASM_TOKEN_TYPE_ c_token_type)
{
	assert(rdwr_token_line->m_token_cnt > 0);

	rdwr_token_line->m_tokens[rdwr_token_line->m_token_cnt-1].m_token_type = c_token_type;
}

static libdice_word_t libdasm_tokenize_line(struct libdasm_token_line *rdwr_token_line, const char *rd_src)
{
	libdice_word_t src_len = 0;
	libdice_word_t pc = 0;
	char c = 0;
	enum e_tokenizer_state state = TOKENIZER_STATE_IDLE;
	libdice_word_t char_cnt = 0;

	libdasm_init_token_line(rdwr_token_line);

	src_len = strlen(rd_src) + 1;
	
	for (pc=0; pc<src_len;) {
		c = rd_src[pc];
		switch (state)
		{
		case TOKENIZER_STATE_IDLE:
		{
			if (c==' ') {
				pc++;
			} else if (c=='\n') {
				libdasm_create_new_token(rdwr_token_line);
				libdasm_insert_token_char(rdwr_token_line, c);
				libdasm_set_token_type(rdwr_token_line, LIBDASM_TOKEN_TYPE_EOL);
				pc++;
				return pc;
			} else if (c=='*') {
				libdasm_create_new_token(rdwr_token_line);
				libdasm_insert_token_char(rdwr_token_line, c);
				libdasm_set_token_type(rdwr_token_line, LIBDASM_TOKEN_TYPE_OPERATOR);
				state = TOKENIZER_STATE_OPERATOR;
				pc++;
			} else if (is_alphabet(c) || c=='_') {
				libdasm_create_new_token(rdwr_token_line);
				libdasm_insert_token_char(rdwr_token_line, c);
				libdasm_set_token_type(rdwr_token_line, LIBDASM_TOKEN_TYPE_IDENT);
				state = TOKENIZER_STATE_IDENT;
				pc++;
			} else if (c=='\"') {
				libdasm_create_new_token(rdwr_token_line);
				libdasm_insert_token_char(rdwr_token_line, c);
				libdasm_set_token_type(rdwr_token_line, LIBDASM_TOKEN_TYPE_STRING);
				state = TOKENIZER_STATE_STRING;
				pc++;
			} else if (c=='\'') {
				libdasm_create_new_token(rdwr_token_line);
				libdasm_insert_token_char(rdwr_token_line, c);
				libdasm_set_token_type(rdwr_token_line, LIBDASM_TOKEN_TYPE_CHAR);
				state = TOKENIZER_STATE_CHAR;
				char_cnt = 0;
				pc++;
			} else if (c=='\0') {
				libdasm_create_new_token(rdwr_token_line);
				libdasm_insert_token_char(rdwr_token_line, c);
				libdasm_set_token_type(rdwr_token_line, LIBDASM_TOKEN_TYPE_EOP);
				pc++;
				return pc;
			} else if (is_number(c)) {
				libdasm_create_new_token(rdwr_token_line);
				libdasm_insert_token_char(rdwr_token_line, c);
				libdasm_set_token_type(rdwr_token_line, LIBDASM_TOKEN_TYPE_NUMBER);
				state = TOKENIZER_STATE_NUMBER;
				pc++;
			} else {
				assert(0);
			}
			break;
		}
		case TOKENIZER_STATE_IDENT:
		{
			if (is_alphabet(c) || is_number(c) || c=='_') {
				libdasm_insert_token_char(rdwr_token_line, c);
				pc++;
			} else if (c==':') {
				libdasm_set_token_type(rdwr_token_line, LIBDASM_TOKEN_TYPE_LABEL);
				state = TOKENIZER_STATE_IDLE;
				pc++;
			} else {
				state = TOKENIZER_STATE_IDLE;
			}
			break;
		}
		case TOKENIZER_STATE_NUMBER:
		{
			if (is_number(c)) {
				libdasm_insert_token_char(rdwr_token_line, c);
				pc++;
			} else {
				state = TOKENIZER_STATE_IDLE;
			}
			break;
		}
		case TOKENIZER_STATE_STRING:
		{
			assert(c!='\n' && c!='\0');
			/* TODO : escape sequence handling  */
			if (c=='\"') {
				libdasm_insert_token_char(rdwr_token_line, c);
				pc++;
				state = TOKENIZER_STATE_IDLE;
			} else {
				libdasm_insert_token_char(rdwr_token_line, c);
				pc++;
			}
			break;
		}
		case TOKENIZER_STATE_CHAR:
		{
			if (char_cnt==1 && c=='\'') {
				libdasm_insert_token_char(rdwr_token_line, c);
				pc++;
				state = TOKENIZER_STATE_IDLE;
			} else if (char_cnt == 0 && (unsigned char)c < 0x80) {
				libdasm_insert_token_char(rdwr_token_line, c);
				pc++;
				char_cnt++;
			} else {
				assert(0);	/* not 1 character or Non ascii */
			}
			break;
		}
		case TOKENIZER_STATE_OPERATOR:
		{
			if (c=='*') {
				libdasm_insert_token_char(rdwr_token_line, c);
				pc++;
			} else {
				state = TOKENIZER_STATE_IDLE;
			}
			break;
		}
		default:
		{
			assert(0);
		}
		}
		
		
	}
	return pc;
}


/**
 * @brief Reports the size occupied by the program in words.
 * * */
libdice_word_t libdasm_get_token_line_len(const struct libdasm_token_line *rd_token_line)
{
	libdice_word_t i = 0;
	libdice_word_t len = 0;
	enum LIBDASM_TOKEN_TYPE_ token_type = 0;

	for (i=0; i<rd_token_line->m_token_cnt; i++) {
		token_type = rd_token_line->m_tokens[i].m_token_type;
		if (token_type == LIBDASM_TOKEN_TYPE_LABEL || token_type == LIBDASM_TOKEN_TYPE_EOL || token_type == LIBDASM_TOKEN_TYPE_EOP) {
			continue;
		}
		len++;
	}
	return len;
}

static libdice_word_t libdasm_tokenize_programme(struct libdasm_token_line rdwr_token_lines[], const libdice_word_t c_token_table_len, const char *rd_src)
{
	libdice_word_t pc = 0;
	libdice_word_t token_line_cnt = 0;
	libdice_word_t src_len = strlen(rd_src) + 1; 

	while (pc < src_len) {
		assert(token_line_cnt + 1 <= c_token_table_len);
		pc += libdasm_tokenize_line(&rdwr_token_lines[token_line_cnt], rd_src+pc);
		token_line_cnt++;
	} 

	return token_line_cnt;
}