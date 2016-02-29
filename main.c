/**
  ******************************************************************************
  * @file    USART/USART_Printf/main.c 
  * @author  MCD Application Team
  * @version V1.6.0
  * @date    04-September-2015
  * @brief   Main program body
  ******************************************************************************
 
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "my_usart.h"
#include "misc.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
 NVIC_InitTypeDef NVIC_Struct;
 	GPIO_InitTypeDef GPIO_StructForSPI1;
	SPI_InitTypeDef SPI1_Struct;
 #define LED_1 "LED_1_ON"
 #define LED_2 "LED_2_ON"
 #define LED_3 "LED_3_ON"
 #define LED_4 "LED_4_ON"
 #define GAIN 16384
 #define pre 32
 float x,y,z;
 
void SPI1_InitFunction(void);
 void SPI1_Write_Data(uint8_t address,uint8_t data);
 uint8_t SPI1_Read_Data(uint8_t address);
volatile char *received_string; // this will hold the recieved string
void NVIC_Config(void);
/* Private functions ---------------------------------------------------------*/
void Delay(void) {
    uint32_t i;
    for(i=0; i<0xffffff/10; ++i) {
   
    }
}
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	uint8_t reg;
  received_string = malloc(sizeof(char));
 GPIO_Config();
  USART_Config();
  NVIC_Config();
  SPI1_InitFunction();
	//GPIO_ResetBits(GPIOE, GPIO_Pin_3); 
 
	SPI1_Write_Data(0x23, 0xc9); 
 SPI1_Write_Data(0x20, 0x67);	// resetting the accelerometer internal circuit    ``                    // 100Hz data update rate, block data update disable, x/y/z enabled 
  SPI1_Write_Data(0x24, 0x20);                         // Anti aliasing filter bandwidth 800Hz, 16G (very sensitive), no self-test, 4-wire interface
  SPI1_Write_Data(0x10, 0x00);                         // Output(X) = Measurement(X) - OFFSET(X) * 32;
  SPI1_Write_Data(0x11, 0x00);                         // Output(Y) = Measurement(Y) - OFFSET(Y) * 32;
  SPI1_Write_Data(0x12, 0x00);                         // Output(Z) = Measurement(Z) - OFFSET(Z) * 32;
	reg =SPI1_Read_Data(0x23);
	printf("Who am 3 is 0x%x",reg);
	
 while (1)
 {
	x=(SPI1_Read_Data(0x29)<<8|SPI1_Read_Data(0x28));
	y=(SPI1_Read_Data(0x2B)<<8|SPI1_Read_Data(0x2A));
	z=(SPI1_Read_Data(0x2D)<<8|SPI1_Read_Data(0x2C));
	printf("x = %f  ======== y = %f =======z =%f\n",x,y,z);
	 Delay();
 }
 
}

