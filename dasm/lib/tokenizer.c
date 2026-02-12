#include "tokenizer.h"
#include <string.h>
#include <stdlib.h>
#include <ae2f/c90/StdBool.h>
#include <assert.h>

#include <ctype.h>
enum e_tokenizer_state {
	TOKENIZER_STATE_IDLE = 0,
	TOKENIZER_STATE_IDENT,
	TOKENIZER_STATE_NUMBER,
	TOKENIZER_STATE_STRING,
	TOKENIZER_STATE_CHAR,
	TOKENIZER_STATE_OPERATOR
};

#define	is_alphabet	isalpha
#define	is_number	isdigit

static void dasm_init_token_line(struct dasm_token_line rdwr_tokens[])
{
	rdwr_tokens->m_token_cnt = 0;
	memset(rdwr_tokens->m_tokens
			, 0, sizeof(struct dasm_token) * DASM_TOKEN_MAX_CNT_PER_LINE
			);
}

static bool dasm_create_new_token(struct dasm_token_line rdwr_tokens[])
{
	if (rdwr_tokens->m_token_cnt == DASM_TOKEN_MAX_CNT_PER_LINE) {
		return false;
	}

	memset(rdwr_tokens->m_tokens[rdwr_tokens->m_token_cnt].m_text, 0, DASM_TOKEN_MAX_LEN);
	rdwr_tokens->m_token_cnt++;

	return true;
}

/* Append c_c behind last token's last character. Doesn't create new token */
static inline bool dasm_insert_token_char(struct dasm_token_line rdwr_tokens[], const char c_c)
{
	libdice_word_t i = 0;
	char *token = rdwr_tokens->m_tokens[rdwr_tokens->m_token_cnt-1].m_text;	/* last token */

	if (!rdwr_tokens->m_token_cnt) {
		return false;
	}

	for (i=0; i<DASM_TOKEN_MAX_LEN; ++i) {
		if (token[i] == '\0') {
			/* Boundary check */
			if (i+1 >= DASM_TOKEN_MAX_LEN) {
				return false;
			}
			token[i] = c_c;
			token[i+1] = '\0';

			return true;
		}
	}

	return false;
}

/* Set last token's token type. Doesn't create new token */
static inline bool dasm_set_token_type(struct dasm_token_line rdwr_tokens[], const enum DASM_TOKEN_TYPE_ c_token_type)
{
	if (!rdwr_tokens->m_token_cnt) {
		return false;
	}

	rdwr_tokens->m_tokens[rdwr_tokens->m_token_cnt-1].m_token_type = c_token_type;
	return true;
}

