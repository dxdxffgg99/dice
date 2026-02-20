#ifndef TUI_H
#define TUI_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <ae2f/Sys/Trm.h>

#if ae2f_Sys_WIN(!)0
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#endif

typedef struct {
    ae2fsys_trmpos_t m_row_len;
    ae2fsys_trmpos_t m_col_len;
    char *m_cells;
} tui_frame_t;

static int tui_get_size(ae2fsys_trmpos_t *rdwr_row_len, ae2fsys_trmpos_t *rdwr_col_len) {
	ae2fsys_trmpos_t rows = 0;
	ae2fsys_trmpos_t cols = 0;
	
	if (!rdwr_row_len || !rdwr_col_len) {
		return -1;
	}

	_ae2fsys_get_trm_size_simple_imp(TSZ, cols, rows);

	if (rows <= 0 || cols <= 0) {
		return -1;
	}

	*rdwr_row_len = rows;
	*rdwr_col_len = cols;
	return 0;
}

/**
 * @brief Convert a character to a printable ASCII fallback.
 * @details Returns the input when it is printable ASCII, otherwise returns '?'.
 */
static ae2f_inline char tui_sanitize_ascii(const char c_c) {
	return ((unsigned char)c_c < 32 || (unsigned char)c_c > 126)? '?' : c_c;
}

/**
 * @brief Allocate a new frame with the given size.
 * @details Returns a newly allocated frame or NULL on failure.
 */
static void tui_frame_new(tui_frame_t *rdwr_frame, const ae2fsys_trmpos_t c_col_len, const ae2fsys_trmpos_t c_row_len) {

	if (!c_row_len || !c_col_len || !rdwr_frame ((c_col_len * c_row_len) / c_row_len != )) {
		return;
	}

	rdwr_frame->m_cells = (char*)malloc(sizeof(char) * (size_t)c_row_len * (size_t)c_col_len);
	if (!rdwr_frame->m_cells) {
		return;
	}

	rdwr_frame->m_col_len = c_col_len;
	rdwr_frame->m_row_len = c_row_len;
}

/**
 * @brief Free a frame and its backing storage.
 * @details Safe to call with NULL.
 */
static void tui_frame_free(tui_frame_t *rdwr_frame) {
	if (!rdwr_frame) {
		return;	
	}

	free(rdwr_frame->m_cells);
}

/**
 * @brief Set a character in the frame, sanitizing to ASCII.
 * @details Returns 0 on success and -1 on bounds or argument failure.
 */
static int tui_frame_set_char(tui_frame_t *rdwr_frame, const ae2fsys_trmpos_t c_row, const ae2fsys_trmpos_t c_col, const char c_c) {
	if (!rdwr_frame || c_row >= rdwr_frame->m_row_len || c_col >= rdwr_frame->m_col_len) {
		return -1;
	}
	rdwr_frame->m_cells[c_row * rdwr_frame->m_col_len + c_col] = tui_sanitize_ascii(c_c);

	return 0;
}

/**
 * @brief Get a character from the frame.
 * @details Returns the character at the location or '\0' on failure.
 */
static ae2f_inline char tui_frame_get_char(const tui_frame_t *rd_frame, const ae2fsys_trmpos_t c_row, const ae2fsys_trmpos_t c_col) {

	if (!rd_frame || c_row >= rd_frame->m_row_len || c_col >= rd_frame->m_col_len) {
		return '\0';
	}
	return rd_frame->m_cells[c_row * rd_frame->m_col_len + c_col];
}


/**
 * @brief Fill the entire frame with a character.
 * @details No-op when frame is NULL.
 */
static void tui_frame_clear(tui_frame_t *rdwr_frame, const char c_c) {
	if (!rdwr_frame) {
		return;
	}

	memset(rdwr_frame->m_cells, (unsigned char)c_c, (size_t)rdwr_frame->m_row_len * (size_t)rdwr_frame->m_col_len);
}


