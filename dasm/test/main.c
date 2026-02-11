
#include <dasm.h>
#include <stdio.h>

#define  PROGRAMME 	"label0: nop		\n"	\
			"iadd 111 *111 #1	\n"	\
			"igt 112 *111 #8	\n"	\
			"puti *111 		\n"	\
			"jmpz *112 *label0	\n"	\
			"eop"


int main(void)
{	
	libdice_word_t exe[LIBDASM_PROGRAMME_MAX_LEN] = {0,};
	libdice_word_t exe_cnt = 0;
	libdice_word_t i = 0;

	char asm_programme[LIBDASM_PROGRAMME_MAX_LEN] = PROGRAMME;
	exe_cnt = libdasm_assemble(exe, LIBDASM_PROGRAMME_MAX_LEN, asm_programme, LIBDASM_PROGRAMME_MAX_LEN);
	if (exe_cnt == LIBDASM_ERR_RET) {
		printf("Syntax error\n");
		return 0;
	}

	for (i=0; i<exe_cnt; ++i) {
		printf("%u ", exe[i]);
	}
	printf("\n");
	return 0; 
} 	
