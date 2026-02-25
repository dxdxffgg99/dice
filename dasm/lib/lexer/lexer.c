#include <ae2f/cc.h>
#include "lexer.h"
#include <ctype.h>
#include "dasm/err.h"
#include <dasm/keys.h>
#include <libdice/opcode.h>



static const char *s_mnemonics[DASM_MNEMONIC_CNT] = {
	DASM_MNEMONIC_NOP,
	DASM_MNEMONIC_IADD,
	DASM_MNEMONIC_ISUB,
	DASM_MNEMONIC_IMUL,
	DASM_MNEMONIC_IDIV,
	DASM_MNEMONIC_IREM,
	DASM_MNEMONIC_UMUL,
	DASM_MNEMONIC_UDIV,
	DASM_MNEMONIC_UREM,
	DASM_MNEMONIC_FADD,
	DASM_MNEMONIC_FSUB,
	DASM_MNEMONIC_FMUL,
	DASM_MNEMONIC_FDIV,
	DASM_MNEMONIC_INEG,
	DASM_MNEMONIC_FNEG,
	DASM_MNEMONIC_JMP,
	DASM_MNEMONIC_JMPA,
	DASM_MNEMONIC_JMPN,
	DASM_MNEMONIC_JMPZ,
	DASM_MNEMONIC_JMPZA,
	DASM_MNEMONIC_JMPZN,
	DASM_MNEMONIC_BAND,
	DASM_MNEMONIC_BOR,
	DASM_MNEMONIC_BXOR,
	DASM_MNEMONIC_BLSHIFT,
	DASM_MNEMONIC_BRSHIFT,
	DASM_MNEMONIC_LRSHIFT,
	DASM_MNEMONIC_BNOT,
	DASM_MNEMONIC_LAND,
	DASM_MNEMONIC_LOR,
	DASM_MNEMONIC_LNOT,
	DASM_MNEMONIC_TOBIT,
	DASM_MNEMONIC_EQ,
	DASM_MNEMONIC_NEQ,
	DASM_MNEMONIC_SET,
	DASM_MNEMONIC_MSET,
	DASM_MNEMONIC_MOV,
	DASM_MNEMONIC_ITOF,
	DASM_MNEMONIC_FTOI,
	DASM_MNEMONIC_IGT,
	DASM_MNEMONIC_FGT,
	DASM_MNEMONIC_ILT,
	DASM_MNEMONIC_FLT,
	DASM_MNEMONIC_PUTS,
	DASM_MNEMONIC_PUTI,
	DASM_MNEMONIC_PUTU,
	DASM_MNEMONIC_PUTC,
	DASM_MNEMONIC_PUTF,
	DASM_MNEMONIC_DEF,
	DASM_MNEMONIC_UNDEF,
	DASM_MNEMONIC_SETRANDSEED,
	DASM_MNEMONIC_IRAND,
	DASM_MNEMONIC_FRAND,
	DASM_MNEMONIC_EOP
};


DICEIMPL bool dasm_lexer_init(struct dasm_lexer *rdwr_lexer, 
	struct dasm_tok rdwr_toks[], const libdice_word_t c_toks_len, 
	const char rd_src[], const libdice_word_t c_src_len)
{
	if (!rdwr_lexer || !rdwr_toks || !rd_src) {
		return false;
	}

	rdwr_lexer->m_toks = rdwr_toks;
	rdwr_lexer->m_toks_len = c_toks_len;
	rdwr_lexer->m_tok_cnt = 0;

	rdwr_lexer->m_src = rd_src;
	rdwr_lexer->m_src_len = c_src_len;
	rdwr_lexer->m_src_cnt = 0;

	rdwr_lexer->m_state = DASM_LEXER_STATE_IDLE;

	return true;
}

DICEIMPL bool dasm_lexer_deinit(struct dasm_lexer *rdwr_lexer)
{
	if (!rdwr_lexer) {
		return false;
	}

	rdwr_lexer->m_toks = NULL;
	rdwr_lexer->m_toks_len = 0;
	rdwr_lexer->m_tok_cnt = 0;

	rdwr_lexer->m_src = NULL;
	rdwr_lexer->m_src_len = 0;
	rdwr_lexer->m_src_cnt = 0;

	return true;
}

static ae2f_inline bool dasm_lexer_init_tok(struct dasm_tok *rdwr_tok)
{
	if (!rdwr_tok) {
		return false;
	}

	rdwr_tok->m_lexeme = NULL;
	rdwr_tok->m_lexeme_len = 0;
	rdwr_tok->m_tok_type = DASM_TOK_TYPE_EOP;

	return true;
}