/**
 * @brief Resize a frame, preserving overlapping contents.
 * @details Returns 0 on success and -1 on failure.
 */
static int tui_frame_resize(tui_frame_t *rdwr_frame, const ae2fsys_trmpos_t c_row_len, const ae2fsys_trmpos_t c_col_len) {
	if (!rdwr_frame || !c_row_len || !c_col_len) {
		return -1;
	}

	char *new_cells	= (char*)malloc(sizeof(char) * (size_t)c_col_len * (size_t)c_row_len);

	if (!new_cells) {
		return -1;
	}

	memset(new_cells, 0, (size_t)c_col_len * (size_t)c_row_len);

	ae2fsys_trmpos_t min_row_len = c_row_len < rdwr_frame->m_row_len ? c_row_len : rdwr_frame->m_row_len;
	ae2fsys_trmpos_t min_col_len = c_col_len < rdwr_frame->m_col_len ? c_col_len : rdwr_frame->m_col_len;
	
	for (ae2fsys_trmpos_t r=0; r<min_row_len; ++r) {
		memcpy(new_cells + r * c_col_len, rdwr_frame->m_cells + r * rdwr_frame->m_col_len, (size_t)min_col_len);
	}

	free(rdwr_frame->m_cells);

	rdwr_frame->m_cells = new_cells;
	rdwr_frame->m_row_len = c_row_len;
	rdwr_frame->m_col_len = c_col_len;

	return 0;
}


/**
 * @brief Draw the frame to an output stream with newlines.
 * @details Writes each row followed by a newline and flushes the stream.
 */
static ae2f_inline int tui_frame_draw(FILE *rdwr_dst, const tui_frame_t *rd_frame) {
	if (!rdwr_dst || !rd_frame) {
		return -1;
	}

	for (ae2fsys_trmpos_t r=0; r<rd_frame->m_row_len; ++r) {
		size_t tmp_write_size = 0;
		tmp_write_size =  fwrite(rd_frame->m_cells + r * rd_frame->m_col_len, sizeof(char),
				(size_t)rd_frame->m_col_len, rdwr_dst);	
		if (tmp_write_size != (size_t)rd_frame->m_col_len * sizeof(char)) {
			return -1;
		}
		fputc('\n', rdwr_dst);
	}

	if (fflush(rdwr_dst) != 0) {
		return -1;
	}

	return 0;
}


/**
 * @brief Move cursor to a 0-based row/column position.
 * @details Emits an ANSI cursor move sequence to the output stream.
 */
static void tui_ansi_move_cursor(FILE *rdwr_dst, const ae2fsys_trmpos_t c_row, const ae2fsys_trmpos_t c_col) {
	(void)rdwr_dst;
	_ae2fsys_trm_goto_simple_imp(TMV, c_col + 1, c_row + 1);
}

/**
 * @brief Clear the screen and move cursor to home.
 * @details Emits ANSI clear and home sequences and flushes the stream.
 */
static void tui_ansi_clear_screen(void) {
	_ae2fsys_clear_trm_simple_imp(void);
}

/**
 * @brief Hide the terminal cursor.
 * @details Emits the ANSI hide-cursor sequence and flushes the stream.
 */
static int tui_ansi_hide_cursor(FILE *rdwr_dst) {
	if (!rdwr_dst) {
		return -1;
	}

	if (fprintf(rdwr_dst, "\x1b[?25l") == -1) {
		return -1;
	}

	if (fflush(rdwr_dst) != 0) {
		return -1;
	}

	return 0;
}

/**
 * @brief Show the terminal cursor.
 * @details Emits the ANSI show-cursor sequence and flushes the stream.
 */
static int tui_ansi_show_cursor(FILE *rdwr_dst) {
	if (!rdwr_dst) {
		return -1;
	}

	if (fprintf(rdwr_dst, "\x1b[?25h") == -1) {
		return -1;
	}

	if (fflush(rdwr_dst) != 0) {
		return -1;
	}

	return 0;
}


