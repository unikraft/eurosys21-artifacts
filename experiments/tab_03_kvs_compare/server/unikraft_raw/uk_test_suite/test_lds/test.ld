SECTIONS
{
	.test_ld_data : {
		KEEP (*(.test_ld_data))
	}
}
INSERT BEFORE .rodata

SECTIONS
{
	.zzzzz : {
		KEEP (*(.zzzzz))
	}
}
INSERT BEFORE .rodata