void SPI1_InitFunction(void){
	/*config for GPIO SPI
	Base on schematic of F4 KIT:
	- PA5 : SPI_Clock
	- PA7 :SPI_MOSI
	- PA6: SPI_MISO
	- PE3 : CS
	*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	//for SPI 
	GPIO_StructForSPI1.GPIO_Mode = GPIO_Mode_AF;
	GPIO_StructForSPI1.GPIO_OType = GPIO_OType_PP;
	GPIO_StructForSPI1.GPIO_Pin =GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_StructForSPI1.GPIO_PuPd =GPIO_PuPd_NOPULL;
	GPIO_StructForSPI1.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_StructForSPI1);
	//pin source
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
	//for chip select
	GPIO_StructForSPI1.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_StructForSPI1.GPIO_OType = GPIO_OType_PP;
	GPIO_StructForSPI1.GPIO_Pin =GPIO_Pin_3;
	GPIO_StructForSPI1.GPIO_PuPd =GPIO_PuPd_UP;
	GPIO_StructForSPI1.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOE, &GPIO_StructForSPI1);

	//
	/*config for SPI
	SPI_Direction: Unidirectional or bidirectional. We need bidirectional here for read and write.
	SPI_Mode: Master or Slave - our stm32f4 shoulb be the master here!
	SPI_DataSize: You can send 8 Bits and 16 Bits - we use 8 Bits.
	SPI_CPOL: Clock polarity - I set this to High, as it is in the read&write protocol in LIS302DL datasheet.
	SPI_CPHA: Defines the edge for bit capture - I use 2nd edge.
	SPI_NSS: Chip select hardware/sofware - I set this so software.
	SPI_BaudRatePrescaler:  Defines the clock speed of our SPI - set it to maximum here (Prescaler 2)
	SPI_FirstBit: Starting with MSB or LSB ? - LIS302DL datasheet -> its MSB !
	SPI_CRCPolynomial: Polynomial for CRC calculation - we don't use it in this example.
	*/
	SPI1_Struct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
	SPI1_Struct.SPI_CPHA =SPI_CPHA_2Edge;//=1
	SPI1_Struct.SPI_CPOL=SPI_CPOL_High;
	SPI1_Struct.SPI_DataSize = SPI_DataSize_8b;
	SPI1_Struct.SPI_Direction=SPI_Direction_2Lines_FullDuplex;
	SPI1_Struct.SPI_FirstBit=SPI_FirstBit_MSB;
	SPI1_Struct.SPI_Mode=SPI_Mode_Master;
	SPI1_Struct.SPI_NSS = SPI_NSS_Soft;
	SPI_Init(SPI1, &SPI1_Struct);
	//
	SPI_Cmd(SPI1, ENABLE);
}
void SPI1_Write_Data(uint8_t address, uint8_t data){
	GPIO_ResetBits(GPIOE, GPIO_Pin_3);
 
while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)); 
SPI_I2S_SendData(SPI1, address);
while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
SPI_I2S_ReceiveData(SPI1);
 
while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)); 
SPI_I2S_SendData(SPI1, data);
while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
SPI_I2S_ReceiveData(SPI1);
 
GPIO_SetBits(GPIOE, GPIO_Pin_3);
}
uint8_t SPI1_Read_Data(uint8_t address){
	GPIO_ResetBits(GPIOE, GPIO_Pin_3); 
	address = 0x80 | address;
 
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)); 
	SPI_I2S_SendData(SPI1, address);
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
	SPI_I2S_ReceiveData(SPI1); //Clear RXNE bit
 
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)); 
	SPI_I2S_SendData(SPI1, 0x00); //Dummy byte to generate clock
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
 
GPIO_SetBits(GPIOE, GPIO_Pin_3);
 
return  SPI_I2S_ReceiveData(SPI1);
}
void NVIC_Config(void){
	NVIC_Struct.NVIC_IRQChannel = USART1_IRQn;//choose USART1 interrupt
	NVIC_Struct.NVIC_IRQChannelCmd = ENABLE;// enable global interrupt
	NVIC_Struct.NVIC_IRQChannelPreemptionPriority = 0;//the usart1 interrupt is more important
	NVIC_Struct.NVIC_IRQChannelSubPriority =0;
	NVIC_Init(&NVIC_Struct);
}
void USART1_IRQHandler(void){       
	
	if( USART_GetITStatus(USART1, USART_IT_RXNE) ){
		
		static uint8_t cnt = 0; // this counter is used to determine the string length
		char t = USART1->DR; // the character from the USART1 data register is saved in t
		
		/* check if the received character is not the LF character (used to determine end of string) 
		 * or the if the maximum string length has been been reached 
		 */
		if( t!='0' ){ 
			received_string[cnt] = t;
			cnt++;
		}
		else{ // otherwise reset the character counter and print the received string
		
			USART_puts(received_string,cnt);
			cnt = 0;
		}
	}
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line)
{ 
  while (1)
  {
  }
}
#endif

/**
  * @}
  */ 