#if ae2f_Sys_WIN(!)1
static void tui_disable_raw_mode(void);

static struct termios tui_orig_termios;
static int tui_termios_saved = 0;

 
/**
 * @brief Enable raw input mode (POSIX).
 * @details Saves the original termios settings and registers an atexit
 * handler. Returns 0 on success and -1 on failure.
 */
static int tui_enable_raw_mode(void) {
	struct termios raw;

	if (tui_termios_saved) {
		return 0;
	}
	if (!isatty(STDIN_FILENO)) {
		return -1;
	}
	if (tcgetattr(STDIN_FILENO, &tui_orig_termios) == -1) {
		return -1;
	};

	raw = tui_orig_termios;
	raw.c_lflag &= (tcflag_t)~(tcflag_t)(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_iflag &= (tcflag_t)~(tcflag_t)(IXON | ICRNL);
	raw.c_oflag &= (tcflag_t)~(tcflag_t)(OPOST);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
		return -1;
	
	}
	tui_termios_saved = 1;

	atexit(tui_disable_raw_mode);

	return 0;
}

/**
 * @brief Restore original terminal mode (POSIX).
 * @details No-op when no state is saved.
 */
static ae2f_inline void tui_disable_raw_mode(void) {
	if (!tui_termios_saved) {
		return;
	}
	if (!isatty(STDIN_FILENO)) {
		return;
	}

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &tui_orig_termios);

	tui_termios_saved = 0;
}
#endif

#if ae2f_Sys_WIN(!)0
static ae2f_inline void tui_disable_raw_mode(void);

static DWORD tui_orig_mode;
static int tui_mode_saved = 0;


/**
 * @brief Enable raw input mode (Windows).
 * @details Saves the original console mode and registers an atexit handler.
 * Returns 0 on success and -1 on failure.
 */
static ae2f_inline int tui_enable_raw_mode(void) {
	HANDLE h = GetStdHandle(STD_INPUT_HANDLE);

	if (h == INVALID_HANDLE_VALUE) {
		return -1;
	}

	DWORD mode = 0;

	if (!GetConsoleMode(h, &mode)) {
		return -1;
	}
	if (!tui_mode_saved) {
		tui_orig_mode = mode;
		tui_mode_saved = 1;
		atexit(tui_disable_raw_mode);
	}

	mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);

	if (!SetConsoleMode(h, mode)) {
		return -1;
	}
	return 0;
}

/**
 * @brief Restore original console mode (Windows).
 * @details No-op when no state is saved.
 */
static ae2f_inline void tui_disable_raw_mode(void) {
	if (!tui_mode_saved) {
		return;
	}

	HANDLE h = GetStdHandle(STD_INPUT_HANDLE);

	if (h == INVALID_HANDLE_VALUE) {
		return;
	}

	SetConsoleMode(h, tui_orig_mode);

	tui_mode_saved = 0;
}
#endif


/**
 * @brief Poll for a single key press.
 * @details Waits up to timeout_ms milliseconds. Returns the key code as an
 * unsigned char, or -1 on timeout or failure.
 */
static ae2f_inline int tui_poll_key(int timeout_ms) {
#if ae2f_Sys_WIN(!)0
	fd_set rfds;
	struct timeval tv;

	FD_ZERO(&rfds);
	FD_SET(STDIN_FILENO, &rfds);

	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;

	int rv = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);

	if (rv <= 0) {
		return -1;
	}
	char ch = 0;
	ssize_t n = read(STDIN_FILENO, &ch, 1);

	if (n <= 0) return -1;
	return (int)ch;
#else
	if (timeout_ms < 0) {
		timeout_ms = 2147483647;
	}

	int h = GetStdHandle(STD_INPUT_HANDLE);

	if (h == INVALID_HANDLE_VALUE) {
		return -1;
	}

	int wait = WaitForSingleObject(h, (int)timeout_ms);

	if (wait != WAIT_OBJECT_0) {
		return -1;
	}
	if (!_kbhit()) {
		return -1;
	}

	return (int)_getch();