static ae2f_inline bool dasm_lexer_create_tok(struct dasm_lexer *rdwr_lexer)
{
	if (!rdwr_lexer) {
		return false;
	}
	
	if ((rdwr_lexer->m_tok_cnt + 1) >= rdwr_lexer->m_toks_len) {
		return false;
	}

	dasm_lexer_init_tok(&(rdwr_lexer->m_toks[rdwr_lexer->m_tok_cnt]));

	rdwr_lexer->m_tok_cnt++;
	
	return true;
}

static ae2f_inline bool dasm_lexer_set_lexeme(struct dasm_lexer *rdwr_lexer, const char c_rd_lexeme[])
{
	if (!rdwr_lexer) {
		return false;
	}

	rdwr_lexer->m_toks[rdwr_lexer->m_tok_cnt-1].m_lexeme = c_rd_lexeme;

	return true;
}

static ae2f_inline bool dasm_lexer_increase_lexeme_len(struct dasm_lexer *rdwr_lexer, const libdice_word_t c_cnt)
{
	if (!rdwr_lexer) {
		return false;
	}

	rdwr_lexer->m_toks[rdwr_lexer->m_tok_cnt-1].m_lexeme_len += c_cnt;
		
	return true;
}

static ae2f_inline bool dasm_lexer_set_tok_type(struct dasm_lexer *rdwr_lexer, const enum DASM_TOK_TYPE_ c_tok_type)
{
	if (!rdwr_lexer) {
		return false;
	}

	rdwr_lexer->m_toks[rdwr_lexer->m_tok_cnt-1].m_tok_type = c_tok_type;

	return true;

}

