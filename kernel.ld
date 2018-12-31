MEMORY
{
	RAM 	(RWX) : ORIGIN = 0x60004000, LENGTH = 0xFFFC000
	RAM2 	(RX) : ORIGIN = 0x70000000, LENGTH = 0x10000000
}
PROVIDE(normal_memory_base = 0x60000000);
PROVIDE(normal_memory_size = 0x20000000);
PROVIDE(peripheral_device_base = 0x10000000);
PROVIDE(peripheral_device_size = 0x10000000);

PROVIDE(__vector_base = 0x70000000);

/* 1MB Sections: 4096 X 4-byte entries for 4G virtual space */
PROVIDE(ttb_address = 0x60000000);
PROVIDE(TTB_DIR_SIZE = 0x4000);

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

