#include "..\App\includes.h"

#define TP_CS()	GPIO_ResetBits(GPIOB,GPIO_Pin_7)
#define TP_DCS() GPIO_SetBits(GPIOB,GPIO_Pin_7)

void tp_Config(void) ;

extern void I2C_FM_Init(void);

extern void USART1_InitConfig(int BaudRate);
extern void USART2_InitConfig(int BaudRate);
extern void USART3_InitConfig(int BaudRate);
extern void FSMC_LCD_Init(void);

void SonyRM_InitConfig(void);

void RCC_Configuration(void)
{
	ErrorStatus HSEStartUpStatus;	 
	RCC_DeInit();
	RCC_HSEConfig(RCC_HSE_ON);
	HSEStartUpStatus = RCC_WaitForHSEStartUp();

	if(HSEStartUpStatus == SUCCESS)
	{
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		FLASH_SetLatency(FLASH_Latency_2);
		RCC_HCLKConfig(RCC_SYSCLK_Div1); 
		RCC_PCLK2Config(RCC_HCLK_Div1); 
		RCC_PCLK1Config(RCC_HCLK_Div2);
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
		RCC_PLLCmd(ENABLE);
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{
		}

		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		while(RCC_GetSYSCLKSource() != 0x08)
		{
		}
	}

	/* Enable GPIO_LED clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); //RipZ
}

void UnableJTAG(void)
{
	 
	 RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;										 // enable clock for Alternate Function
	 AFIO->MAPR &= ~(7UL<<24); // clear used bit
	 AFIO->MAPR |= (4UL<<24); // set used bits
}

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
												 RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE , ENABLE); 	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_13;		 //LED4, LED3
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;		 //LED1, LED2
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 ; 	 //LCD-RST
	GPIO_Init(GPIOE, &GPIO_InitStructure);		
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; // SONY RM-X6S INPUT
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // IPU
	GPIO_Init(GPIOE, &GPIO_InitStructure);		
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; // SONY RM-X6S mirror test output
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);		
	
	// Initialization Bluetooth	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_11 | GPIO_Pin_12;
//	GPIO_Init(GPIOA, &GPIO_InitStructure); // TODO: ???		

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
//	GPIO_Init(GPIOC, &GPIO_InitStructure); // TODO: ??? 	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; // BT_POWER_KEY
	GPIO_Init(GPIOA, &GPIO_InitStructure);		

// LCD interface

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
																GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 | 
																GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | 
																GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | 
																GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure); 
	
	/* NE1 configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; 
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	/* RS */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 ; 
	GPIO_Init(GPIOD, &GPIO_InitStructure); 
	
	GPIO_SetBits(GPIOD, GPIO_Pin_7);			//CS=1 
	GPIO_SetBits(GPIOD, GPIO_Pin_14| GPIO_Pin_15 |GPIO_Pin_0 | GPIO_Pin_1);		 
	GPIO_SetBits(GPIOE, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10);	 
	GPIO_ResetBits(GPIOE, GPIO_Pin_0);
	GPIO_ResetBits(GPIOE, GPIO_Pin_1);			//RESET=0
	GPIO_SetBits(GPIOD, GPIO_Pin_4);				//RD=1
	GPIO_SetBits(GPIOD, GPIO_Pin_5);			//WR=1		

	//usart1_gpio_init----------------------------------------------------
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1 , ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//usart2_gpio_init----------------------------------------------------
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2 , ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//usart3_gpio_init----------------------------------------------------
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART3 , ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// I2C bus initialization (SCL - PB.8, SDA - PB.9)
	GPIO_InitStructure.GPIO_Pin =	GPIO_Pin_8 | GPIO_Pin_9;				
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;																							 
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void NVIC_Configuration(void)
{
#if defined (VECT_TAB_RAM)
	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#elif defined(VECT_TAB_FLASH_IAP)
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x2000);
#else
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);	 
#endif 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
}

