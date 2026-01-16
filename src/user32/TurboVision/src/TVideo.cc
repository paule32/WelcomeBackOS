# include "stdint.h"
# include "stdarg.h"

static unsigned char  csr_x  = 0;
static unsigned char  csr_y  = 0;
static unsigned char  saved_csr_x  = 0;
static unsigned char  saved_csr_y  = 0;
static unsigned char  attrib = 0x0F;

static USHORT* tui_vidmem = (USHORT*) 0xb8000;
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %0, %1" :: "a"(val), "dN"(port));
}

extern void ki2hex(UINT val, char* dest, int len);
extern void kitoa(int value, char* valuestring);

void update_cursor()
{
	USHORT position = csr_y * 80 + csr_x;

	// cursor HIGH port to vga INDEX register
	outb(0x3D4, 0x0E);
	outb(0x3D5, (unsigned char)((position>>8) & 0xFF));

	// cursor LOW port to vga INDEX register
	outb(0x3D4, 0x0F);
	outb(0x3D5, (unsigned char)(position & 0xFF));
};

void scroll()
{
    // UINT blank, temp;
    //blank = 0x20 | (attrib << 8);
    if(csr_y >= 25)
    {
    /*
        temp = csr_y - 25 + 1;
        kmemcpy (tui_vidmem, tui_vidmem + temp * 80, (25 - temp) * 80 * 2);
        kmemsetw (tui_vidmem + (25 - temp) * 80, blank, 80);
        csr_y = 25 - 1;
    */
    csr_y = 1;
    csr_x = 1;
    }
}

void move_cursor_right()
{
    ++csr_x;
    if(csr_x>=80)
    {
      ++csr_y;
      csr_x=0;
    }
}

void move_cursor_left()
{
    if(csr_x)
        --csr_x;
    if(!csr_x && csr_y>0)
    {
        csr_x=79;
        --csr_y;
    }
}

void move_cursor_home()
{
    csr_x = 0; update_cursor();
}

void move_cursor_end()
{
    csr_x = 79; update_cursor();
}

void set_cursor(unsigned char x, unsigned char y)
{
    csr_x = x; csr_y = y; update_cursor();
}

void putch(char c)
{
    USHORT* pos;
    UINT att = attrib << 8;

    if(c == 0x08) // backspace: move the cursor back one space and delete
    {
        if(csr_x)
        {
            --csr_x;
            putch(' ');
            --csr_x;
        }
        if(!csr_x && csr_y>0)
        {
            csr_x=79;
            --csr_y;
            putch(' ');
            csr_x=79;
            --csr_y;
        }
    }
    else if(c == 0x09) // tab: increment csr_x (divisible by 8)
    {
        csr_x = (csr_x + 8) & ~(8 - 1);
    }
    else if(c == '\r') // cr: cursor back to the margin
    {
        csr_x = 0;
    }
    else if(c == '\n') // newline: like 'cr': cursor to the margin and increment csr_y
    {
        csr_x = 0; ++csr_y;
    }
    /* Any character greater than and including a space, is a printable character.
    *  Index = [(y * width) + x] */
    else if(c >= ' ')
    {
        pos = tui_vidmem + (csr_y * 80 + csr_x);
        *pos = c | att; // Character AND attributes: color
        ++csr_x;
    }

    if(csr_x >= 80) // cursor reaches edge of the screen's width, a new line is inserted
    {
        csr_x = 0;
        ++csr_y;
    }

    /* Scroll the screen if needed, and finally move the cursor */
    scroll();
    update_cursor();
}

void puts(char* text)
{
    for(; *text; putch(*text), ++text);
}

/* Lean version of printf: printformat(...): supports %u, %d, %x/%X, %s, %c */
extern "C" void printformat (char *args, ...)
{
	va_list ap;
	va_start (ap, args);
	int index = 0, d;
	UINT u;
	char c, *s;
	char buffer[256];

	while (args[index])
	{
		switch (args[index])
		{
		case '%':
			++index;
			switch (args[index])
			{
			case 'u':
				u = va_arg (ap, UINT);
				kitoa(u, buffer);
				puts(buffer);
				break;
			case 'd':
			case 'i':
				d = va_arg (ap, int);
				kitoa(d, buffer);
				puts(buffer);
				break;
			case 'X':
			case 'x':
				d = va_arg (ap, int);
				ki2hex(d, buffer,8);
				puts(buffer);
				break;
			case 's':
				s = va_arg (ap, char*);
				puts(s);
				break;
			case 'c':
				c = (char) va_arg (ap, int);
				putch(c);
				break;
			default:
				putch('%');
				putch('%');
				break;
			}
			break;

		default:
			putch(args[index]); //printf("%c",*(args+index));
			break;
		}
		++index;
	}
}
