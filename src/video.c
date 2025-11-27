extern void outportb(unsigned int, unsigned int);

unsigned char cursor_x  = 0;
unsigned char cursor_y  = 0;

unsigned char saved_cursor_x  = 0;
unsigned char saved_cursor_y  = 0;

void update_cursor()
{
	unsigned short position = cursor_y * 80 + cursor_x;
	// cursor HIGH port to vga INDEX register
	outportb(0x3D4, 0x0E);
	outportb(0x3D5, (unsigned char)((position>>8)&0xFF));
	// cursor LOW port to vga INDEX register
	outportb(0x3D4, 0x0F);
	outportb(0x3D5, (unsigned char)(position&0xFF));
};
