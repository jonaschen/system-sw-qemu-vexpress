ENTRY(reset_handler)
MEMORY
{
	RAM 	(RWX) : ORIGIN = 0x60000000, LENGTH = 0x40000000
}

SECTIONS
{
	.text :
	{
		KEEP(*(.vectors))		/* LD keeps the symbols in the section even if symbols are not referenced. */
		*(.text)
		*(.text.*)
		*(.rodata)
		*(.rodata.*)
	} > RAM

	_estack = ORIGIN(RAM) + LENGTH(RAM);
}

