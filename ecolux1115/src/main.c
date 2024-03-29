
#include "printf.h"
#include "stm32f10x.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "hw_config.h"
#include "usb_pwr.h"


extern __IO uint32_t count_out;
extern uint8_t buffer_out[VIRTUAL_COM_PORT_DATA_SIZE];
__IO uint32_t TimeDisplay = 0;

USART_InitTypeDef USART_InitStructureTerm;

int puts(int c) {
	USART_SendData(USART2, c);
	/* spin until the end of transmission */
	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET) {
	}
}

/**
 * @brief  Configures the RTC.
 * @param  None
 * @retval None
 */
void RTC_Configuration(void) {
	/* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);

	/* Reset Backup Domain */
	BKP_DeInit();

	/* Enable LSE */
	RCC_LSEConfig(RCC_LSE_ON);
	/* Wait till LSE is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {
	}

	/* Select LSE as RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

	/* Enable RTC Clock */
	RCC_RTCCLKCmd(ENABLE);

	/* Wait for RTC registers synchronization */
	RTC_WaitForSynchro();

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	/* Enable the RTC Second */
	RTC_ITConfig(RTC_IT_SEC, ENABLE);

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	/* Set RTC prescaler: set RTC period to 1sec */
	RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
}

/**
 * @brief  Returns the time entered by user, using Hyperterminal.
 * @param  None
 * @retval Current time RTC counter value
 */
uint32_t Time_Regulate(void) {
	uint32_t Tmp_HH = 0xFF, Tmp_MM = 0xFF, Tmp_SS = 0xFF;

	//printf("\n==============Time Settings===========");
	//printf("\n  Please Set Hours");

	//while (Tmp_HH == 0xFF) {
		Tmp_HH = 12; // USART_Scanf(23);
	//}
	//printf(":  %d", Tmp_HH);
	//printf("\n  Please Set Minutes");
	//while (Tmp_MM == 0xFF) {
		Tmp_MM = 30; // USART_Scanf(59);
	//}
	//printf(":  %d", Tmp_MM);
	//printf("\n  Please Set Seconds");
	//while (Tmp_SS == 0xFF) {
		Tmp_SS = 30; // USART_Scanf(59);
	//}
	//printf(":  %d", Tmp_SS);

	/* Return the value to store in RTC counter register */
	return ((Tmp_HH*3600 + Tmp_MM*60 + Tmp_SS));
}

/**
 * @brief  Adjusts time.
 * @param  None
 * @retval None
 */
void Time_Adjust(void) {
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	/* Change the current time */
	RTC_SetCounter(Time_Regulate());
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
}

/**
 * @brief  Displays the current time.
 * @param  TimeVar: RTC counter value.
 * @retval None
 */
void Time_Display(uint32_t TimeVar) {
	uint32_t THH = 0, TMM = 0, TSS = 0;

	/* Compute  hours (may have rolled into date while powered down) */
	THH = (TimeVar % 86400) / 3600;
	/* Compute minutes */
	TMM = (TimeVar % 3600) / 60;
	/* Compute seconds */
	TSS = TimeVar % 60;

	printf("Time: %02d:%02d:%02d\n", THH, TMM, TSS);

}



/**
 * @brief Definition for COM port2, connected to USART2
 */
#define EVAL_COM2                   USART2
#define EVAL_COM2_GPIO              GPIOA
#define EVAL_COM2_CLK               RCC_APB1Periph_USART2
#define EVAL_COM2_GPIO_CLK          RCC_APB2Periph_GPIOA
#define EVAL_COM2_RxPin             GPIO_Pin_3
#define EVAL_COM2_TxPin             GPIO_Pin_2

void STM_EVAL_COMInit(USART_InitTypeDef* USART_InitStruct)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB1Periph_USART2 | RCC_APB2Periph_AFIO, ENABLE);

  /* Enable UART clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);


  /* Configure USART Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure USART Rx as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* USART configuration */
  USART_Init(USART2, USART_InitStruct);

  /* Enable USART */
  USART_Cmd(USART2, ENABLE);
}


/*******************************************************************************
 * Function Name  : main.
 * Description    : Main routine.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
int main(void) {
	Set_System();
	//Set_USBClock();
	//USB_Interrupts_Config();
	//USB_Init();

	USART_InitStructureTerm.USART_BaudRate = 115200;
	USART_InitStructureTerm.USART_WordLength = USART_WordLength_8b;
	USART_InitStructureTerm.USART_StopBits = USART_StopBits_1;
	USART_InitStructureTerm.USART_Parity = USART_Parity_No;
	USART_InitStructureTerm.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructureTerm.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	STM_EVAL_COMInit(&USART_InitStructureTerm);

	printf("   Booting up: %02d\n", 55);

	printf("   RTC init: %d\n", 1);
	rtcinit();
	Time_Display(RTC_GetCounter());

	printf("   Device scan 1-wire: %d\n", 2);
	probeinit();


	while (1) {

		/* Time event */
		if (TimeDisplay == 1) {
			TimeDisplay = 0;
			/* Display current time */
			Time_Display(RTC_GetCounter());
		}

		/* temperature conversion finished */

	}
}

#ifdef USE_FULL_ASSERT
/*******************************************************************************
 * Function Name  : assert_failed
 * Description    : Reports the name of the source file and the source line number
 *                  where the assert_param error has occurred.
 * Input          : - file: pointer to the source file name
 *                  - line: assert_param error line source number
 * Output         : None
 * Return         : None
 *******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\n", file, line) */

	/* Infinite loop */
	while (1)
	{}
}
#endif



void rtcinit(void) {

	// enable RTC interupt routing in the NVIC
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);


	if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5) {
		/* Backup data register value is not correct or not yet programmed (when
		 the first time the program is executed) */

		printf("Initialising RTC to zero\n");

		/* RTC Configuration */
		RTC_Configuration();

		printf("RTC configured\n");

		/* Adjust time by values entred by the user on the hyperterminal */
		Time_Adjust();

		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
	} else {
		/* Check if the Power On Reset flag is set */
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET) {
			printf("Power On Reset occurred\n");
		}
		/* Check if the Pin Reset flag is set */
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET) {
			printf("External Reset occurred\n");
		}

		printf("No need to configure RTC\n");
		/* Wait for RTC registers synchronization */
		RTC_WaitForSynchro();

		/* Enable the RTC Second interupt */
		RTC_ITConfig(RTC_IT_SEC, ENABLE);
		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();

	}
}


void probeinit(void) {

	// enable RTC interupt routing in the NVIC
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}
