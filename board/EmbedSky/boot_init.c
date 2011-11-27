#include <common.h>
#include <s3c2410.h>

#define GSTATUS1        (*(volatile unsigned int *)0x560000B0)
#define BUSY            1

#define NAND_SECTOR_SIZE	512
#define NAND_BLOCK_MASK	(NAND_SECTOR_SIZE - 1)

#define NAND_SECTOR_SIZE_LP	2048
#define NAND_BLOCK_MASK_LP	(NAND_SECTOR_SIZE_LP - 1)

char bLARGEBLOCK;			//HJ_add 20090807
char b128MB;				//HJ_add 20090807

/* 供外部调用的函数 */
void nand_init_ll(void);
int nand_read_ll(unsigned char *buf, unsigned long start_addr, int size);
int nand_read_ll_lp(unsigned char *buf, unsigned long start_addr, int size);

/* NAND Flash操作的总入口, 它们将调用S3C2410或S3C2440的相应函数 */
static void nand_reset(void);
static void wait_idle(void);
static void nand_select_chip(void);
static void nand_deselect_chip(void);
static void write_cmd(int cmd);
static void write_addr(unsigned int addr);
static void write_addr_lp(unsigned int addr);
static unsigned char read_data(void);
int NF_ReadID(void);				//HJ_add 20090807

/* S3C2410的NAND Flash处理函数 */
static void s3c2410_nand_reset(void);
static void s3c2410_wait_idle(void);
static void s3c2410_nand_select_chip(void);
static void s3c2410_nand_deselect_chip(void);
static void s3c2410_write_cmd(int cmd);
static void s3c2410_write_addr(unsigned int addr);
static unsigned char s3c2410_read_data(void);

/* S3C2440的NAND Flash处理函数 */
static void s3c2440_nand_reset(void);
static void s3c2440_wait_idle(void);
static void s3c2440_nand_select_chip(void);
static void s3c2440_nand_deselect_chip(void);
static void s3c2440_write_cmd(int cmd);
static void s3c2440_write_addr(unsigned int addr);
static void s3c2440_write_addr_lp(unsigned int addr);
static unsigned char s3c2440_read_data(void);

/* S3C2410的NAND Flash操作函数 */

/* 复位 */
static void s3c2410_nand_reset(void)
{
	s3c2410_nand_select_chip();
	s3c2410_write_cmd(0xff);  // 复位命令
	s3c2410_wait_idle();
	s3c2410_nand_deselect_chip();
}

/* 等待NAND Flash就绪 */
static void s3c2410_wait_idle(void)
{
	int i;
	S3C2410_NAND * s3c2410nand = (S3C2410_NAND *)0x4e000000;
	
	volatile unsigned char *p = (volatile unsigned char *)&s3c2410nand->NFSTAT;
	while(!(*p & BUSY))
        for(i=0; i<10; i++);
}

/* 发出片选信号 */
static void s3c2410_nand_select_chip(void)
{
	int i;
	S3C2410_NAND * s3c2410nand = (S3C2410_NAND *)0x4e000000;

	s3c2410nand->NFCONF &= ~(1<<11);
	for(i=0; i<10; i++);    
}

/* 取消片选信号 */
static void s3c2410_nand_deselect_chip(void)
{
	S3C2410_NAND * s3c2410nand = (S3C2410_NAND *)0x4e000000;

	s3c2410nand->NFCONF |= (1<<11);
}

/* 发出命令 */
static void s3c2410_write_cmd(int cmd)
{
	S3C2410_NAND * s3c2410nand = (S3C2410_NAND *)0x4e000000;

	volatile unsigned char *p = (volatile unsigned char *)&s3c2410nand->NFCMD;
	*p = cmd;
}

/* 发出地址 */
static void s3c2410_write_addr(unsigned int addr)
{
	int i;
	S3C2410_NAND * s3c2410nand = (S3C2410_NAND *)0x4e000000;
	volatile unsigned char *p = (volatile unsigned char *)&s3c2410nand->NFADDR;
    
	*p = addr & 0xff;
	for(i=0; i<10; i++);
	*p = (addr >> 9) & 0xff;
	for(i=0; i<10; i++);
	*p = (addr >> 17) & 0xff;
	for(i=0; i<10; i++);
	*p = (addr >> 25) & 0xff;
	for(i=0; i<10; i++);
}

