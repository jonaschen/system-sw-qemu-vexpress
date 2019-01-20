MEMORY
{
	USER	(RWX): ORIGIN = 0x0, LENGTH = 0x80000000

	KERNEL	(RWX) : ORIGIN = 0x80000000, LENGTH = 0x80000000
}
ENTRY(main)

SECTIONS
{
	.text :
	{
		_stext = .;
		*(.text)
		*(.text.*)
		*(.rodata)
		*(.rodata.*)

		. = ALIGN(4);
		_etext = .;
	} > USER

	.data :
	{
		. = ALIGN(4);
		_sdata = .;
		*(.data)
		*(.data*)

		. = ALIGN(4);
		_edata = .;
	} > USER

	.bss (NOLOAD):
	{
		. = ALIGN(4);
		_sbss = .;
		*(.bss)

		. = ALIGN(4);
		_ebss = .;
	} > USER
}

/* Note. */
/* KEEP: LD keeps the symbols in the section even if symbols are not referenced. */

