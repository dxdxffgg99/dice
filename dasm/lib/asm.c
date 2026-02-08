

#include <dasm.h>
#include <libdice/opcode.h>
#include <../preprocessor/pp.h>	/* TODO : fix relative path*/
#include <dasm/keys.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>





DICEIMPL libdice_word_t libdasm_assemble_line(
		ae2f_LP(c_num_ret) libdice_word_t* ae2f_restrict	rdwr_ret_buf,
		const libdice_word_t					c_ret_buf_len,
		ae2f_LP(str_len) const char* ae2f_restrict		rd_instruction,
		const libdice_word_t					c_instruction_len
		)
{
	(void)rdwr_ret_buf;
	(void)c_ret_buf_len;
	(void)rd_instruction;
	(void)c_instruction_len;

	char tmp_buf[LIBDASM_PROGRAMME_MAX_LEN] = {0,};
	

	return 0;
}