/* 读取数据 */
static unsigned char s3c2410_read_data(void)
{
	S3C2410_NAND * s3c2410nand = (S3C2410_NAND *)0x4e000000;

	volatile unsigned char *p = (volatile unsigned char *)&s3c2410nand->NFDATA;
	return *p;
}

/* S3C2440的NAND Flash操作函数 */

/* 复位 */
static void s3c2440_nand_reset(void)
{
	s3c2440_nand_select_chip();
	s3c2440_write_cmd(0xff);  // 复位命令
	s3c2440_wait_idle();
	s3c2440_nand_deselect_chip();
}

/* 等待NAND Flash就绪 */
static void s3c2440_wait_idle(void)
{
	int i;
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;
	volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFSTAT;

	while(!(*p & BUSY))
        for(i=0; i<10; i++);
}

/* 发出片选信号 */
static void s3c2440_nand_select_chip(void)
{
	int i;
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;

	s3c2440nand->NFCONT &= ~(1<<1);
	for(i=0; i<10; i++);    
}

/* 取消片选信号 */
static void s3c2440_nand_deselect_chip(void)
{
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;

	s3c2440nand->NFCONT |= (1<<1);
}

/* 发出命令 */
static void s3c2440_write_cmd(int cmd)
{
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;

	volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFCMD;
	*p = cmd;
}

/* 发出地址 */
static void s3c2440_write_addr(unsigned int addr)
{
	int i;
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;
	volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFADDR;
    
	*p = addr & 0xff;
	for(i=0; i<10; i++);
	*p = (addr >> 9) & 0xff;
	for(i=0; i<10; i++);
	*p = (addr >> 17) & 0xff;
	for(i=0; i<10; i++);
	*p = (addr >> 25) & 0xff;
	for(i=0; i<10; i++);
}


/* 发出地址 */
static void s3c2440_write_addr_lp(unsigned int addr)
{
	int i;
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;
	volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFADDR;
	int col, page;

	col = addr & NAND_BLOCK_MASK_LP;
	page = addr / NAND_SECTOR_SIZE_LP;
	
	*p = col & 0xff;			/* Column Address A0~A7 */
	for(i=0; i<10; i++);		
	*p = (col >> 8) & 0x0f;		/* Column Address A8~A11 */
	for(i=0; i<10; i++);
	*p = page & 0xff;			/* Row Address A12~A19 */
	for(i=0; i<10; i++);
	*p = (page >> 8) & 0xff;	/* Row Address A20~A27 */
	for(i=0; i<10; i++);
if (b128MB == 0)
	*p = (page >> 16) & 0x03;	/* Row Address A28~A29 */
	for(i=0; i<10; i++);
}

/* 读取数据 */
static unsigned char s3c2440_read_data(void)
{
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;
	volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFDATA;
	return *p;
}


/* 在第一次使用NAND Flash前，复位一下NAND Flash */
static void nand_reset(void)
{
	s3c2440_nand_reset();
}

static void wait_idle(void)
{
	s3c2440_wait_idle();
}

static void nand_select_chip(void)
{
	int i;
	
	s3c2440_nand_select_chip();
	
	for(i=0; i<10; i++);
}

static void nand_deselect_chip(void)
{
	s3c2440_nand_deselect_chip();
}

static void write_cmd(int cmd)
{
	s3c2440_write_cmd(cmd);
}
static void write_addr(unsigned int addr)
{
	s3c2440_write_addr(addr);
}

static void write_addr_lp(unsigned int addr)
{
	s3c2440_write_addr_lp(addr);
}

static unsigned char read_data(void)
{
	return s3c2440_read_data();
}

