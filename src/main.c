/*main.c*/
#include "pch.h"
#include "main.h"

unsigned next_power_of_two( unsigned long v )
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

unsigned log2_uint64( uint64_t n )
{
    #define S(k) if (n >= (UINT64_C(1) << k)) { i += k; n >>= k; }

    unsigned i = -(n == 0);
    S(32);
    S(16);
    S(8);
    S(4);
    S(2);
    S(1);
    return i;

    #undef S
}

/*END OF main.c*/
