#include "dice/tui.h"

#include <ae2f/Sys/Trm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

dice_tui_ctx_t dice_tui_ctx = {0};

static dice_tui_status_t dice_tui_realloc_buffers(size_t total) {
    char *n_back = (char *)malloc(total);
    char *n_prev = (char *)malloc(total);

    ae2f_unexpected_but_if(!(n_back && n_prev)) {
        free(n_back);
        free(n_prev);
        return DICE_TUI_ERR_MALLOC_FAILED;
    }

    free(dice_tui_ctx.m_back);
    free(dice_tui_ctx.m_prev);

    dice_tui_ctx.m_back = n_back;
    dice_tui_ctx.m_prev = n_prev;

    memset(dice_tui_ctx.m_back, ' ', total);
    memset(dice_tui_ctx.m_prev, '\0', total);
    return DICE_TUI_SUCCESS;
}

dice_tui_status_t dice_tui_get_size(void) {
    size_t w, h;

    _ae2fsys_get_trm_size_simple_imp(L, dice_tui_ctx.m_width, dice_tui_ctx.m_height);

    ae2f_unexpected_but_if(dice_tui_ctx.m_width <= 0 || dice_tui_ctx.m_height <= 0) {
        return DICE_TUI_ERR_SCREEN_TOO_SMALL;
    }

    w = (size_t)dice_tui_ctx.m_width;
    h = (size_t)dice_tui_ctx.m_height;

    ae2f_unexpected_but_if(h != 0u && w > SIZE_MAX / h) {
        return DICE_TUI_ERR_BUFFER_OVERFLOW;
    }
    return DICE_TUI_SUCCESS;
}

dice_tui_status_t dice_tui_init(void) {
    dice_tui_status_t status;
    size_t total;

    if (dice_tui_ctx.init_flag) {
        return DICE_TUI_ERR_UNKNOWN;
    }

    status = dice_tui_get_size();
    if (status != DICE_TUI_SUCCESS) {
        return status;
    }

    total = (size_t)dice_tui_ctx.m_width * (size_t)dice_tui_ctx.m_height;

    _ae2fsys_clear_trm_simple_imp(L);

    status = dice_tui_realloc_buffers(total);
    ae2f_unexpected_but_if(status != DICE_TUI_SUCCESS) {
        return status;
    }

    dice_tui_ctx.init_flag = 1;

    return DICE_TUI_SUCCESS;
}

dice_tui_status_t dice_tui_resize(void) {
    dice_tui_status_t status;
    ae2fsys_trmpos_t old_width;
    ae2fsys_trmpos_t old_height;
    size_t total;

    old_width = dice_tui_ctx.m_width;
    old_height = dice_tui_ctx.m_height;

    status = dice_tui_get_size();
    ae2f_unexpected_but_if(status != DICE_TUI_SUCCESS) {
        return status;
    }

    total = (size_t)dice_tui_ctx.m_width * (size_t)dice_tui_ctx.m_height;
    
    status = dice_tui_realloc_buffers(total);
    ae2f_unexpected_but_if(status != DICE_TUI_SUCCESS) {
        dice_tui_ctx.m_width = old_width;
        dice_tui_ctx.m_height = old_height;
        return status;
    }

    _ae2fsys_clear_trm_simple_imp(L);

    return DICE_TUI_SUCCESS;
}

ae2f_inline dice_tui_status_t dice_tui_set_char(ae2fsys_trmpos_t x, ae2fsys_trmpos_t y, char c) {
    size_t idx;

    ae2f_unexpected_but_if(!dice_tui_ctx.m_back) {
        return DICE_TUI_ERR_NULL_POINTER;
    }

    ae2f_unexpected_but_if(x < 0 || y < 0) {
        return DICE_TUI_ERR_COORDINATE_NEGATIVE;
    }

    ae2f_unexpected_but_if(x >= dice_tui_ctx.m_width || y >= dice_tui_ctx.m_height) {
        return DICE_TUI_ERR_OUT_OF_BOUNDS;
    }

    idx = (size_t)y * (size_t)dice_tui_ctx.m_width + (size_t)x;
    dice_tui_ctx.m_back[idx] = c;

    return DICE_TUI_SUCCESS;
}

ae2f_inline dice_tui_status_t dice_tui_render(void) {
    ae2f_unexpected_but_if(!(dice_tui_ctx.m_back && dice_tui_ctx.m_prev)) {
        return DICE_TUI_ERR_NULL_POINTER;
    }

    size_t width = (size_t)dice_tui_ctx.m_width;
    size_t height = (size_t)dice_tui_ctx.m_height;

    for (size_t y = 0; y < height; ++y) {
        size_t row_offset = y * width;
        size_t x = 0;

        while (x < width) {
            size_t i = row_offset + x;

            if (dice_tui_ctx.m_back[i] == dice_tui_ctx.m_prev[i]) {
                x++;
                continue;
            }

            _ae2fsys_trm_goto_simple_imp(L, (ae2fsys_trmpos_t)(x + 1), (ae2fsys_trmpos_t)(y + 1));

            size_t start_x = x;

            while (x < width && dice_tui_ctx.m_back[row_offset + x] != dice_tui_ctx.m_prev[row_offset + x]) {
                dice_tui_ctx.m_prev[row_offset + x] = dice_tui_ctx.m_back[row_offset + x];
                x++;
            }

            if (fwrite(&dice_tui_ctx.m_prev[row_offset + start_x], sizeof(char), x - start_x, stdout) != x - start_x) {
                return DICE_TUI_ERR_IO_FAILED;
            }
        }
    }

    ae2f_unexpected_but_if(fflush(stdout) != 0) {
        return DICE_TUI_ERR_IO_FAILED;
    }

    return DICE_TUI_SUCCESS;
}

ae2f_inline dice_tui_status_t dice_tui_clear(void) {
    size_t total;

    ae2f_unexpected_but_if(!(dice_tui_ctx.m_back && dice_tui_ctx.m_prev)) {
        return DICE_TUI_ERR_NULL_POINTER;
    }
    
    total = (size_t)dice_tui_ctx.m_width * (size_t)dice_tui_ctx.m_height;
    memset(dice_tui_ctx.m_back, ' ', total);

    return DICE_TUI_SUCCESS;
}

void dice_tui_shutdown(void) {
    free(dice_tui_ctx.m_back);
    free(dice_tui_ctx.m_prev);
    memset(&dice_tui_ctx, 0, sizeof(dice_tui_ctx));
}
