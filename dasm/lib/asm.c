

#include <dasm.h>
#include <libdice/opcode.h>
#include "pp.h"
#include "tokenizer.h"
#include "parser.h"
#include "assembler.h"



DICEIMPL libdice_word_t dasm_assemble(
		ae2f_LP(c_num_ret) libdice_word_t* ae2f_restrict	rdwr_ret_buf,
		const libdice_word_t					c_ret_buf_len,
		ae2f_LP(str_len) const char* ae2f_restrict		rd_programme,
		const libdice_word_t					c_programme_len
		)
{
	struct dasm_pp_ret ret;

	char tmp_buf[DASM_PROGRAMME_MAX_LEN] = {0,};
	libdice_word_t buf_cnt = 0;
	struct dasm_token_line token_lines[DASM_PROGRAMME_MAX_LEN] = {0,};
	libdice_word_t token_line_cnt = 0;
	struct dasm_parsed_line parsed_lines[DASM_PROGRAMME_MAX_LEN] = {0,};
	libdice_word_t parsed_line_cnt = 0;

	(void)rdwr_ret_buf;
	(void)c_ret_buf_len;
	(void)rd_programme;
	(void)c_programme_len;

	if (c_programme_len > DASM_PROGRAMME_MAX_LEN) {
		return DASM_ERR_RET;
	}

	ret = dasm_preprocess_programme(tmp_buf, DASM_PROGRAMME_MAX_LEN, rd_programme, c_programme_len, &buf_cnt);
	if (ret.err != DASM_PP_ERR_OK) {
		return DASM_ERR_RET;
	}

	token_line_cnt = dasm_tokenize_programme(token_lines, DASM_PROGRAMME_MAX_LEN, tmp_buf, buf_cnt);
	if (token_line_cnt == DASM_ERR_RET) {
		return DASM_ERR_RET;
	}

	parsed_line_cnt = dasm_parse_programme(parsed_lines, DASM_PROGRAMME_MAX_LEN, token_lines, token_line_cnt);
	if (parsed_line_cnt == DASM_ERR_RET) {
		return DASM_ERR_RET;
	}

	return dasm_assemble_programme(rdwr_ret_buf, c_ret_buf_len, parsed_lines, parsed_line_cnt);
}


