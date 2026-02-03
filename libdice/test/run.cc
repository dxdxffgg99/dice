#include <libdice.h>
#include <libdice/opcode.h>

#include <stdio.h>

int main(void) {
	libdice_ctx	CONTEXT;
	libdice_word_t	PROGRAMME[] = {
		LIBDICE_OPCODE_EQ, 1, 0, 0, 0, 0,
		LIBDICE_OPCODE_EOP
	};
	libdice_word_t	RAM[100];
	libdice_word_t	LOOKUP[100];

	CONTEXT.m_lookup_used	= 0;
	CONTEXT.m_pc		= 0;
	CONTEXT.m_state		= 0;

	CONTEXT = libdice_run(
			CONTEXT
			, PROGRAMME
			, sizeof(PROGRAMME) / sizeof(PROGRAMME[0])
			, RAM, sizeof(RAM) / sizeof(RAM[0])
			, LOOKUP, sizeof(LOOKUP) / sizeof(LOOKUP[0])
			);

	printf("Final State: %u\n", CONTEXT.m_state);

	return CONTEXT.m_state != LIBDICE_CTX_EOP;
}
