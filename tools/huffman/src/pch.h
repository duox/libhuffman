/*pch.h*/
#define _CRT_SECURE_NO_WARNINGS

#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include <memory.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//#define __debugbreak_if( expr )		if( expr )
//#define __debugbreak_ifnot( expr )	if( expr )
#define libhuffman_assert( expr )

#define libf2_unreferenced_parameter( param )	( param )

/*#define _countof( arr )		( sizeof(arr) / sizeof(*(arr)) )
#ifndef min
# define min( a, b )			(((a) <= (b)) ? (a) : (b))
# define max( a, b )			(((a) >= (b)) ? (a) : (b))
#endif // ndef min
*/
/*END OF pch.h*/
