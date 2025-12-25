# include "proto.h"

int INT_MAX = 2147483647;

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

void kitoa(int value, char* valuestring)
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

void ki2hex(UINT val, char* dest, int len)
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