/* 初始化NAND Flash */
void nand_init_ll(void)
{
	S3C2410_NAND * s3c2410nand = (S3C2410_NAND *)0x4e000000;
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;

	#define TACLS   0
	#define TWRPH0  3
	#define TWRPH1  0

	/* 设置时序 */
	s3c2440nand->NFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4);
	/* 使能NAND Flash控制器, 初始化ECC, 禁止片选 */
	s3c2440nand->NFCONT = (1<<4)|(1<<1)|(1<<0);

	/* 复位NAND Flash */
	nand_reset();
}
#if 1
int NF_ReadID(void)
{
	char pMID;
	char pDID;
	int  nCnt;
	int  nBAddr, nRet;
	int  nBuff;
	char   n4thcycle;
	int i;
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;
	volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFADDR;

	b128MB = 1;
	n4thcycle = nBuff = 0;

	nand_init_ll();
	nand_select_chip();
	s3c2440_write_cmd(0x90);	// read id command
	*p=0x00 & 0xff;
	for ( i = 0; i < 100; i++ );

	pMID = s3c2440_read_data();
	pDID =  s3c2440_read_data();
	nBuff =  s3c2440_read_data();
	n4thcycle = s3c2440_read_data();

	s3c2440_nand_deselect_chip();
	
	if (pDID >= 0xA0)
	{
		b128MB = 0;
	}

	return (pDID);
}
#endif

/* 读函数 */
int nand_read_ll(unsigned char *buf, unsigned long start_addr, int size)
{
	int i, j;
	char dat;
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;
	volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFADDR;

    
	if ((start_addr & NAND_BLOCK_MASK) || (size & NAND_BLOCK_MASK))
	{
		return ;    /* 地址或长度不对齐 */
	}

	/* 选中芯片 */
	nand_select_chip();

	for(i=start_addr; i < (start_addr + size);)
	{
/* Check Bad Block */
if(0){
		/* 发出READ0命令 */
		s3c2440_write_cmd(0x50);

		*p = 5;
		for(j=0; j<10; j++);
		*p = (i >> 9) & 0xff;
		for(j=0; j<10; j++);
		*p = (i >> 17) & 0xff;
		for(j=0; j<10; j++);
		*p = (i >> 25) & 0xff;
		for(j=0; j<10; j++);
		s3c2440_wait_idle();
		dat = s3c2440_read_data();
		s3c2440_write_cmd(0);
		
		/* 取消片选信号 */
		nand_deselect_chip();
		if(dat != 0xff)
			i += 16384;		// 1 Block = 512*32= 16384
/* Read Page */
		/* 选中芯片 */
		nand_select_chip();
}
		/* 发出READ0命令 */
		s3c2440_write_cmd(0);

		/* Write Address */
		s3c2440_write_addr(i);
		s3c2440_wait_idle();

		for(j=0; j < NAND_SECTOR_SIZE; j++, i++)
		{
			*buf = s3c2440_read_data();
			buf++;
		}
	}

	/* 取消片选信号 */
	nand_deselect_chip();

	return 0;
}

/* 读函数 
  * Large Page
  */
int nand_read_ll_lp(unsigned char *buf, unsigned long start_addr, int size)
{
	int i, j;
	char dat;
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;
	volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFADDR;

	if ((start_addr & NAND_BLOCK_MASK_LP) || (size & NAND_BLOCK_MASK_LP))
	{
		return ;    /* 地址或长度不对齐 */
	}

	/* 选中芯片 */
	nand_select_chip();

	for(i=start_addr; i < (start_addr + size);)
	{
/* Check Bad Block */
if(0){
		int col, page;

		col = i & NAND_BLOCK_MASK_LP;
		page = i / NAND_SECTOR_SIZE_LP;
		/* 发出READ0命令 */
		s3c2440_write_cmd(0x00);

		*p = 5;
		for(j=0; j<10; j++);
		*p = 8;
		for(j=0; j<10; j++);
		*p = page & 0xff;		/* Row Address A12~A19 */
		for(j=0; j<10; j++);
		*p = (page >> 8) & 0xff;		/* Row Address A20~A27 */
		for(j=0; j<10; j++);
if (b128MB == 0)
		*p = (page >> 16) & 0x03;		/* Row Address A28~A29 */
		for(j=0; j<10; j++);

		s3c2440_write_cmd(0x30);
		s3c2440_wait_idle();
		dat = s3c2440_read_data();
		
		/* 取消片选信号 */
		nand_deselect_chip();
		if(dat != 0xff)
			i += 131072;		// 1 Block = 2048*64= 131072
/* Read Page */
		/* 选中芯片 */
		nand_select_chip();
}
		/* 发出READ0命令 */
		write_cmd(0);

		/* Write Address */
		write_addr_lp(i);
		write_cmd(0x30);
		wait_idle();

		for(j=0; j < NAND_SECTOR_SIZE_LP; j++, i++)
		{
			*buf = read_data();
			buf++;
		}
	}

	/* 取消片选信号 */
	nand_deselect_chip();

	return 0;
}

