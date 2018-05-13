MEMORY
{
	RAM 	(RWX) : ORIGIN = 0x60000000, LENGTH = 0x10000000
	RAM2 	(RX) : ORIGIN = 0x70000000, LENGTH = 0x10000000
}

SECTIONS
{
	.text :
	{
		KEEP(*(.vectors))
		*(.text)
		*(.text.*)
		*(.rodata)
		*(.rodata.*)

		. = ALIGN(4);
		_sidata = .;
	} > RAM2

	.data : AT(_sidata)
	{
		. = ALIGN(4);
		_sdata = .;
		*(.data)
		*(.data*)
		_edata = .;
	} > RAM

	.bss :
	{
		. = ALIGN(4);
		_sbss = .;
		*(.bss)
		_ebss = .;
	} > RAM

	_estack = ORIGIN(RAM) + LENGTH(RAM);
}

/* Note. */
/* KEEP: LD keeps the symbols in the section even if symbols are not referenced. */

