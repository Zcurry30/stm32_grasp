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
#define SERVO_MIN_PULSE_US 500U
#define SERVO_MAX_PULSE_US 2500U
#define SERVO_MAX_ANGLE 180U
#define SERVO_STEP_INTERVAL_MS 20U

/* Calibrate these six angles after installing the two mirror-image servos. */
#define SERVO1_INITIAL_ANGLE 90U
#define SERVO2_INITIAL_ANGLE 90U
#define SERVO1_UP_ANGLE 35U
#define SERVO2_UP_ANGLE 145U
#define SERVO1_DOWN_ANGLE 145U
#define SERVO2_DOWN_ANGLE 35U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
typedef enum
{
  CLAW_STATE_INITIAL = 0,
  CLAW_STATE_GRAB_UP,
  CLAW_STATE_GRAB_DOWN
} ClawState_t;

static uint16_t servo1CurrentAngle = SERVO1_INITIAL_ANGLE;
static uint16_t servo2CurrentAngle = SERVO2_INITIAL_ANGLE;
static uint16_t servo1TargetAngle = SERVO1_INITIAL_ANGLE;
static uint16_t servo2TargetAngle = SERVO2_INITIAL_ANGLE;
static uint32_t lastServoStepTick;

static uint16_t Servo_AngleToPulse(uint16_t angle)
{
  if (angle > SERVO_MAX_ANGLE)
  {
    angle = SERVO_MAX_ANGLE;
  }
  return (uint16_t)(SERVO_MIN_PULSE_US +
                    ((uint32_t)angle * (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US)) /
                    SERVO_MAX_ANGLE);
}

static void Servo_WriteAngles(uint16_t servo1Angle, uint16_t servo2Angle)
{
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, Servo_AngleToPulse(servo1Angle));
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, Servo_AngleToPulse(servo2Angle));
}

static void Claw_SetState(ClawState_t state)
{
  if (state == CLAW_STATE_GRAB_UP)
  {
    servo1TargetAngle = SERVO1_UP_ANGLE;
    servo2TargetAngle = SERVO2_UP_ANGLE;
  }
  else if (state == CLAW_STATE_GRAB_DOWN)
  {
    servo1TargetAngle = SERVO1_DOWN_ANGLE;
    servo2TargetAngle = SERVO2_DOWN_ANGLE;
  }
  else
  {
    servo1TargetAngle = SERVO1_INITIAL_ANGLE;
    servo2TargetAngle = SERVO2_INITIAL_ANGLE;
  }
}

static uint16_t Servo_StepToward(uint16_t current, uint16_t target)
{
  if (current < target)
  {
    return current + 1U;
  }
  if (current > target)
  {
    return current - 1U;
  }
  return current;
}

static uint8_t Button_IsPressed(GPIO_TypeDef *buttonPort, uint16_t buttonPin)
{
  return (HAL_GPIO_ReadPin(buttonPort, buttonPin) == CLAW_BUTTON_ACTIVE);
}

static void Claw_Task(void)
{
  static uint8_t previousUpPressed;
  static uint8_t previousDownPressed;
  uint8_t upPressed = Button_IsPressed(CLAW_UP_BUTTON_GPIO_Port, CLAW_UP_BUTTON_Pin);
  uint8_t downPressed = Button_IsPressed(CLAW_DOWN_BUTTON_GPIO_Port, CLAW_DOWN_BUTTON_Pin);

  if ((upPressed != 0U) && (downPressed != 0U))
  {
    Claw_SetState(CLAW_STATE_INITIAL);
  }
  else if ((upPressed != 0U) && (previousUpPressed == 0U))
  {
    Claw_SetState(CLAW_STATE_GRAB_UP);
  }
  else if ((downPressed != 0U) && (previousDownPressed == 0U))
  {
    Claw_SetState(CLAW_STATE_GRAB_DOWN);
  }

  previousUpPressed = upPressed;
  previousDownPressed = downPressed;

  if ((HAL_GetTick() - lastServoStepTick) >= SERVO_STEP_INTERVAL_MS)
  {
    lastServoStepTick = HAL_GetTick();
    servo1CurrentAngle = Servo_StepToward(servo1CurrentAngle, servo1TargetAngle);
    servo2CurrentAngle = Servo_StepToward(servo2CurrentAngle, servo2TargetAngle);
    Servo_WriteAngles(servo1CurrentAngle, servo2CurrentAngle);
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
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  Servo_WriteAngles(servo1CurrentAngle, servo2CurrentAngle);
  if (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }

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
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 19999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 1500;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_TIM_MspPostInit(&htim2);
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