void RCC1_Configuration(void)
{	
	ErrorStatus HSEStartUpStatus; 
	RCC_DeInit();
	RCC_HSEConfig(RCC_HSE_ON);
	HSEStartUpStatus = RCC_WaitForHSEStartUp();

	if(HSEStartUpStatus == SUCCESS)
	{
		RCC_HCLKConfig(RCC_SYSCLK_Div1); 
		RCC_PCLK2Config(RCC_HCLK_Div1); 
		RCC_PCLK1Config(RCC_HCLK_Div2);
		FLASH_SetLatency(FLASH_Latency_2);
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
		RCC_PLLCmd(ENABLE);
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{
		}

		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		while(RCC_GetSYSCLKSource() != 0x08)
		{
		}
	}

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA |
												 RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOD, ENABLE);
}

void BSP_Init(void)
{
	RCC1_Configuration();	 
	GPIO_Configuration();
	NVIC_Configuration();

	USART1_InitConfig(9600); // Connection to Android CarPC
	USART2_InitConfig(1904); // Car ECU connection 1904
//	USART3_InitConfig(4800); // not used at this time

	SonyRM_InitConfig();
	
	tp_Config();
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
 
	FSMC_LCD_Init();
}

void SonyRM_InitConfig(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource0);
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

CPU_INT32U	BSP_CPU_ClkFreq (void)
{
	RCC_ClocksTypeDef	rcc_clocks;
	RCC_GetClocksFreq(&rcc_clocks);
	return ((CPU_INT32U)rcc_clocks.HCLK_Frequency);
}

INT32U	OS_CPU_SysTickClkFreq (void)
{
	INT32U	freq;
	freq = BSP_CPU_ClkFreq();
	return (freq);
}

void	OS_CPU_SysTickInit(void)
{
	RCC_ClocksTypeDef	rcc_clocks;
	INT32U				 cnts;
	RCC_GetClocksFreq(&rcc_clocks);
	cnts = (INT32U)rcc_clocks.HCLK_Frequency/OS_TICKS_PER_SEC;
	SysTick_Config(cnts);
}

void tp_Config(void) 
{ 
	GPIO_InitTypeDef	GPIO_InitStructure; 
	SPI_InitTypeDef	 SPI_InitStructure; 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;					 //SPI1 CS1 
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;					 //SPI1 CS4 
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;					 //SPI1 NSS 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_4);			//SPI CS1
	GPIO_SetBits(GPIOB, GPIO_Pin_12);			//SPI CS4
	GPIO_SetBits(GPIOA, GPIO_Pin_4);			//SPI NSS
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; 
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master; 
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; 
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; 
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; 
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;	 //SPI_NSS_Hard
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64; 
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; 
	SPI_InitStructure.SPI_CRCPolynomial = 7; 
	SPI_Init(SPI1,&SPI_InitStructure); 
	SPI_Cmd(SPI1,ENABLE);	
}


unsigned char SPI_WriteByte(unsigned char data) 
{ 
 unsigned char Data = 0; 
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)==RESET); 
	SPI_I2S_SendData(SPI1,data); 
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)==RESET); 
	Data = SPI_I2S_ReceiveData(SPI1); 
	return Data; 
}	

void SpiDelay(unsigned int DelayCnt)
{
 unsigned int i;
 for(i=0;i<DelayCnt;i++);
}

u16 TPReadX(void)
{ 
	 u16 x=0;
	 TP_CS();
	 SpiDelay(10);
	 SPI_WriteByte(0x90);
	 SpiDelay(10);
	 x=SPI_WriteByte(0x00);
	 x<<=8;
	 x+=SPI_WriteByte(0x00);
	 SpiDelay(10);
	 TP_DCS(); 
	 x = x>>3;
	 return (x);
}

u16 TPReadY(void)
{
 u16 y=0;
	TP_CS();
	SpiDelay(10);
	SPI_WriteByte(0xD0);
	SpiDelay(10);
	y=SPI_WriteByte(0x00);
	y<<=8;
	y+=SPI_WriteByte(0x00);
	SpiDelay(10);
	TP_DCS();
	y = y>>3; 
	return (y);
}

#ifdef	DEBUG
void assert_failed(u8* file, u32 line)
{ 
	while (1)
	{
	}
}
#endif
