/*
 *  针对 kernel 所写的链接脚本 为GRUB准备的，待测试功能
 */
 
SECTIONS
{
	. = 0x100400;
	.text :
	{
		*(.text);
	}
	.data :
	{
		*(.data)
		*(.rodata);
	}
	.bss :
	{
		*(.bss);
	}
	.stab :
	{
		*(.stab);
	}
	.stabstr :
	{
		*(.stabstr);
	}
	.comment :{
		*(.comment);
	}
	
	.symtab :{
		*(.symtab);
	}
	.strtab  :{
		*(.strtab);
	}
	.shstrtab :{
		*(.shstrtab);
	}
}