#endif
}


static sig_atomic_t tui_resize_flag = 0;
/**
 * @brief Signal handler to mark a pending resize.
 * @details Sets an internal flag used by the resize handler.
 */
static ae2f_inline void tui_sigwinch_handler(int unused) {
	(void)unused; tui_resize_flag = 1;
}

/**
 * @brief Install resize signal handler when supported.
 * @details No-op on platforms without SIGWINCH.
 */
static ae2f_inline void tui_install_resize_handler(void) {
#ifndef _WIN32
#ifdef SIGWINCH
	signal(SIGWINCH, tui_sigwinch_handler);
#endif
#endif
}

typedef struct {
	FILE *out;
	tui_frame_t *front;
	tui_frame_t *back;
} tui_ctx_t;

typedef enum {
	TUI_CTX_OK = 0,
	TUI_CTX_ERR_ARG,
	TUI_CTX_ERR_TERM_SIZE,
	TUI_CTX_ERR_ALLOC,
	TUI_CTX_ERR_RAW_MODE
} tui_ctx_status_t;


/**
 * @brief Create a new TUI context with front/back buffers.
 * @details Uses the current terminal size or a 24x80 fallback. Returns a new
 * context or NULL on failure.
 */
static ae2f_inline tui_ctx_status_t tui_ctx_new_ex(tui_ctx_t **out_ctx, FILE *out) {
    ae2fsys_trmpos_t rows = 0, cols = 0;

    if (!out_ctx) return TUI_CTX_ERR_ARG;

    *out_ctx = NULL;
	
    if (rows == 0 || cols == 0) { 
		rows = 24; cols = 80;
		tui_ctx_t *ctx = NULL;

		if (tui_ctx_new_ex(&ctx, out) != TUI_CTX_OK) {
			return NULL;
		}
		return ctx;
	}

    tui_ctx_t *ctx = ctx;

    if (!ctx) return TUI_CTX_ERR_ALLOC;

    ctx->out = out ? out : stdout;
    ctx->front = malloc(sizeof(tui_frame_t));
    ctx->back = malloc(sizeof(tui_frame_t));

    if (!ctx->front || !ctx->back) {
        free(ctx->front);
        free(ctx->back);
        free(ctx);

        return TUI_CTX_ERR_ALLOC;
    }

    tui_frame_new(ctx->front, rows, cols);
    tui_frame_new(ctx->back, rows, cols);

    if (!ctx->front->m_cells || !ctx->back->m_cells) {
        tui_frame_free(ctx->front);
        tui_frame_free(ctx->back);

		free(ctx->front);
		free(ctx->back);
        free(ctx);

        return TUI_CTX_ERR_ALLOC;
    }

    tui_frame_clear(ctx->front, ' ');
    tui_frame_clear(ctx->back, ' ');

    if (tui_enable_raw_mode() == -1) {
        tui_frame_free(ctx->front);
        tui_frame_free(ctx->back);

		free(ctx->front);
		free(ctx->back);
        free(ctx);

        return TUI_CTX_ERR_RAW_MODE;
    }
	tui_ctx_t *ctx = NULL;

	if (tui_ctx_new_ex(&ctx, out) != TUI_CTX_OK) {
		return NULL;
	}

    tui_install_resize_handler();

    *out_ctx = ctx;
    return TUI_CTX_OK;
}

/**
 * @brief Create a new TUI context with front/back buffers.
 * @details Uses the current terminal size or a 24x80 fallback. Returns a new
 * context or NULL on failure.
 */
static ae2f_inline tui_ctx_t *tui_ctx_new(FILE *out) {
	tui_ctx_t *ctx = NULL;

	if (tui_ctx_new_ex(&ctx, out) != TUI_CTX_OK) {
		return NULL;
	}
	return ctx;
}