static libdice_word_t dasm_tokenize_line(struct dasm_token_line rdwr_tokens[], const char rd_src[], const libdice_word_t c_src_len)
{
	libdice_word_t read_cnt = 0;
	enum e_tokenizer_state state = TOKENIZER_STATE_IDLE;
	libdice_word_t char_cnt = 0;

	dasm_init_token_line(rdwr_tokens);
	
	for (read_cnt=0; read_cnt<c_src_len;) {
		const char c = rd_src[read_cnt];
		switch (state)
		{
			case TOKENIZER_STATE_IDLE:
				if (c==' ') {
					read_cnt++;
				} else if (c=='\n') {
					dasm_create_new_token(rdwr_tokens);
					dasm_insert_token_char(rdwr_tokens, c);
					dasm_set_token_type(rdwr_tokens, DASM_TOKEN_TYPE_EOL);
					read_cnt++;
					return read_cnt;
				} else if (c=='*' || c=='#') {	
					dasm_create_new_token(rdwr_tokens);
					dasm_insert_token_char(rdwr_tokens, c);
					dasm_set_token_type(rdwr_tokens, DASM_TOKEN_TYPE_OPERATOR);
					state = TOKENIZER_STATE_OPERATOR;
					read_cnt++;
				} else if (is_alphabet(c) || c=='_') {
					dasm_create_new_token(rdwr_tokens);
					dasm_insert_token_char(rdwr_tokens, c);
					dasm_set_token_type(rdwr_tokens, DASM_TOKEN_TYPE_IDENT);
					state = TOKENIZER_STATE_IDENT;
					read_cnt++;
				} else if (c=='\"') {
					dasm_create_new_token(rdwr_tokens);
					dasm_insert_token_char(rdwr_tokens, c);
					dasm_set_token_type(rdwr_tokens, DASM_TOKEN_TYPE_STRING);
					state = TOKENIZER_STATE_STRING;
					read_cnt++;
				} else if (c=='\'') {
					/* TODO : Don't copy single quote */
					dasm_create_new_token(rdwr_tokens);
					dasm_insert_token_char(rdwr_tokens, c);
					dasm_set_token_type(rdwr_tokens, DASM_TOKEN_TYPE_CHAR);
					state = TOKENIZER_STATE_CHAR;
					char_cnt = 0;
					read_cnt++;
				} else if (c=='\0') {
					dasm_create_new_token(rdwr_tokens);
					dasm_insert_token_char(rdwr_tokens, c);
					dasm_set_token_type(rdwr_tokens, DASM_TOKEN_TYPE_EOP);
					read_cnt++;
					return read_cnt;
				} else if (is_number(c) || c=='-') {
					dasm_create_new_token(rdwr_tokens);
					dasm_insert_token_char(rdwr_tokens, c);
					dasm_set_token_type(rdwr_tokens, DASM_TOKEN_TYPE_NUMBER);
					state = TOKENIZER_STATE_NUMBER;
					read_cnt++;
				} else {
					assert(0);
				}
				break;
			case TOKENIZER_STATE_IDENT:
				if (is_alphabet(c) || is_number(c) || c=='_') {
					dasm_insert_token_char(rdwr_tokens, c);
					read_cnt++;
				} else if (c==':') {
					dasm_set_token_type(rdwr_tokens, DASM_TOKEN_TYPE_LABEL);
					state = TOKENIZER_STATE_IDLE;
					read_cnt++;
				} else {
					state = TOKENIZER_STATE_IDLE;
				}
				break;
			case TOKENIZER_STATE_NUMBER:
				if (is_number(c)) {
					dasm_insert_token_char(rdwr_tokens, c);
					read_cnt++;
				} else {
					state = TOKENIZER_STATE_IDLE;
				}
				break;
			case TOKENIZER_STATE_STRING:
				assert(c!='\n' && c!='\0');
				/* TODO : escape sequence handling  */
				if (c=='\"') {
					dasm_insert_token_char(rdwr_tokens, c);
					read_cnt++;
					state = TOKENIZER_STATE_IDLE;
				} else {
					dasm_insert_token_char(rdwr_tokens, c);
					read_cnt++;
				}
				break;
			case TOKENIZER_STATE_CHAR:
				/* TODO : Don't copy single quote */
				if (char_cnt==1 && c=='\'') {
					dasm_insert_token_char(rdwr_tokens, c);
					read_cnt++;
					state = TOKENIZER_STATE_IDLE;
				} else if (char_cnt == 0 && isascii(c)) {
					dasm_insert_token_char(rdwr_tokens, c);
					read_cnt++;
					char_cnt++;
				} else {
					return DASM_ERR_RET;		/* not 1 character or Non ascii */
				}
				break;
			case TOKENIZER_STATE_OPERATOR:
				if (c=='*') {
					dasm_insert_token_char(rdwr_tokens, c);
					read_cnt++;
				} else {
					state = TOKENIZER_STATE_IDLE;
				}
				break;
			default:
				assert(0);
				break;
		}
		
		
	}
	return read_cnt;
}


/**
 * @brief Reports the size occupied by the program in words.
 * * */
DICEIMPL libdice_word_t dasm_get_token_line_word_len(const struct dasm_token_line *rd_token_line)
{
	libdice_word_t i = 0;
	libdice_word_t word_len = 0;
	
	for (i=0; i<rd_token_line->m_token_cnt; i++) {
		const enum DASM_TOKEN_TYPE_ token_type = rd_token_line->m_tokens[i].m_token_type;
		if (token_type == DASM_TOKEN_TYPE_LABEL || token_type == DASM_TOKEN_TYPE_EOL || token_type == DASM_TOKEN_TYPE_EOP) {
			continue;
		}
		word_len++;
	}
	return word_len;
}

DICEIMPL libdice_word_t dasm_tokenize_programme(struct dasm_token_line rdwr_token_lines[], const libdice_word_t c_token_lines_len, const char rd_src[], const libdice_word_t c_src_len)
{
	libdice_word_t read_cnt = 0;
	libdice_word_t token_line_cnt = 0;
	libdice_word_t read_src_len;
	read_src_len = (libdice_word_t)strlen(rd_src) + 1;

	while (read_cnt < c_src_len) {
		libdice_word_t tmp_read_cnt = 0;
		if (token_line_cnt + 1 > c_token_lines_len) {
			return DASM_ERR_RET;
		}
		tmp_read_cnt = dasm_tokenize_line(&rdwr_token_lines[token_line_cnt], rd_src+read_cnt, c_src_len-read_cnt);
		if (tmp_read_cnt == DASM_ERR_RET) {
			return DASM_ERR_RET;
		}
		read_cnt += tmp_read_cnt;
		token_line_cnt++;

		if (read_cnt >= read_src_len) {
			break;
		}
	} 

	return token_line_cnt;
}
