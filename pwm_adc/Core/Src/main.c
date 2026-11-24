/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
//空闲       exti下降沿触发的中断		       消抖 		长短按判定
	KEY_IDLE,    KEY_FALING_EDGE,       KEY_DEBOUNCE,KEY_PRESS
}key_state_t;
typedef enum
{
	close=0,open
}led_blink_state_t;
typedef enum
{
	keep_open=0,keep_close
}led_keep_state_t;
typedef enum
{
	light_level_0=0,light_level_1,light_level_2,light_level_3,breath
}led_breath_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define KEY_LEVEL_LOW	HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==GPIO_PIN_RESET
#define K_T_RAD (2.0f*3.1415f*20.0f/1000.0f/2.0f)
#define k_DR_V ((3.3f-0.0f)/4095.0f)
#define ARR (999+1)
#define servo_min 25u
#define	servo_max 120u
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
uint16_t CCR[4]={250,500,750,1000};
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t volatile key_state_v,led_blink_state_v,led_keep_state_v,led_breath_state_v;
uint32_t volatile press_cnt,led_blink_cnt,led_breath_cnt;
float vol;
uint32_t dr;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void key_long_cb(void);
void key_short_cb(void);
uint8_t get_ccr(float rad);
void servo_display(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void key_long_cb(void)
{
	//翻转led闪烁状态
	if(led_blink_state_v==open)	led_blink_state_v=close;
	else												led_blink_state_v=open;
	//进入/退出呼吸灯
	if(led_breath_state_v==breath)	led_breath_state_v=light_level_0;
	else												led_breath_state_v=breath;
	//释放按钮状态
	key_state_v=KEY_IDLE;
}
void key_short_cb(void)
{
	//强制关闭闪烁
	led_blink_state_v=close;
	//根据上次短按状态切换长亮长灭
	if(led_keep_state_v==keep_close)	led_keep_state_v=keep_open;
	else															led_keep_state_v=keep_close;
	//调光亮度 （进阶任务）
	if(led_breath_state_v!=breath)	led_breath_state_v=(led_breath_state_v+1)%4;
	//释放按钮状态
	key_state_v=KEY_IDLE;
}
/*
舵机角度转换为脉宽，获取的值为ccr
*/
uint8_t get_ccr(float rad)
{
	return (uint8_t)(rad/180.0f*(servo_max-servo_min))+servo_min;
}
/*对应作业task2*/
void servo_display(void)
{
//舵机运动展示，10ms更新一次角度
	for(int i=0;i<=180;i++)
	{
		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,get_ccr(i));
		HAL_Delay(10);
	}
	HAL_Delay(500);
	for(int i=180;i>=0;i--)
	{
		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,get_ccr(i));
		HAL_Delay(10);
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
  MX_ADC1_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2);
	
	//task2_舵机
	servo_display();
	HAL_ADC_Start(&hadc1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		//长亮灭轮询
	
		if(led_blink_state_v==close)
		{
		if(led_keep_state_v==keep_close)	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET);
		if(led_keep_state_v==keep_open)	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);
		}
		//轮询扫adc电压
		HAL_ADC_PollForConversion(&hadc1,HAL_MAX_DELAY);
		dr=HAL_ADC_GetValue(&hadc1);
		vol=dr*k_DR_V;
		//电位器控制舵机0->4096映射为servo_max->servo_min
		uint8_t ccr=get_ccr((4096u-dr)/4096.0f*180.0f);
		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,ccr);
		//轮询扫led2亮度
		if(led_breath_state_v<breath)
		{
			__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,CCR[led_breath_state_v]);
		}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/*
按钮，只置isr
*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
		if(GPIO_Pin==GPIO_PIN_0)
		{
			if(key_state_v==KEY_IDLE)	key_state_v=KEY_FALING_EDGE;
		}

}
/*
定时器中断，20ms周期，负责，按钮消抖,led闪烁,呼吸灯
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance!=TIM1)	return;
	//1.按钮处理,状态机控制
	switch(key_state_v)
	{
		case KEY_IDLE:
			//不处理
			break;
		case KEY_FALING_EDGE:
			//20ms后进消抖
			key_state_v=KEY_DEBOUNCE;
			break;
		case KEY_DEBOUNCE:
			if(KEY_LEVEL_LOW)
			{
				//确认被按下，开窗判断长短
				key_state_v=KEY_PRESS;
			}
			else
			{
				//抖动，恢复空闲
				key_state_v=KEY_IDLE;
			}
			break;
		case KEY_PRESS:
			//低计数，高判断,执行对应回调函数
			if(KEY_LEVEL_LOW)
			{
				press_cnt++;
			}
			else
			{
				if(press_cnt>=50)	key_long_cb();
				else 							key_short_cb();
				press_cnt=0;
			}
			break;
		}
		//2,led1闪烁，50hz5分频得到闪烁频率
			if(led_blink_state_v==open)
			{
				led_blink_cnt++;
			if(led_blink_cnt>=5)
			{
				HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_1);
				led_blink_cnt=0;
			}
			}
		//3,led2呼吸灯控制,正弦波
			if(led_breath_state_v==breath)
			{
				__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,((sinf((float)(led_breath_cnt)*K_T_RAD)+1.0f)/2.0f*ARR));
			led_breath_cnt=(led_breath_cnt+1)%100;
		
			}
		



}
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
#ifdef USE_FULL_ASSERT
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
