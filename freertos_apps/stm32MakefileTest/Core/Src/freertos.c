/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include "allocators.h"
#include "cmsis_os.h"

#include <rcl/rcl.h>
#include <rmw_microxrcedds_c/config.h>
#include <ucdr/microcdr.h>
#include <uxr/client/client.h>
#include <lwip.h>
#include <stdio.h>

#include <rmw_microxrcedds_c/config.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern struct netif gnetif;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 1500 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
  bool availableNetwork = false;

#ifdef RMW_UXRCE_TRANSPORT_CUSTOM 
  availableNetwork = true; 
  rmw_uros_set_custom_transport( 
    true, 
    (void *) &huart3, 
    freertos_serial_open, 
    freertos_serial_close, 
    freertos_serial_write, 
    freertos_serial_read); 
#elif defined(MICRO_XRCEDDS_UDP)
  printf("Ethernet Initialization \r\n");

  printf("Waiting for IP \r\n");
  int retries = 0;
  
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);

  while(gnetif.ip_addr.addr == 0 && retries < 10){
  	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);
	osDelay(500);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET);
	retries++;
  };

  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);

  availableNetwork = (gnetif.ip_addr.addr != 0);
  if(availableNetwork){
  	printf("IP: %s \r\n", ip4addr_ntoa(&(gnetif.ip_addr)));
  } else {
  	printf("Impossible to retrieve an IP \n");
  }
#endif
  printf("endif----- \r\n");

  rcl_allocator_t freeRTOS_allocator = rcutils_get_zero_initialized_allocator();
  freeRTOS_allocator.allocate = __freertos_allocate;
  freeRTOS_allocator.deallocate = __freertos_deallocate;
  freeRTOS_allocator.reallocate = __freertos_reallocate;
  freeRTOS_allocator.zero_allocate = __freertos_zero_allocate;
  
  if(!rcutils_set_default_allocator(&freeRTOS_allocator)){
  	printf("Error on default allocators (line %d)\n", __LINE__);
  }
  else {
  	printf("freeRTOS allocator success \r\n");
  }

  osThreadAttr_t attributes;
  memset(&attributes, 0x0, sizeof(osThreadAttr_t));
  attributes.stack_size = 5 * 3000;
  attributes.priority = (osPriority_t)osPriorityNormal1;
  printf("before appMain \r\n");
  osThreadNew(appMain, NULL, &attributes);
  osDelay(500);
  char ptrTaskList[500];
  vTaskList(ptrTaskList);
  printf("**********************************\n");
  printf("Task  State   Prio    Stack    Num\n");
  printf("**********************************\n");
  printf(ptrTaskList);
  printf("**********************************\n");

  TaskHandle_t xHandle;
  xHandle = xTaskGetHandle("microROS_app");

  for(;;)
  {
    printf("into the printf \r\n");
    if (eTaskGetState(xHandle) != eSuspended && availableNetwork)
    //if(1)
    {
    	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);    
	osDelay(150);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET);
	osDelay(150);	
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);    
	osDelay(150);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
	osDelay(150);
	printf("available \r\n");
    }
    else {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);    
	osDelay(1000);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET);
	osDelay(1000);	
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);    
	osDelay(1000);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);
	osDelay(1000);
	printf("not available \r\n");
    }
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
