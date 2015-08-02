//***************************************************************************** 
//函数名:	Flash_New //功能描述:	EEPROM数据初始化程序 
//***************************************************************************** 
void Flash_New(void) { Flash_Reload();
 Rom_Logo = Logo;

	//数据准备 Rom_Ver = Version;
 Dividing_D = 0x03;
	
	//3槽 Steper_C_1 = 0x1291a43;
 Steper_C_2 = 0x1296741;
 Steper_C_3 = 0x12dadb3;

	Steper_C_4 = 0x12a2a28;
 Steper_C_5 = 0x128f33c;
 Steper_Conve_S = Conve_Dist;
	
	//传送带默认运行距离 Flash_Updata();
 
	}
	//***************************************************************************** 
	//函数名:	Flash_Reload 
	//功能描述:	EEPROM数据恢复程序 
	//***************************************************************************** 
	void Flash_Reload(void) { uchar i;
 pread = 0x8000;
 pgen = Rom_Dat;
 
	for(i=0;
 i<70;
 i++) 
	{ 
	*pgen = *pread;
 
	pread++;

	pgen++;

	}
	pgen = Rom_Dat;

	Rom_Logo = *(pgen+0);
 
	Rom_Ver = *(pgen+1);
 
	Dividing_D = *(pgen+2);

	Steper_C_1 = *((ulong *)pgen+8);

	Steper_C_2 = *((ulong *)pgen+9);
 
	Steper_C_3 = *((ulong *)pgen+10);
 
	Steper_C_4 = *((ulong *)pgen+11);

	Steper_C_5 = *((ulong *)pgen+12);

	Steper_C_6 = *((ulong *)pgen+13);
 
	Steper_C_7 = *((ulong *)pgen+14);
 
	Steper_C_8 = *((ulong *)pgen+15);
 
	Steper_Conve_S = *((ulong *)pgen+16);

	} 
	//*****************************************************************************
	//函数名:	Flash_Updata 
	//功能描述:	EEPROM数据更新程序 
	//***************************************************************************** 
	void Flash_Updata(void) 
	{ uchar i;

	EA = 0;

	//WDTCN = 0xde;

	//WDTCN = 0xad;

	FLSCL = 0x89;
 
	PSCTL = 0x03;
 
	pwrite = 0x8000;
 
	*pwrite = 0;
	
	//页擦除 PSCTL = 0x01;
 
	pgen = Rom_Dat_1;
 
	*(pgen+0) = Rom_Logo;
 *(pgen+1) = Rom_Ver;

	*(pgen+2) = Dividing_D;
 *(pgen+3) = 0x00;

	pwrite = 0x8000;
 
	for(i=0;
 i<4;
 i++)
	{ *pwrite = *pgen;

	pwrite++;
 pgen++;

	}
	pgen = Rom_Dat_1;

	*((ulong *)pgen) = Steper_C_1;

	pwrite = 0x8020;

	for(i=0;
 i<4;
 i++)
	{ *pwrite = *pgen;
 pwrite++;
 pgen++;
 }
	pgen = Rom_Dat_1;

	*((ulong *)pgen) = Steper_C_2;

	pwrite = 0x8024;
 
	for(i=0;
 i<4;
 i++)
	{ *pwrite = *pgen;
 pwrite++;
 pgen++;
 }
	pgen = Rom_Dat_1;
 
	*((ulong *)pgen) = Steper_C_3;
 pwrite = 0x8028;
 for(i=0;
 i<4;
 i++) { *pwrite = *pgen;
 pwrite++;
 pgen++;

 } pgen = Rom_Dat_1;
 *((ulong *)pgen) = Steper_C_4;
 pwrite = 0x802c;
 for(i=0;
 i<4;
 i++) { *pwrite = *pgen;
 pwrite++;
 pgen++;
 } pgen = Rom_Dat_1;
 *((ulong *)pgen) = Steper_C_5;
 pwrite = 0x8030;
 for(i=0;
 i<4;
 i++) { *pwrite = *pgen;
 pwrite++;
 pgen++;
 } pgen = Rom_Dat_1;
 *((ulong *)pgen) = Steper_C_6;
 pwrite = 0x8034;
 for(i=0;
 i<4;
 i++) { *pwrite = *pgen;
 pwrite++;
 pgen++;
 } pgen = Rom_Dat_1;
 *((ulong *)pgen) = Steper_C_7;
 pwrite = 0x8038;
 for(i=0;
 i<4;
 i++) { *pwrite = *pgen;
 pwrite++;
 pgen++;
 } pgen = Rom_Dat_1;
 *((ulong *)pgen) = Steper_C_8;
 pwrite = 0x803c;
 for(i=0;
 i<4;
 i++) { *pwrite = *pgen;
 pwrite++;
 pgen++;
 } pgen = Rom_Dat_1;
 *((ulong *)pgen) = Steper_Conve_S;
 pwrite = 0x8040;
 for(i=0;
 i<4;
 i++) { *pwrite = *pgen;
 pwrite++;
 pgen++;
 } PSCTL = 0x00;
 FLSCL = 0x8f;
 EA = 1;
 }