static ae2f_inline enum DASM_ERR_ dasm_lexer_execute_line(struct dasm_lexer *rdwr_lexer)
{
	char ch; 
	const char *lexeme = NULL; 

	if (!rdwr_lexer || !rdwr_lexer->m_toks || !rdwr_lexer->m_src) {
		return DASM_ERR_UNKNOWN;
	}

	while (rdwr_lexer->m_src_cnt < rdwr_lexer->m_src_len 
		&& rdwr_lexer->m_tok_cnt < rdwr_lexer->m_toks_len) {
		
		lexeme = &(rdwr_lexer->m_src[rdwr_lexer->m_src_cnt]);
		ch = lexeme[0];

		switch (rdwr_lexer->m_state) {
		case DASM_LEXER_STATE_IDLE:

			if (ch == ' ') {
				rdwr_lexer->m_src_cnt++;
			} else if (ch == '\'') {

				rdwr_lexer->m_state = DASM_LEXER_STATE_CHAR_IMM;
				dasm_lexer_create_tok(rdwr_lexer);
				dasm_lexer_set_lexeme(rdwr_lexer, lexeme);
				dasm_lexer_increase_lexeme_len(rdwr_lexer, 1);
				dasm_lexer_set_tok_type(rdwr_lexer, DASM_TOK_TYPE_CHAR_IMM);
				rdwr_lexer->m_src_cnt++;

			} else if (ch == '\"') {

				rdwr_lexer->m_state = DASM_LEXER_STATE_STRING_IMM;
				dasm_lexer_create_tok(rdwr_lexer);
				dasm_lexer_set_lexeme(rdwr_lexer, lexeme);
				dasm_lexer_increase_lexeme_len(rdwr_lexer, 1);
				dasm_lexer_set_tok_type(rdwr_lexer, DASM_TOK_TYPE_STRING_IMM);
				rdwr_lexer->m_src_cnt++;

			} else if (ch == '*') {

				rdwr_lexer->m_state = DASM_LEXER_STATE_STAR;
				dasm_lexer_create_tok(rdwr_lexer);
				dasm_lexer_set_lexeme(rdwr_lexer, lexeme);
				dasm_lexer_increase_lexeme_len(rdwr_lexer, 1);
				dasm_lexer_set_tok_type(rdwr_lexer, DASM_TOK_TYPE_STAR);
				rdwr_lexer->m_src_cnt++;

			} else if (ch == '\n') {

				dasm_lexer_create_tok(rdwr_lexer);
				dasm_lexer_set_lexeme(rdwr_lexer, lexeme);
				dasm_lexer_increase_lexeme_len(rdwr_lexer, 1);
				dasm_lexer_set_tok_type(rdwr_lexer, DASM_TOK_TYPE_EOL);
				rdwr_lexer->m_src_cnt++;

				return DASM_ERR_OK;

			} else if (ch == '\0') {

				dasm_lexer_create_tok(rdwr_lexer);
				dasm_lexer_set_lexeme(rdwr_lexer, lexeme);
				dasm_lexer_increase_lexeme_len(rdwr_lexer, 1);
				dasm_lexer_set_tok_type(rdwr_lexer, DASM_TOK_TYPE_EOP);
				rdwr_lexer->m_src_cnt++;

				return DASM_ERR_OK;

			} else if (isalpha(ch) || ch == '_'){

				/** tok_type = (IDENT or MNEMONIC) */
				rdwr_lexer->m_state = DASM_LEXER_STATE_IDENT;
				dasm_lexer_create_tok(rdwr_lexer);
				dasm_lexer_set_lexeme(rdwr_lexer, lexeme);
				dasm_lexer_increase_lexeme_len(rdwr_lexer, 1);
				dasm_lexer_set_tok_type(rdwr_lexer, DASM_TOK_TYPE_IDENT);
				rdwr_lexer->m_src_cnt++;

			} else if (isdigit(ch)) {

				rdwr_lexer->m_state = DASM_LEXER_STATE_INT_IMM;
				dasm_lexer_create_tok(rdwr_lexer);
				dasm_lexer_set_lexeme(rdwr_lexer, lexeme);
				dasm_lexer_increase_lexeme_len(rdwr_lexer, 1);
				dasm_lexer_set_tok_type(rdwr_lexer, DASM_TOK_TYPE_INT_IMM);
				rdwr_lexer->m_src_cnt++;

			} else if (ch == ',') {
				
				dasm_lexer_create_tok(rdwr_lexer);
				dasm_lexer_set_lexeme(rdwr_lexer, lexeme);
				dasm_lexer_increase_lexeme_len(rdwr_lexer, 1);
				dasm_lexer_set_tok_type(rdwr_lexer, DASM_TOK_TYPE_COMMA);
				rdwr_lexer->m_src_cnt++;

			} else {
				return DASM_ERR_UNKNOWN;
			}
			break;

		case DASM_LEXER_STATE_IDENT:

			if (isalnum(ch) || ch == '_') {
				dasm_lexer_increase_lexeme_len(rdwr_lexer, 1);
				rdwr_lexer->m_src_cnt++;
			} else {
				/** TODO : Is mnemonic */
				rdwr_lexer->m_state = DASM_LEXER_STATE_IDLE;
			}
			break;
			
		case DASM_LEXER_STATE_INT_IMM:

			if (isdigit(ch)) {
				dasm_lexer_increase_lexeme_len(rdwr_lexer, 1);
				rdwr_lexer->m_src_cnt++;
			} else {
				rdwr_lexer->m_state = DASM_LEXER_STATE_IDLE;
			}
			break;

		case DASM_LEXER_STATE_CHAR_IMM:

			switch (ch) {
			case '\'':

				dasm_lexer_increase_lexeme_len(rdwr_lexer, 1);
				rdwr_lexer->m_src_cnt++;
				rdwr_lexer->m_state = DASM_LEXER_STATE_IDLE;
				break;

			case '\n':
			case '\0':
				rdwr_lexer->m_src_cnt++;
				return DASM_ERR_INVAL_CHAR_IMM;
			default:
				dasm_lexer_increase_lexeme_len(rdwr_lexer, 1);
				rdwr_lexer->m_src_cnt++;
				break;
			}
			break;

		case DASM_LEXER_STATE_STRING_IMM:

			switch (ch) {
			case '\"':

				dasm_lexer_increase_lexeme_len(rdwr_lexer, 1);
				rdwr_lexer->m_src_cnt++;
				rdwr_lexer->m_state = DASM_LEXER_STATE_IDLE;
				break;

			case '\n':
			case '\0':
				rdwr_lexer->m_src_cnt++;
				return DASM_ERR_INVAL_STRING_IMM;
			default:

				dasm_lexer_increase_lexeme_len(rdwr_lexer, 1);
				rdwr_lexer->m_src_cnt++;
				break;

			}
			break;

		case DASM_LEXER_STATE_STAR:

			if (ch == '*') {
				dasm_lexer_increase_lexeme_len(rdwr_lexer, 1);
				rdwr_lexer->m_src_cnt++;
			} else {
				rdwr_lexer->m_state = DASM_LEXER_STATE_IDLE;
				rdwr_lexer->m_src_cnt++;
			}		
			break;

		default:
			rdwr_lexer->m_src_cnt++;
			return DASM_ERR_UNKNOWN;
		}			
	}

	if (rdwr_lexer->m_tok_cnt == rdwr_lexer->m_toks_len) {
		return DASM_ERR_MEM_INSUF;
	}

	if (rdwr_lexer->m_src_cnt == rdwr_lexer->m_src_len) {
		return DASM_ERR_NO_TERM;
	}

	return DASM_ERR_UNKNOWN;
}

DICEIMPL enum DASM_ERR_ dasm_lexer_execute(struct dasm_lexer *rdwr_lexer)
{
	enum DASM_ERR_ err = DASM_ERR_OK;

	if (!rdwr_lexer) {
		return DASM_ERR_UNKNOWN;
	}

	while (rdwr_lexer->m_tok_cnt < rdwr_lexer->m_toks_len
		&& rdwr_lexer->m_src_cnt < rdwr_lexer->m_src_len) {
		err = dasm_lexer_execute_line(rdwr_lexer);
		if (err != DASM_ERR_OK) {
			break;
		}
	}

	return err;
}