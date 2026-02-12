#include <libdice.h>

ae2f_extern DICECALL libdice_ctx libdice_run(
		libdice_ctx					c_ctx,
		const libdice_put_interface* ae2f_restrict	rd_interface_put,
		const libdice_word_t* ae2f_restrict const 	rd_programme,
		const libdice_word_t				c_num_programme,
		libdice_word_t* ae2f_restrict const		rdwr_ram,
		const libdice_word_t				c_num_ram,
		libdice_word_t* ae2f_restrict const		rdwr_lookup,
		const libdice_word_t				c_num_lookup,
		volatile c89atomic_uint32* const	rdwr_a32lck
		)
{
	while(c_ctx.m_state == LIBDICE_CTX_GOOD) {
		c_ctx = libdice_run_one(
				c_ctx
				, rd_interface_put
				, rd_programme
				, c_num_programme
				, rdwr_ram, c_num_ram, rdwr_lookup
				, c_num_lookup
				, rdwr_a32lck
				);
	}

	return c_ctx;
}
