/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define ARM_MATH_CM4
#include "arm_math.h"
#include "TSL2591.h"
#include "bmp280.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BUFFER_SIZE 256 // We can chnage and see, currently 51 cycles worth at 10Fs
#define FFT_BUFFER_SIZE 1024 //FFT Buffer Double Buffering
#define SAMPLE_RATE_HZ 20500 //TIMER 2 Sampling Rate change for accuracy
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint16_t adcData[BUFFER_SIZE]; // ADC Sampling Data From DMA

float fftBufIn[FFT_BUFFER_SIZE]; //FFT float buffer
float fftBufOut[FFT_BUFFER_SIZE]; //FFT float buffer, alternating Real and Imaginary

BMP280_HandleTypedef bmp280;
float pressure=32.55;
float temperature=25.45;
float humidity=64.65;



static volatile uint16_t*inBufPtr; //Will help for double buffering, as we change the address of where we need to store ADC

uint8_t dataReadyFlag=0; //set to 0 and 1 if we can process the data

uint8_t fftFlag=0; // Flag for fft completed

uint8_t fftPeakCplt=0; //Flag for peak value FFT completed
//uint16_t adcControl; // might delete, only have 1 channel

uint8_t pirDetected=0;
uint8_t radarDetected=0;

char message[128];
int len = 0;
uint8_t currentHz=0;
uint8_t wakeUp=0;
uint8_t espTransfer=2;
uint8_t RxData[20];
uint8_t buf[15] = {0};
uint8_t fftCounter=0;
uint32_t timeBefore=0;



arm_rfft_fast_instance_f32 fftHandler; //FFT handler

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == ExButton_Pin)
	{
				sprintf(message, "ButtonTest\r");
				len = strlen(message);
				HAL_UART_Transmit(&huart1 , (uint8_t*)message , len , 100);

	}


	if(GPIO_Pin == PIR_Pin)
	{
		if(wakeUp)
		{
			HAL_GPIO_WritePin(PIR_LED_GPIO_Port, PIR_LED_Pin, SET);
			pirDetected=1;

		}
		else
		{
			HAL_GPIO_WritePin(PIR_LED_GPIO_Port, PIR_LED_Pin, SET);
			wakeUp=1;
			pirDetected = 1;

			HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
			HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
			buf[14] = sDate.Date;
			buf[13] = sTime.Hours;
			buf[12] = sTime.Minutes;
			buf[11] = sTime.Seconds;
//			==========================================================
		    uint8_t triggers = 0;
		    uint8_t PIR_triggered = 1, RADAR_triggered = 0;
		    triggers = (PIR_triggered << 1) | RADAR_triggered;
		    buf[10] = triggers;
//			===============================================================
			bmp280_read_float(&bmp280, &temperature, &pressure, &humidity);
			//HAL_HalfDuplex_EnableReceiver(&huart6);

			uint8_t* temp_buf = (uint8_t*) &temperature;
			buf[9] = temp_buf[0];
			buf[8] = temp_buf[1];
			buf[7] = temp_buf[2];
			buf[6] = temp_buf[3];
//			==============================================
			uint8_t* hum_buf = (uint8_t*) &humidity;
		    buf[5] = hum_buf[0];
		    buf[4] = hum_buf[1];
		    buf[3] = hum_buf[2];
		    buf[2] = hum_buf[3];
//			==============================================
			//sprintf(message, "C");
		    uint16_t lux = 2200; //TSL2591_Read_Lux();

			HAL_UART_Transmit(&huart1 , buf , 15 , 100); //name of structure, name of message , size of message, timeout
//			wakeUp=0;


		}
	}

	if(GPIO_Pin==Radar_Pin)
	{
		fftCounter=0;
		radarDetected=1;
		fftFlag=0;
		espTransfer=0;
		wakeUp=1;
		HAL_ResumeTick();
//		__disable_irq();
		HAL_GPIO_WritePin(RADAR_LED_GPIO_Port, RADAR_LED_Pin, SET);
		timeBefore = HAL_GetTick();

		//gpio
	}

}




