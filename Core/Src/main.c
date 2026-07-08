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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CLAW_BUTTON_ACTIVE GPIO_PIN_RESET

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
typedef enum
{
  MOTOR_STOP = 0,
  MOTOR_FORWARD,
  MOTOR_REVERSE
} MotorDirection_t;

static void Motor_Set(GPIO_TypeDef *in1Port, uint16_t in1Pin,
                      GPIO_TypeDef *in2Port, uint16_t in2Pin,
                      MotorDirection_t direction)
{
  if (direction == MOTOR_FORWARD)
  {
    HAL_GPIO_WritePin(in2Port, in2Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(in1Port, in1Pin, GPIO_PIN_SET);
  }
  else if (direction == MOTOR_REVERSE)
  {
    HAL_GPIO_WritePin(in1Port, in1Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(in2Port, in2Pin, GPIO_PIN_SET);
  }
  else
  {
    HAL_GPIO_WritePin(in1Port, in1Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(in2Port, in2Pin, GPIO_PIN_RESET);
  }
}

static void Claw_Stop(void)
{
  Motor_Set(MOTOR1_IN1_GPIO_Port, MOTOR1_IN1_Pin,
            MOTOR1_IN2_GPIO_Port, MOTOR1_IN2_Pin,
            MOTOR_STOP);
  Motor_Set(MOTOR2_IN1_GPIO_Port, MOTOR2_IN1_Pin,
            MOTOR2_IN2_GPIO_Port, MOTOR2_IN2_Pin,
            MOTOR_STOP);
}

static void Claw_MoveUp(void)
{
  Motor_Set(MOTOR1_IN1_GPIO_Port, MOTOR1_IN1_Pin,
            MOTOR1_IN2_GPIO_Port, MOTOR1_IN2_Pin,
            MOTOR_FORWARD);
  Motor_Set(MOTOR2_IN1_GPIO_Port, MOTOR2_IN1_Pin,
            MOTOR2_IN2_GPIO_Port, MOTOR2_IN2_Pin,
            MOTOR_FORWARD);
}

static void Claw_MoveDown(void)
{
  Motor_Set(MOTOR1_IN1_GPIO_Port, MOTOR1_IN1_Pin,
            MOTOR1_IN2_GPIO_Port, MOTOR1_IN2_Pin,
            MOTOR_REVERSE);
  Motor_Set(MOTOR2_IN1_GPIO_Port, MOTOR2_IN1_Pin,
            MOTOR2_IN2_GPIO_Port, MOTOR2_IN2_Pin,
            MOTOR_REVERSE);
}

static uint8_t Button_IsPressed(GPIO_TypeDef *buttonPort, uint16_t buttonPin)
{
  return (HAL_GPIO_ReadPin(buttonPort, buttonPin) == CLAW_BUTTON_ACTIVE);
}

static void Claw_Task(void)
{
  uint8_t upPressed = Button_IsPressed(CLAW_UP_BUTTON_GPIO_Port, CLAW_UP_BUTTON_Pin);
  uint8_t downPressed = Button_IsPressed(CLAW_DOWN_BUTTON_GPIO_Port, CLAW_DOWN_BUTTON_Pin);

  if ((upPressed != 0U) && (downPressed == 0U))
  {
    Claw_MoveUp();
  }
  else if ((downPressed != 0U) && (upPressed == 0U))
  {
    Claw_MoveDown();
  }
  else
  {
    Claw_Stop();
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
  /* USER CODE BEGIN 2 */
  Claw_Stop();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    Claw_Task();
    HAL_Delay(10);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, MOTOR1_IN1_Pin|MOTOR1_IN2_Pin|MOTOR2_IN1_Pin|MOTOR2_IN2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : MOTOR1_IN1_Pin MOTOR1_IN2_Pin MOTOR2_IN1_Pin MOTOR2_IN2_Pin */
  GPIO_InitStruct.Pin = MOTOR1_IN1_Pin|MOTOR1_IN2_Pin|MOTOR2_IN1_Pin|MOTOR2_IN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : CLAW_UP_BUTTON_Pin CLAW_DOWN_BUTTON_Pin */
  GPIO_InitStruct.Pin = CLAW_UP_BUTTON_Pin|CLAW_DOWN_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
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
