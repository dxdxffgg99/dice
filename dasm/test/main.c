#include <string.h>
#include <stdio.h>
#include "../lib/pp/pp.h"
#include "../lib/lexer/lexer.h"
#include "libdice/type.h"

#define PROGRAMME	"iadd 1000 10, 10 	   \r\n"	\
			" }    \"	dsdsf 	    \n"	\
			"      		   		\n"	\
			"\0"	


int main(void)
{
	libdice_word_t i,j;

	struct dasm_pp pp;
	struct dasm_lexer lexer;

	const char src[100] = PROGRAMME; 
	char dst[100] = {0,};
	struct dasm_tok toks[100] = {0,};
	
	dasm_pp_init(&pp, dst, 100, src, strlen(src)+1);
	printf("err = %u\n", dasm_pp_execute(&pp));
	dasm_pp_reset_dst(&pp, NULL, 100);
	printf("err = %u\n", dasm_pp_execute(&pp));
	
	printf("-----------------\n");
	fwrite(dst, 1, pp.m_dst_cnt, stdout);
	printf("------------------\n");

	dasm_lexer_init(&lexer, toks, 100, pp.m_dst, pp.m_dst_cnt);
	printf("err = %u\n", dasm_lexer_execute(&lexer));

	printf("tok_cnt = %u\n", lexer.m_tok_cnt);

	for (i=0; i<lexer.m_tok_cnt; ++i) {
		for (j=0; j<lexer.m_toks[i].m_lexeme_len; ++j) {
			printf("%c", lexer.m_toks[i].m_lexeme[j]);
		}
		printf("\n");
	}

	printf("------------------\n");

	dasm_pp_deinit(&pp);
	dasm_lexer_deinit(&lexer);

	return 0;
}