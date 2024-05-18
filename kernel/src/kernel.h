#pragma once

#include <types.h>
#include <limine.h>
#include <flanterm/flanterm.h>

extern u64 hhdm_offset;
extern struct limine_framebuffer *framebuffer;
extern struct flanterm_context *ft_ctx;

void* get_mod_addr(int pos);