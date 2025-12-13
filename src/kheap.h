#pragma once

typedef unsigned int uint32_t;

void kheap_init(void);
void* kmalloc(uint32_t size);
void  kfree(void* ptr);
