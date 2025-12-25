#ifndef ORDERED_ARRAY_H
#define ORDERED_ARRAY_H

#include "os.h"

typedef void* type_t;
typedef char (*lessthan_predicate_t)(type_t,type_t);

typedef struct
{
    type_t* array;
    ULONG size;
    ULONG max_size;
    lessthan_predicate_t less_than;
} ordered_array_t;

char standard_lessthan_predicate(type_t a, type_t b);

ordered_array_t create_ordered_array  (ULONG max_size, lessthan_predicate_t less_than);
ordered_array_t place_ordered_array   (void* addr, ULONG max_size, lessthan_predicate_t less_than);
void            insert_ordered_array  (type_t item, ordered_array_t* array);
type_t          lookup_ordered_array  (ULONG i, ordered_array_t* array);
void            remove_ordered_array  (ULONG i, ordered_array_t* array);
void            destroy_ordered_array (ordered_array_t* array);

extern ULONG k_malloc( ULONG size, unsigned char align, ULONG* phys );
extern void kfree( void* p );
#endif
