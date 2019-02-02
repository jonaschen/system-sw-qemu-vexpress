MEMORY
{
	USER_CODE	(RWX): ORIGIN = 0x0, LENGTH = 0x8000
	USER_DATA	(RWX): ORIGIN = 0x8000, LENGTH = 0x8000

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

		_etext = .;
	} > USER_CODE

	.data :
	{
		. = ALIGN(4);
		_sdata = .;
		*(.data)
		*(.data*)

		. = ALIGN(4);
		_edata = .;
	} > USER_DATA

	.bss (NOLOAD):
	{
		. = ALIGN(4);
		_sbss = .;
		*(.bss)

		. = ALIGN(4);
		_ebss = .;
	} > USER_DATA
}

/* Note. */
/* KEEP: LD keeps the symbols in the section even if symbols are not referenced. */

