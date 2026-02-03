#ifndef libdice_type_h
#define libdice_type_h

enum LIBDICE_TYPE_ {
	LIBDICE_TYPE_I32,
	LIBDICE_TYPE_F32,
	LIBDICE_TYPE_PTR,
	LIBDICE_TYPE_FUNC,
	LIBDICE_TYPE_LBL
};

#include <ae2f/c90/StdInt.h>


typedef u32_least	libdice_word_t;

typedef union {
	libdice_word_t	m_word;
	i32_least	m_i32;
	u32_least	m_u32;

	float		m_f32;
} libdice_type_literal;

#endif
