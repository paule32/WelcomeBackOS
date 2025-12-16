#pragma once
# include "stdint.h"

void gdt_init(uint32_t);
void tss_set_kernel_stack(uint32_t stack_top);
