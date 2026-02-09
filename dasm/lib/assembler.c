#include "assembler.h"
#include <string.h>
#include <stdlib.h>

static libdice_word_t libdasm_assemble_line(libdice_word_t rdwr_dst[], libdice_word_t dst_len, const struct libdasm_parsed_line *rd_parsed_line)
{
	libdice_word_t i = 0;
	long tmp = 0;
	char *end;

	assert(dst_len >= rd_parsed_line->m_operand_cnt + 1);
	rdwr_dst[0] = rd_parsed_line->m_opcode;

	for (i=1; i<rd_parsed_line->m_operand_cnt; i++) {
		tmp = strtol(rd_parsed_line->m_operands[i-1].m_text, &end, 10);
		if (tmp<0) {
			/* TODO : compare with libdice_word_t's max value */
			assert(0);
		}
		if (end == rd_parsed_line->m_operands[i-1].m_text) {
			assert(0);	/* not number */
		} else if (*end != '\0') {
			assert(0);	/* trash after number */
		}
		rdwr_dst[i] = (libdice_word_t)tmp;
	}

	/* word cnt */
	return i;
}












libdice_word_t libdasm_assemble_programme(libdice_word_t rdwr_programme[], const libdice_word_t c_programme_len, 
					const struct libdasm_parsed_line rd_parsed_lines[], const libdice_word_t parsed_line_len)
{
	libdice_word_t i = 0;
	libdice_word_t tmp = 0;
	libdice_word_t programme_word_cnt = 0;

	for (i=0; i<parsed_line_len; i++) {
		tmp = libdasm_assemble_line(rdwr_programme+programme_word_cnt, c_programme_len-programme_word_cnt, &rd_parsed_lines[i]);
		assert(programme_word_cnt+tmp <= c_programme_len);
		programme_word_cnt += tmp;
	}

	return programme_word_cnt;
}