int bBootFrmNORFlash(void)
{
	volatile unsigned int *pdw = (volatile unsigned int *)0;
	unsigned int dwVal;

	/*
	 * 无论是从NOR Flash还是从NAND Flash启动，
	 * 地址0处为指令"b	Reset", 机器码为0xEA00000B，
	 * 对于从NAND Flash启动的情况，其开始4KB的代码会复制到CPU内部4K内存中，
	 * 对于从NOR Flash启动的情况，NOR Flash的开始地址即为0。
	 * 对于NOR Flash，必须通过一定的命令序列才能写数据，
	 * 所以可以根据这点差别来分辨是从NAND Flash还是NOR Flash启动:
	 * 向地址0写入一个数据，然后读出来，如果没有改变的话就是NOR Flash
	 */

	dwVal = *pdw;       
	*pdw = 0x12345678;
	if (*pdw != 0x12345678)
	{
		return 1;
	}
	else
	{
		*pdw = dwVal;
		return 0;
	}
}

int CopyCode2Ram(unsigned long start_addr, unsigned char *buf, int size)
{
	unsigned int *pdwDest;
	unsigned int *pdwSrc;
	int i;

	if (bBootFrmNORFlash())
	{
		pdwDest = (unsigned int *)buf;
		pdwSrc  = (unsigned int *)start_addr;
		/* 从 NOR Flash启动 */
		for (i = 0; i < size / 4; i++)
		{
			pdwDest[i] = pdwSrc[i];
		}
		return 0;
	}
	else
	{
		/* 初始化NAND Flash */
		nand_init_ll();

		/* 从 NAND Flash启动 */
		if (NF_ReadID() == 0x76 )
			nand_read_ll(buf, start_addr, (size + NAND_BLOCK_MASK)&~(NAND_BLOCK_MASK));
		else
			nand_read_ll_lp(buf, start_addr, (size + NAND_BLOCK_MASK_LP)&~(NAND_BLOCK_MASK_LP));
		return 0;
	}
}

static inline void delay (unsigned long loops)
{
    __asm__ volatile ("1:\n"
      "subs %0, %1, #1\n"
      "bne 1b":"=r" (loops):"0" (loops));
}

/* S3C2440: Mpll = (2*m * Fin) / (p * 2^s), UPLL = (m * Fin) / (p * 2^s)
 * m = M (the value for divider M)+ 8, p = P (the value for divider P) + 2
 */
/* Fin = 12.0000MHz */
#define S3C2440_MPLL_400MHZ	((0x5c<<12)|(0x01<<4)|(0x01))						//HJ 400MHz
#define S3C2440_MPLL_405MHZ	((0x7f<<12)|(0x02<<4)|(0x01))						//HJ 405MHz
#define S3C2440_MPLL_440MHZ	((0x66<<12)|(0x01<<4)|(0x01))						//HJ 440MHz
#define S3C2440_MPLL_480MHZ	((0x98<<12)|(0x02<<4)|(0x01))						//HJ 480MHz
#define S3C2440_MPLL_200MHZ	((0x5c<<12)|(0x01<<4)|(0x02))
#define S3C2440_MPLL_100MHZ	((0x5c<<12)|(0x01<<4)|(0x03))

#define S3C2440_UPLL_48MHZ	((0x38<<12)|(0x02<<4)|(0x02))						//HJ 100MHz

#define S3C2440_CLKDIV		0x05    /* FCLK:HCLK:PCLK = 1:4:8, UCLK = UPLL */		//HJ 100MHz
#define S3C2440_CLKDIV136	0x07    /* FCLK:HCLK:PCLK = 1:3:6, UCLK = UPLL */		//HJ 133MHz
#define S3C2440_CLKDIV188	0x04    /* FCLK:HCLK:PCLK = 1:8:8 */
#define S3C2440_CAMDIVN188	((0<<8)|(1<<9)) /* FCLK:HCLK:PCLK = 1:8:8 */

