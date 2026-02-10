#ifndef libdasm_pp_h
#define libdasm_pp_h

#include <libdice/type.h>
#include <dasm.h>


DICECALL libdice_word_t libdasm_preprocess_programme(char rdwr_dst[], const libdice_word_t c_dst_len, const char rd_src[], const libdice_word_t c_src_len);

#endif /* libdasm_pp_h */