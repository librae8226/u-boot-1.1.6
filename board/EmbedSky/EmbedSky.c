/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <s3c2410.h>

DECLARE_GLOBAL_DATA_PTR;


/*
 * Miscellaneous platform dependent initialisations
 */
int board_init (void)
{
	S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	/* set up the I/O ports */
	gpio->GPACON = 0x007FFFFF;
	gpio->GPBCON = 0x00055555;
	gpio->GPBUP = 0x000007FF;
	gpio->GPCCON = 0xAAAAAAAA;
	gpio->GPCUP = 0x0000FFFF;
	gpio->GPDCON = 0xAAAAAAAA;
	gpio->GPDUP = 0x0000FFFF;
	gpio->GPECON = 0xAAAAAAAA;
	gpio->GPEUP = 0x0000FFFF;
	gpio->GPFCON = 0x000055AA;
	gpio->GPFUP = 0x000000FF;
	gpio->GPGCON = 0xFF94FFBA;
	gpio->GPGUP = 0x0000FFEF;
	gpio->GPGDAT = gpio->GPGDAT & (~(1<<4)) | (1<<4) ;
	gpio->GPHCON = 0x002AFAAA;
	gpio->GPHUP = 0x000007FF;
	gpio->GPJCON = 0x02aaaaaa;
	gpio->GPJUP = 0x00001fff;

//	S3C24X0_I2S * const i2s = S3C24X0_GetBase_I2S();	//HJ_add 屏蔽IIS,
//	i2s->IISCON = 0x00;					//HJ_add 屏蔽IIS,

	/* support both of S3C2410 and S3C2440, by www.embedsky.net */
	if ((gpio->GSTATUS1 == 0x32410000) || (gpio->GSTATUS1 == 0x32410002))
	{
		/* arch number of SMDK2410-Board */
		gd->bd->bi_arch_number = MACH_TYPE_SMDK2410;
	}
	else
	{
		/* arch number of SMDK2440-Board */
		gd->bd->bi_arch_number = MACH_TYPE_S3C2440;
	}

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x30000100;

	icache_enable();
	dcache_enable();

	return 0;
}

int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

#ifdef CONFIG_SURPORT_WINCE

extern S3C24X0_GPIO * gpioregs;