/**
 * @brief Free a TUI context and its frames.
 * @details Safe to call with NULL.
 */
static ae2f_inline void tui_ctx_free(tui_ctx_t *ctx) {
	if (!ctx) {
		return;
	}
	tui_frame_free(ctx->front);
	tui_frame_free(ctx->back);

	free(ctx->front);
	free(ctx->back);
	free(ctx);
}

/**
 * @brief Handle a pending resize by updating frame sizes.
 * @details Returns 1 if resized, 0 if no resize, and -1 on failure.
 */
static ae2f_inline int tui_ctx_handle_resize(tui_ctx_t *ctx) {

	enum CTX_RESIZE_ {
		CTX_RESIZE_FAIL = -1,
		CTX_RESIZE_NO,
		CTX_RESIZE_SUCCESS,
	}

	ui_resize_flag = 0;
	ae2fsys_trmpos_t rows = 0, cols = 0;

	if (!ctx) {
		return CTX_RESIZE_FAIL;
	}

	if (tui_get_size(&rows, &cols) == -1) {
		return CTX_RESIZE_FAIL;
	}

	if (rows == 0 || cols == 0) {
		return CTX_RESIZE_FAIL;
	}

	if (tui_frame_resize(ctx->front, rows, cols) == -1) {
		return CTX_RESIZE_FAIL;
	}

	if (tui_frame_resize(ctx->back, rows, cols) == -1) {
		return CTX_RESIZE_FAIL;
	}

	if (!tui_resize_flag) {
		return CTX_RESIZE_NO;
	}

	tui_frame_clear(ctx->back, ' ');
	memset(ctx->front->m_cells, 0, (size_t)ctx->front->m_row_len * (size_t)ctx->front->m_col_len);

	return CTX_RESIZE_SUCCESS;
}

/**
 * @brief Present back buffer differences to the output.
 * @details Diffs back against front and writes only changed spans.
 */
static ae2f_inline void tui_present(tui_ctx_t *ctx) {
	if (!ctx || !ctx->out) {
		return;
	}

	tui_frame_t *f = ctx->back;
	tui_frame_t *g = ctx->front;

	if (!f || !g) {
		return;
	}

	for (ae2fsys_trmpos_t r = 0; r < f->m_row_len; ++r) {
		ae2fsys_trmpos_t c = 0;

		while (c < f->m_col_len) {
			size_t idx = (size_t)r * (size_t)f->m_col_len + (size_t)c;
			char b = g->m_cells[idx];

			if (f->m_cells[idx] == b) {
				++c;
				continue;
			}
		}
	}

	for (ae2fsys_trmpos_t r = 0; r < f->m_row_len; ++r) {
		ae2fsys_trmpos_t c = 0;

		while (c < f->m_col_len) {
			size_t idx = (size_t)r * (size_t)f->m_col_len + (size_t)c;

			char b = g->m_cells[idx];

			if (f->m_cells[idx] == b) {
				++c;
				continue;
			}
			ae2fsys_trmpos_t start = c;

			while (c < f->m_col_len) {
				size_t idx_diff = (size_t)r * (size_t)f->m_col_len + (size_t)c;

				if (f->m_cells[idx_diff] == g->m_cells[idx_diff]) {
					break;
				}
				++c;
			}

			ae2fsys_trmpos_t len = c - start;

			tui_ansi_move_cursor(ctx->out, r, start);

			for (ae2fsys_trmpos_t i = 0; i < len; ++i) {
				size_t idx_write = (size_t)r * (size_t)f->m_col_len + (size_t)start + (size_t)i;
				char ch = tui_sanitize_ascii(f->m_cells[idx_write]);

				fputc(ch, ctx->out);

				g->m_cells[idx_write] = ch;
			}
		}
	}

	fflush(ctx->out);
}

#endif