void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) //Half way full buffer Interrupt
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1); //LED on signal

	inBufPtr = &adcData[0];

	dataReadyFlag =1;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) //Completely full buffer interrupt
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0); //LED off signal

	inBufPtr = &adcData[BUFFER_SIZE/2]; //Load Second half into pointer

	dataReadyFlag = 1;

}


void Process_HalfBuffer()
{


	static int16_t fftIndex =0; //We're not gonna needd since we're filling all the way

	for(uint8_t i = 0 ; i<(BUFFER_SIZE/2)-1 ; i++)
	{

		fftBufIn[fftIndex] = (float) inBufPtr[i];
		fftIndex++;


		//===================================
		//Test might delete nd stick to many multiple 512 and 256

		if(fftIndex==FFT_BUFFER_SIZE)
		{

			arm_rfft_fast_f32(&fftHandler, fftBufIn, fftBufOut, 0);

			fftFlag =1; //That we have filled our FFT buffer

			fftIndex=0;


		}
	}

}



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
//	char message[128];
//	int len = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */

  //UART==============================



  HAL_ADC_Start_DMA(&hadc1, (uint32_t *) adcData, BUFFER_SIZE);
  HAL_TIM_Base_Start(&htim2);

  arm_rfft_fast_init_f32(&fftHandler, FFT_BUFFER_SIZE);
//  DEV_ModuleInit();
//  //DEV_I2C_Init(&hi2c1);
//  TSL2591_Init();
//
//  bmp280_init_default_params(&bmp280.params);
//  bmp280.addr = BMP280_I2C_ADDRESS_0;
//  bmp280.i2c = &hi2c2;