/* Fin = 16.9344MHz */
#define S3C2440_MPLL_399MHz     		((0x6e<<12)|(0x03<<4)|(0x01))
#define S3C2440_UPLL_48MHZ_Fin16MHz		((60<<12)|(4<<4)|(2))


/* S3C2410: Mpll,Upll = (m * Fin) / (p * 2^s)
 * m = M (the value for divider M)+ 8, p = P (the value for divider P) + 2
 */
#define S3C2410_MPLL_200MHZ     ((0x5c<<12)|(0x04<<4)|(0x00))
#define S3C2410_UPLL_48MHZ      ((0x28<<12)|(0x01<<4)|(0x02))
#define S3C2410_CLKDIV          0x03    /* FCLK:HCLK:PCLK = 1:2:4 */
void clock_init(void)
{
	S3C24X0_CLOCK_POWER *clk_power = (S3C24X0_CLOCK_POWER *)0x4C000000;

	/* support both of S3C2410 and S3C2440, by www.embedsky.net */
	if ((GSTATUS1 == 0x32410000) || (GSTATUS1 == 0x32410002))
	{
		/* FCLK:HCLK:PCLK = 1:2:4 */
		clk_power->CLKDIVN = S3C2410_CLKDIV;

		/* change to asynchronous bus mod */
		__asm__(    "mrc    p15, 0, r1, c1, c0, 0\n"    /* read ctrl register   */  
			     "orr    r1, r1, #0xc0000000\n"      /* Asynchronous         */  
			     "mcr    p15, 0, r1, c1, c0, 0\n"    /* write ctrl register  */  
			    :::"r1"
			);
        
		/* to reduce PLL lock time, adjust the LOCKTIME register */
		clk_power->LOCKTIME = 0xFFFFFF;

		/* configure MPLL */
		clk_power->MPLLCON = S3C2410_MPLL_200MHZ;

		/* some delay between MPLL and UPLL */
		delay (4000);

		/* configure UPLL */
		clk_power->UPLLCON = S3C2410_UPLL_48MHZ;

		/* some delay between MPLL and UPLL */
		delay (8000);
	}
	else
	{
		/* FCLK:HCLK:PCLK = ?:?:? */
#if CONFIG_133MHZ_SDRAM
		clk_power->CLKDIVN = S3C2440_CLKDIV136;			//HJ 1:3:6
#else
		clk_power->CLKDIVN = S3C2440_CLKDIV;				//HJ 1:4:8
#endif
		/* change to asynchronous bus mod */
		__asm__(    "mrc    p15, 0, r1, c1, c0, 0\n"    /* read ctrl register   */  
        	            "orr    r1, r1, #0xc0000000\n"      /* Asynchronous         */  
         	           "mcr    p15, 0, r1, c1, c0, 0\n"    /* write ctrl register  */  
        	            :::"r1"
        	            );

		/* to reduce PLL lock time, adjust the LOCKTIME register */
		clk_power->LOCKTIME = 0xFFFFFF;

		/* configure UPLL */
		clk_power->UPLLCON = S3C2440_UPLL_48MHZ;		//fin=12.000MHz
//		clk_power->UPLLCON = S3C2440_UPLL_48MHZ_Fin16MHz;	//fin=16.934MHz

		/* some delay between MPLL and UPLL */
		delay (4000);

		/* configure MPLL */
		clk_power->MPLLCON = S3C2440_MPLL_400MHZ;		//fin=12.000MHz
//		clk_power->MPLLCON = S3C2440_MPLL_405MHZ;				//HJ 405MHz
//		clk_power->MPLLCON = S3C2440_MPLL_440MHZ;				//HJ 440MHz
//		clk_power->MPLLCON = S3C2440_MPLL_480MHZ;				//HJ 480MHz
//		clk_power->MPLLCON = S3C2440_MPLL_399MHz;		//fin=16.934MHz

		/* some delay between MPLL and UPLL */
		delay (8000);
	}
}

