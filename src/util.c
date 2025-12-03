#include "os.h"
int INT_MAX = 2147483647;

inline void sti() {	asm volatile ( "sti" ); }	// Enable interrupts
inline void cli() { asm volatile ( "cli" ); }	// Disable interrupts
inline void nop() { asm volatile ( "nop" ); }	// Do nothing
oda_t* pODA = &ODA;
oda_t ODA;
void initODA()
{
    int i;
    for(i = 0; i < KQSIZE; ++i)
       pODA->KEYQUEUE[i] = 0;        // circular queue buffer

    pODA->pHeadKQ = pODA->KEYQUEUE;  // pointer to the head of valid data
    //for (;;);
//    pODA->pTailKQ = pODA->KEYQUEUE;  // pointer to the tail of valid data
//for (;;);    
    pODA->KQ_count_read  = 0;        // number of data read from queue buffer
    pODA->KQ_count_write = 0;        // number of data put into queue buffer
}

ULONG fetchESP()
{
    ULONG esp;
    asm volatile("mov %%esp, %0" : "=r"(esp));
    return esp;
}

ULONG fetchEBP()
{
    ULONG ebp;
    asm volatile("mov %%ebp, %0" : "=r"(ebp));
    return ebp;
}

/*
ULONG fetchEBP()
{
    register ULONG eax asm("%eax");
    asm volatile ( "movl %ebp,%eax" );
    return eax;
}
*/
ULONG fetchSS()
{
    register ULONG eax asm("%eax");
    asm volatile ( "movl %ss,%eax" );
    return eax;
}

ULONG fetchCS()
{
    register ULONG eax asm("%eax");
    asm volatile ( "movl %cs,%eax" );
    return eax;
}

ULONG fetchDS()
{
    register ULONG eax asm("%eax");
    asm volatile ( "movl %ds,%eax" );
    return eax;
}

inline UINT inportb(UINT port)
{
	UINT ret_val;
	asm volatile ("inb %w1,%b0"	: "=a"(ret_val)	: "d"(port));
	return ret_val;
}

inline void outportb(UINT port, UINT val)
{
    asm volatile ("outb %b0,%w1" : : "a"(val), "d"(port));
}

void panic_assert(const char* file, ULONG line, const char* desc)
{
    cli();
    printformat("ASSERTION FAILED(");
    printformat("%s",desc);
    printformat(") at ");
    printformat("%s",file);
    printformat(":");
    printformat("%i",line);
    printformat("OPERATING SYSTEM HALTED\n");
    // Halt by going into an infinite loop.
    for(;;);
}

void k_memshow(void* start, size_t count)
{
    const unsigned char* end = (const unsigned char*)(start+count);
    for(; count != 0; count--) printformat("%x ",*(end-count));
}

void* k_memcpy(void* dest, const void* src, size_t count)
{
    const unsigned char* sp = (const unsigned char*)src;
    unsigned char* dp = (unsigned char*)dest;
    for(; count != 0; count--) *dp++ = *sp++;
    return dest;
}

void* k_memset(void* dest, char val, size_t count)
{
    char* temp = (char*)dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}

USHORT* k_memsetw(USHORT* dest, USHORT val, size_t count)
{
    USHORT* temp = (USHORT*) dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}

size_t k_strlen(const char* str)
{
    size_t retval;
    for(retval = 0; *str != '\0'; ++str)
        ++retval;
    return retval;
}

// Compare two strings. Returns -1 if str1 < str2, 0 if they are equal or 1 otherwise.
int k_strcmp( const char* s1, const char* s2 )
{
    while ( ( *s1 ) && ( *s1 == *s2 ) )
    {
        ++s1;
        ++s2;
    }
    return ( *s1 - *s2 );
}

// Copy the NUL-terminated string src into dest, and return dest.
char* k_strcpy(char* dest, const char* src)
{
    do { *dest++ = *src++;} while(*src);
    return dest;
}

char* k_strncpy(char* dest, const char* src, size_t n)
{
    if(n != 0)
    {
        char* d       = dest;
        const char* s = src;
        do
        {
            if ((*d++ = *s++) == 0)
            {
                /* NUL pad the remaining n-1 bytes */
                while(--n != 0)
                   *d++ = 0;
                break;
            }
        }
        while(--n != 0);
     }
     return (dest);
}

char* k_strcat(char* dest, const char* src)
{
    char *d = dest;

    // ans Ende von dest
    while (*d != '\0') {
        d++;
    }

    // src Zeichen für Zeichen kopieren
    const char *s = src;
    while (*s != '\0') {
        *d = *s;
        d++;
        s++;
    }

    // Nullterminator setzen
    *d = '\0';

    return dest;
}

void reboot()
{
	int temp; // A temporary int for storing keyboard info. The keyboard is used to reboot
    do //flush the keyboard controller
    {
       temp = inportb( 0x64 );
       if( temp & 1 )
         inportb( 0x60 );
    }
	while ( temp & 2 );

    // Reboot
    outportb(0x64, 0xFE);
}

void k_itoa(int value, char* valuestring)
{
  int min_flag;
  char swap, *p;
  min_flag = 0;

  if (0 > value)
  {
    *valuestring++ = '-';
    value = -INT_MAX > value ? min_flag = INT_MAX : -value;
  }

  p = valuestring;

  do
  {
    *p++ = (char)(value % 10) + '0';
    value /= 10;
  } while (value);

  if (min_flag != 0)
  {
    ++*valuestring;
  }
  *p-- = '\0';

  while (p > valuestring)
  {
    swap = *valuestring;
    *valuestring++ = *p;
    *p-- = swap;
  }
}

void k_i2hex(UINT val, char* dest, int len)
{
	char* cp;
	char x;
	UINT n;
	n = val;
	cp = &dest[len];
	while (cp > dest)
	{
		x = n & 0xF;
		n >>= 4;
		*--cp = x + ((x > 9) ? 'A' - 10 : '0');
	}
    dest[len]  ='h';
    dest[len+1]='\0';
}

void float2string(float value, int decimal, char* valuestring) // float --> string
{
   int neg = 0;    char tempstr[20];
   int i = 0;   int j = 0;   int c;    long int val1, val2;
   char* tempstring;
   tempstring = valuestring;
   if (value < 0)
     {neg = 1; value = -value;}
   for (j=0; j < decimal; ++j)
     {value = value * 10;}
   val1 = (value * 2);
   val2 = (val1 / 2) + (val1 % 2);
   while (val2 !=0)
   {
     if ((decimal > 0) && (i == decimal))
     {
       tempstr[i] = (char)(0x2E);
       ++i;
     }
     else
     {
       c = (val2 % 10);
       tempstr[i] = (char) (c + 0x30);
       val2 = val2 / 10;
       ++i;
     }
   }
   if (neg)
   {
     *tempstring = '-';
      ++tempstring;
   }
   i--;
   for (;i > -1;i--)
   {
     *tempstring = tempstr[i];
     ++tempstring;
   }
   *tempstring = '\0';
}