//***************************[ PORTS ]****************************************************
void Wince_Port_Init(void)
{
    //CAUTION:Follow the configuration order for setting the ports. 
    // 1) setting value(GPnDAT) 
    // 2) setting control register  (GPnCON)
    // 3) configure pull-up resistor(GPnUP)  

    //32bit data bus configuration  
    //*** PORT A GROUP
    //Ports  : GPA22 GPA21  GPA20 GPA19 GPA18 GPA17 GPA16 GPA15 GPA14 GPA13 GPA12  
    //Signal : nFCE nRSTOUT nFRE   nFWE  ALE   CLE  nGCS5 nGCS4 nGCS3 nGCS2 nGCS1 
    //Binary :  1     1      1  , 1   1   1    1   ,  1     1     1     1
    //Ports  : GPA11   GPA10  GPA9   GPA8   GPA7   GPA6   GPA5   GPA4   GPA3   GPA2   GPA1  GPA0
    //Signal : ADDR26 ADDR25 ADDR24 ADDR23 ADDR22 ADDR21 ADDR20 ADDR19 ADDR18 ADDR17 ADDR16 ADDR0 
    //Binary :  1       1      1      1   , 1       1      1      1   ,  1       1     1      1         
    gpioregs->GPACON = 0x7fffff; 

    //**** PORT B GROUP
    //Ports  : GPB10    GPB9    GPB8    GPB7    GPB6     GPB5    GPB4   GPB3   GPB2     GPB1      GPB0
    //Signal : nXDREQ0 nXDACK0 nXDREQ1 nXDACK1 nSS_KBD nDIS_OFF L3CLOCK L3DATA L3MODE nIrDATXDEN Keyboard
    //Setting: INPUT  OUTPUT   INPUT  OUTPUT   INPUT   OUTPUT   OUTPUT OUTPUT OUTPUT   OUTPUT    OUTPUT 
    //Binary :   00  ,  01       00  ,   01      00   ,  01       01  ,   01     01   ,  01        01  
    gpioregs->GPBCON = 0x044555;
    gpioregs->GPBUP  = 0x7ff;     // The pull up function is disabled GPB[10:0]

    //*** PORT C GROUP
    //Ports  : GPC15 GPC14 GPC13 GPC12 GPC11 GPC10 GPC9 GPC8  GPC7   GPC6   GPC5 GPC4 GPC3  GPC2  GPC1 GPC0
    //Signal : VD7   VD6   VD5   VD4   VD3   VD2   VD1  VD0 LCDVF2 LCDVF1 LCDVF0 VM VFRAME VLINE VCLK LEND  
    //Binary :  10   10  , 10    10  , 10    10  , 10   10  , 10     10  ,  10   10 , 10     10 , 10   10
    gpioregs->GPCCON = 0xaaaaaaaa;       
    gpioregs->GPCUP  = 0xffff;     // The pull up function is disabled GPC[15:0] 

    //*** PORT D GROUP
    //Ports  : GPD15 GPD14 GPD13 GPD12 GPD11 GPD10 GPD9 GPD8 GPD7 GPD6 GPD5 GPD4 GPD3 GPD2 GPD1 GPD0
    //Signal : VD23  VD22  VD21  VD20  VD19  VD18  VD17 VD16 VD15 VD14 VD13 VD12 VD11 VD10 VD9  VD8
    //Binary : 10    10  , 10    10  , 10    10  , 10   10 , 10   10 , 10   10 , 10   10 ,10   10
    //rGPDCON  = 0xaaa9aaaa;   // GPD8: USB_CHG_DIS: Output
    //rGPDUP   = 0xffff;       // The pull up function is disabled GPD[15:0]
    //rGPDDAT &= ~(0x1 << 8);  // Set USB_CHG_DIS Low

    //Ports  : GPD15        GPD14   GPD13   GPD12   GPD11   GPD10   GPD9    GPD8         
    //Signal : USB_PULLUP   NC      NC      NC      NC      nFWP    SELI    USB_CHG_DIS  
    //CON    : 00           00      00      00      00      01      01      01
    //DAT    : 0            0       0       0       0       1       0(100ma)0 (low)
    //UP     : 1            0       0       0       0       1       1       1

    //Ports  : GPD7         GPD6    GPD5    GPD4    GPD3    GPD2    GPD1    GPD0
    //Signal : NC           EL_EN   KEEPACT IR_SD   PWREN2  NC      TXON    NC
    //CON    : 00           01      01      01      01      00      01      00
    //DAT    : 0            0(on)   1(high) 1(on)   0(low)  0       0(low)  0
    //UP     : 0            1       1       1       1       0       1       0
    gpioregs->GPDCON  = 0x00151544;
    gpioregs->GPDDAT  = 0x0430;
    gpioregs->GPDUP   = 0x877A;

    //*** PORT E GROUP
    //Ports  : GPE15  GPE14 GPE13   GPE12   GPE11   GPE10   GPE9    GPE8     GPE7  GPE6  GPE5   GPE4  
    //Signal : IICSDA IICSCL SPICLK SPIMOSI SPIMISO SDDATA3 SDDATA2 SDDATA1 SDDATA0 SDCMD SDCLK I2SSDO 
    //Binary :  10     10  ,  10      10  ,  00      10   ,  10      10   ,   10    10  , 10     10  ,     
    //-------------------------------------------------------------------------------------------------------
    //Ports  :  GPE3   GPE2  GPE1    GPE0    
    //Signal : I2SSDI CDCLK I2SSCLK I2SLRCK     
    //Binary :  10     10  ,  10      10 
    //rGPECON = 0xaaaaaaaa;       
    //rGPEUP  = 0xffff;     // The pull up function is disabled GPE[15:0]
    gpioregs->GPECON = 0xaa2aaaaa;       
    gpioregs->GPEUP  = 0xf7ff;       // GPE11 is NC

    //*** PORT F GROUP
    //Ports  : GPF7   GPF6   GPF5   GPF4      GPF3     GPF2  GPF1   GPF0
    //Signal : nLED_8 nLED_4 nLED_2 nLED_1 nIRQ_PCMCIA EINT2 KBDINT EINT0
    //Setting: Output Output Output Output    EINT3    EINT2 EINT1  EINT0
    //Binary :  01      01 ,  01     01  ,     10       10  , 10     10
    gpioregs->GPFCON = 0x55aa;
    gpioregs->GPFUP  = 0xff;     // The pull up function is disabled GPF[7:0]

    //*** PORT G GROUP
    //Ports  : GPG15 GPG14 GPG13 GPG12  GPG11    GPG10    GPG9     GPG8     GPG7      GPG6    
    //Signal : nYPON  YMON nXPON XMON   NC       SDWP     WHEEL_SW WHEEL_A  WHEEL_B   ONE_WIRE
    //Setting: nYPON  YMON nXPON XMON   IN       EINT18   EINT17   ENT16    EINT15    OUTPUT
    //Binary :   11    11 , 11    11  , 00       01    ,  10       10   ,   10        01
    //UP     :   1     1    1     1     0        1        1        1        1         1
    //-----------------------------------------------------------------------------------------
    //Ports  : GPG5       GPG4      GPG3    GPG2    GPG1     GPG0    
    //Signal : NC         LCD_PWREN NC      nSS_SPI JACK_CLK JACK_DATA
    //Setting: IN         LCD_PWRDN IN      nSS0    EINT9    EINT8
    //Binary : 00         11   ,    00      11  ,   10       10
    //UP     : 0          1         0       1       1        1
// Behavior changed from board rev A to rev D. Don't power LCD down.
    
    //*** PORT H GROUP
    //Ports  :  GPH10    GPH9  GPH8 GPH7  GPH6  GPH5 GPH4 GPH3 GPH2 GPH1  GPH0 
    //Signal : CLKOUT1 CLKOUT0 UCLK nCTS1 nRTS1 RXD1 TXD1 RXD0 TXD0 nRTS0 nCTS0
    //Binary :   01   ,  01     10 , 11    11  , 10   10 , 10   10 , 10    10    
    gpioregs->GPHCON = 0x16faaa;
    gpioregs->GPHUP  = 0x7ff;    // The pull up function is disabled GPH[10:0]

    //External interrupt will be falling edge triggered. 
    gpioregs->EXTINT0 = 0x22222222;    // EINT[7:0]
    gpioregs->EXTINT1 = 0x22222222;    // EINT[15:8]
    gpioregs->EXTINT2 = 0x22222222;    // EINT[23:16]
}
#endif /* CONFIG_SURPORT_WINCE */
    