//  while (!bmp280_init(&bmp280, &bmp280.params)) { come back too
//    		sprintf(message, "BMP280 initialization failed\n");
//    		len = strlen(message);
//    		HAL_UART_Transmit(&huart1 , (uint8_t*)message , len , 100); //name of structure, name of message , size of message, timeout
//    		HAL_Delay(2000);
//    	}
//    	bool bme280p = bmp280.id == BME280_CHIP_ID;
//    	 sprintf(message, "BMP280: found %s\n", bme280p ? "BME280" : "BMP280");
//    	 len = strlen(message);
//    	 HAL_UART_Transmit(&huart1 , (uint8_t*)message , len , 100); //name of structure, name of message , size of message, timeout



  float peakVal =0.0f;
  uint16_t peakHz=0;

  uint32_t timeAfter=0;


  uint8_t once =1;


  //__disable_irq();
  //asm("diq")

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  if(!wakeUp)
	  {
		  HAL_SuspendTick();
		  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	  }



	  if(dataReadyFlag) // Confirm if we have filled our half buffer
	  {
		  Process_HalfBuffer();

		  dataReadyFlag = 0;
	  }

	  if(fftFlag) //Confirm if we have filled our FFT buffer
	  {

		  // Compute absolute value of complex FFT and reset for next buffer

		  peakVal=0.0f;
		  peakHz = 0;

		  uint16_t freqIndex = 0;

		  for (uint16_t index = 2; index < FFT_BUFFER_SIZE; index+=2) //We start at 2 because of offset
		  {


			  float curVal = sqrtf((fftBufOut[index] * fftBufOut[index])+(fftBufOut[index+1] * fftBufOut[index+1]));

			  if(curVal > peakVal)
			  {
				  peakVal = curVal;
				  peakHz = (uint16_t)(freqIndex *(SAMPLE_RATE_HZ/FFT_BUFFER_SIZE));
			  }

			  freqIndex++;

		  }

		  fftFlag = 0;

		  fftPeakCplt=1;



	  }

	  if((fftPeakCplt && fftCounter<3 && radarDetected) || espTransfer==0) //Confirm if we have complete FFT
	  {


		  	  sprintf(message, "Frequency: %dHz \r", peakHz );
		  	  len = strlen(message);
		  	  HAL_UART_Transmit(&huart1 , (uint8_t*)message , len , 100); //name of structure, name of message , size of message, timeout

		  	  if(peakHz>60 )
		  	  {
		  		  espTransfer=1;
		  	  }


		  	  fftPeakCplt =0;
		  	  currentHz = peakHz;

		  	  fftCounter=fftCounter+1;
	  }



	  	  if(espTransfer==1)
	  		  	  {
//	  		  		  	  	  	  HAL_Delay(4000);
	  		  		  	  	  	  	  	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	  		  		  	  				HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	  		  		  	  				buf[14] = sDate.Date;
	  		  		  	  				buf[13] = sTime.Hours;
	  		  		  	  				buf[12] = sTime.Minutes;
	  		  		  	  				buf[11] = sTime.Seconds;
	  		  		  	  	//			==========================================================
	  		  		  	  			    uint8_t triggers = 0;
	  		  		  	  			    uint8_t PIR_triggered = 1, RADAR_triggered = 0;
	  		  		  	  			    triggers = (PIR_triggered << 1) | RADAR_triggered;
	  		  		  	  			    buf[10] = triggers;
	  		  		  	  	//			===============================================================
	  		  		  	  				//bmp280_read_float(&bmp280, &temperature, &pressure, &humidity);
	  		  		  	  				//HAL_HalfDuplex_EnableReceiver(&huart6);

	  		  		  	  				uint8_t* temp_buf = (uint8_t*) &temperature;
	  		  		  	  				buf[9] = temp_buf[0];
	  		  		  	  				buf[8] = temp_buf[1];
	  		  		  	  				buf[7] = temp_buf[2];
	  		  		  	  				buf[6] = temp_buf[3];
	  		  		  	  	//			==============================================
	  		  		  	  				uint8_t* hum_buf = (uint8_t*) &humidity;
	  		  		  	  			    buf[5] = hum_buf[0];
	  		  		  	  			    buf[4] = hum_buf[1];
	  		  		  	  			    buf[3] = hum_buf[2];
	  		  		  	  			    buf[2] = hum_buf[3];
	  		  		  	  	//			==============================================
	  		  		  	  				//sprintf(message, "C");
	  		  		  	  			    uint16_t lux = 2200; //TSL2591_Read_Lux();
	  		  		  	  			    buf[1] = (lux >> 8) & 0xFF;
	  		  		  	  			    buf[0] = lux & 0xFF;

	  		  		  	  				HAL_UART_Transmit(&huart1 , buf , 15 , 100); //name of structure, name of message , size of message, timeout
	  		  		  	  				timeAfter = HAL_GetTick();


	  		  		  	  				radarDetected=0;
	  		  		  	  				espTransfer = 2;
	  		  		  	  				fftCounter=3;

	  		  		  	  				HAL_GPIO_WritePin(RADAR_LED_GPIO_Port, RADAR_LED_Pin, 0);
	  		  		  	  		  		sprintf(message, "\r%dSleeping%d",(timeAfter-timeBefore), fftCounter);
	  		  		  	  		  		len = strlen(message);
	  		  		  	  		  		HAL_UART_Transmit(&huart1 , (uint8_t*)message , len , 100);

	  		  	  }


  }

  if(fftCounter==3 && HAL_GPIO_ReadPin(Radar_GPIO_Port, Radar_Pin)==GPIO_PIN_SET)
  {
//	  __enable_irq();

  }


  if(fftCounter==3)
  	  {

//  		  HAL_GPIO_WritePin(RADAR_LED_GPIO_Port, RADAR_LED_Pin, 0);
//  		  sprintf(message, "%dSleeping",(timeAfter-timeBefore));
//  		  len = strlen(message);
//  		  HAL_UART_Transmit(&huart1 , (uint8_t*)message , len , 100); //name of structure, name of message , size of message, timeout

  		  wakeUp=0;
//  		  __enable_irq();


  	  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
