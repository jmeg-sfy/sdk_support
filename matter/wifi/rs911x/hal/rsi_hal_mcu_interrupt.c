/*******************************************************************************
* @file  rsi_hal_mcu_interrupt.c
* @brief
*******************************************************************************
* # License
* <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* The licensor of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
******************************************************************************/

/**
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"
#include "em_ldma.h"
#include "dmadrv.h"
#include "em_core.h"
#include "sl_status.h"
#include "sl_device_init_clocks.h"
#include "gpiointerrupt.h"
#include "dmadrv.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

#include "wfx_host_events.h"
#include "wfx_rsi.h"

#include "rsi_driver.h"
#include "rsi_board_configuration.h"

typedef void (*UserIntCallBack_t)(void);
UserIntCallBack_t call_back, gpio_callback;
#ifdef LOGGING_STATS
uint8_t current_pin_set, prev_pin_set;
#endif /* LOGGING_STATS */

/* ARGSUSED */
void rsi_gpio_irq_cb(uint8_t irqnum)
{
  //uint32_t interrupt_mask;

  //WFX_RSI_LOG ("RSI: Got Int=%d", irqnum)
  if (irqnum != SL_WFX_HOST_PINOUT_SPI_IRQ)
    return;
  //interrupt_mask = GPIO_IntGet ();
  GPIO_IntClear(1 << SL_WFX_HOST_PINOUT_SPI_IRQ);

  //WFX_RSI_LOG ("Got SPI intr, cb=%x", (uint32_t)call_back);
  if (call_back != NULL)
    (*call_back)();
}
/*===================================================*/
/**
 * @fn           void rsi_hal_intr_config(void (* rsi_interrupt_handler)())
 * @brief        Starts and enables the SPI interrupt
 * @param[in]    rsi_interrupt_handler() ,call back function to handle interrupt
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to initialize the register/pins
 *               related to interrupts and enable the interrupts.
 */
void rsi_hal_intr_config(void (*rsi_interrupt_handler)(void))
{
  call_back = rsi_interrupt_handler;
  WFX_RSI_LOG("RSI:Set SPI intr CB to=%x", (uint32_t)call_back);
}

/*===================================================*/
/**
 * @fn           void rsi_hal_log_stats_intr_config(void (* rsi_give_wakeup_indication)())
 * @brief        Checks the interrupt and map/set gpio callback function
 * @param[in]    rsi_give_wakeup_indication() ,gpio call back function to handle interrupt
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code
 *               related to mapping of gpio callback function.
 */
#ifdef LOGGING_STATS
void rsi_hal_log_stats_intr_config(void (*rsi_give_wakeup_indication)())
{
  gpio_callback = rsi_give_wakeup_indication;
}
#endif
/*===================================================*/
/**
 * @fn           void rsi_hal_intr_mask(void)
 * @brief        Disables the SPI Interrupt
 * @param[in]    none
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to mask/disable interrupts.
 */
void rsi_hal_intr_mask(void)
{
  //WFX_RSI_LOG ("RSI:Disable IRQ");
  //NVIC_DisableIRQ(GPIO_ODD_IRQn);
  GPIO_IntDisable(1 << SL_WFX_HOST_PINOUT_SPI_IRQ);
}

/*===================================================*/
/**
 * @fn           void rsi_hal_intr_unmask(void)
 * @brief        Enables the SPI interrupt
 * @param[in]    none
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to enable interrupts.
 */
void rsi_hal_intr_unmask(void)
{
  // Unmask/Enable the interrupt
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
  GPIO_IntEnable(1 << SL_WFX_HOST_PINOUT_SPI_IRQ);
  //WFX_RSI_LOG ("RSI:Enable IRQ (mask=%x)", GPIO_IntGetEnabled ());
}

/*===================================================*/
/**
 * @fn           void rsi_hal_intr_clear(void)
 * @brief        Clears the pending interrupt
 * @param[in]    none
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to clear the handled interrupts.
 */
void rsi_hal_intr_clear(void)
{
  GPIO_IntClear(1 << SL_WFX_HOST_PINOUT_SPI_IRQ);
}

/*===================================================*/
/**
 * @fn          void rsi_hal_intr_pin_status(void)
 * @brief       Checks the SPI interrupt at pin level
 * @param[in]   none
 * @param[out]  uint8_t, interrupt status
 * @return      none
 * @description This API is used to check interrupt pin status(pin level whether it is high/low).
 */
uint8_t rsi_hal_intr_pin_status(void)
{
  uint32_t mask;
  // Return interrupt pin  status(high(1) /low (0))
  mask = GPIO_PinInGet (WFX_INTERRUPT_PIN.port, WFX_INTERRUPT_PIN.pin);

  return !!mask;
}
