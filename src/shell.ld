ENTRY(_shell_main)
SECTIONS
{
    . = 0;

    .text : {
        *(.text*)
    }
    ___end = .;
}
