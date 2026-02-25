


#ifndef dasm_lexer_h
#define dasm_lexer_h
 
#include <stdlib.h>
#include <ae2f/c90/StdBool.h>
#include <dasm/err.h>
#include <libdice/abi.h>
#include <libdice/type.h>


enum DASM_TOK_TYPE_ {
	DASM_TOK_TYPE_MNEMONIC,
	DASM_TOK_TYPE_IDENT,
	DASM_TOK_TYPE_INT_IMM,
	DASM_TOK_TYPE_CHAR_IMM,
	DASM_TOK_TYPE_STRING_IMM,
	DASM_TOK_TYPE_STAR,
	DASM_TOK_TYPE_COMMA,
	DASM_TOK_TYPE_EOL,
	DASM_TOK_TYPE_EOP
};

enum DASM_LEXER_STATE_ {
	DASM_LEXER_STATE_IDLE,
	DASM_LEXER_STATE_IDENT,
	DASM_LEXER_STATE_INT_IMM,
	DASM_LEXER_STATE_CHAR_IMM,
	DASM_LEXER_STATE_STRING_IMM,
	DASM_LEXER_STATE_STAR
};

struct dasm_tok {
	enum DASM_TOK_TYPE_ m_tok_type;
	const char *m_lexeme;
	libdice_word_t m_lexeme_len;
};	

struct dasm_lexer {
	libdice_word_t m_toks_len;
	libdice_word_t m_tok_cnt;
	libdice_word_t m_src_len;
	libdice_word_t m_src_cnt;

	struct dasm_tok *m_toks;
	const char *m_src;

	enum DASM_LEXER_STATE_ m_state;
};

DICECALL bool dasm_lexer_init(struct dasm_lexer *rdwr_lexer, 
	struct dasm_tok rdwr_toks[], const libdice_word_t c_tokens_len, 
	const char rd_src[], const libdice_word_t c_src_len);
DICECALL bool dasm_lexer_deinit(struct dasm_lexer *rdwr_lexer);
DICECALL enum DASM_ERR_ dasm_lexer_execute(struct dasm_lexer *rdwr_lexer);

#endif /* dasm_lexer_h */