#include <libdice.h>

ae2f_extern DICECALL libdice_ctx libdice_run(
		libdice_ctx					c_ctx,
		const libdice_word_t* ae2f_restrict const 	rd_programme,
		const libdice_word_t				c_num_programme,
		libdice_word_t* ae2f_restrict const		rdwr_ram,
		const libdice_word_t				c_num_ram,
		libdice_word_t* ae2f_restrict const		rdwr_lookup,
		const libdice_word_t				c_num_lookup
		)
{
	while(!c_ctx.m_state) {
		c_ctx = libdice_run_one(c_ctx, rd_programme, c_num_programme
				, rdwr_ram, c_num_ram, rdwr_lookup
				, c_num_lookup);
	}

	return c_ctx;
}
