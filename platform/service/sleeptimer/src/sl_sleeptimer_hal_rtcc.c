/***************************************************************************//**
 * @file
 * @brief SLEEPTIMER hardware abstraction implementation for RTCC.
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include "em_rtcc.h"
#include "sl_sleeptimer.h"
#include "sl_sleeptimer_hal.h"
#include "em_core.h"
#include "em_cmu.h"

#if SL_SLEEPTIMER_PERIPHERAL == SL_SLEEPTIMER_PERIPHERAL_RTCC

// Minimum difference between current count value and what the comparator of the timer can be set to.
#define SLEEPTIMER_COMPARE_MIN_DIFF  2

#define SLEEPTIMER_TMR_WIDTH (_RTCC_CNT_MASK)

__STATIC_INLINE uint32_t get_time_diff(uint32_t a,
                                       uint32_t b);

/******************************************************************************
 * Initializes RTCC sleep timer.
 *****************************************************************************/
void sleeptimer_hal_init_timer(void)
{
  RTCC_Init_TypeDef rtcc_init   = RTCC_INIT_DEFAULT;
  RTCC_CCChConf_TypeDef channel = RTCC_CH_INIT_COMPARE_DEFAULT;

  CMU_ClockEnable(cmuClock_RTCC, true);

  rtcc_init.enable = false;
  rtcc_init.presc = (RTCC_CntPresc_TypeDef)(CMU_PrescToLog2(SL_SLEEPTIMER_FREQ_DIVIDER - 1));

  RTCC_Init(&rtcc_init);
  RTCC_ChannelInit(1u, &channel);

  RTCC_IntDisable(_RTCC_IEN_MASK);
  RTCC_IntClear(_RTCC_IF_MASK);
  RTCC_CounterSet(0u);

  RTCC_Enable(true);

  NVIC_ClearPendingIRQ(RTCC_IRQn);
  NVIC_EnableIRQ(RTCC_IRQn);
}

/******************************************************************************
 * Gets RTCC counter value.
 *****************************************************************************/
uint32_t sleeptimer_hal_get_counter(void)
{
  return RTCC_CounterGet();
}

/******************************************************************************
 * Gets RTCC compare value.
 *****************************************************************************/
uint32_t sleeptimer_hal_get_compare(void)
{
  return RTCC_ChannelCCVGet(1u);
}

/******************************************************************************
 * Sets RTCC compare value.
 *****************************************************************************/
void sleeptimer_hal_set_compare(uint32_t value)
{
  uint32_t counter = sleeptimer_hal_get_counter();
  uint32_t compare = sleeptimer_hal_get_compare();
  uint32_t compare_value = value;
  if (((RTCC_IntGet() & RTCC_IEN_CC1) != 0)
      || get_time_diff(compare, counter) > SLEEPTIMER_COMPARE_MIN_DIFF
      || compare == counter) {
    // Add margin if necessary
    if (get_time_diff(compare_value, counter) < SLEEPTIMER_COMPARE_MIN_DIFF) {
      compare_value = counter + SLEEPTIMER_COMPARE_MIN_DIFF;
    }
    compare_value %= SLEEPTIMER_TMR_WIDTH;

    RTCC_ChannelCCVSet(1u, compare_value);
    sleeptimer_hal_enable_int(SLEEPTIMER_EVENT_COMP);
  }
}

/******************************************************************************
 * Enables RTCC interrupts.
 *****************************************************************************/
void sleeptimer_hal_enable_int(uint8_t local_flag)
{
  uint32_t rtcc_ien = 0u;

  if (local_flag & SLEEPTIMER_EVENT_OF) {
    rtcc_ien |= RTCC_IEN_OF;
  }

  if (local_flag & SLEEPTIMER_EVENT_COMP) {
    rtcc_ien |= RTCC_IEN_CC1;
  }

  RTCC_IntEnable(rtcc_ien);
}

/******************************************************************************
 * Disables RTCC interrupts.
 *****************************************************************************/
void sleeptimer_hal_disable_int(uint8_t local_flag)
{
  uint32_t rtcc_int_dis = 0u;

  if (local_flag & SLEEPTIMER_EVENT_OF) {
    rtcc_int_dis |= RTCC_IEN_OF;
  }

  if (local_flag & SLEEPTIMER_EVENT_COMP) {
    rtcc_int_dis |= RTCC_IEN_CC1;
  }

  RTCC_IntDisable(rtcc_int_dis);
}

/*******************************************************************************
 * RTCC interrupt handler.
 ******************************************************************************/
void RTCC_IRQHandler(void)
{
  CORE_DECLARE_IRQ_STATE;
  uint8_t local_flag = 0;
  uint32_t irq_flag;

  CORE_ENTER_ATOMIC();
  irq_flag = RTCC_IntGet();

  if (irq_flag & RTCC_IF_OF) {
    local_flag |= SLEEPTIMER_EVENT_OF;
  }
  if (irq_flag & RTCC_IF_CC1) {
    local_flag |= SLEEPTIMER_EVENT_COMP;
  }
  RTCC_IntClear(irq_flag & (RTCC_IF_OF | RTCC_IF_CC1 | RTCC_IF_CC0));

  process_timer_irq(local_flag);

  CORE_EXIT_ATOMIC();
}

/*******************************************************************************
 * Gets RTCC timer frequency.
 ******************************************************************************/
uint32_t sleeptimer_hal_get_timer_frequency(void)
{
  return (CMU_ClockFreqGet(cmuClock_RTCC) >> (CMU_PrescToLog2(SL_SLEEPTIMER_FREQ_DIVIDER - 1)));
}

/*******************************************************************************
 * Computes difference between two times taking into account timer wrap-around.
 *
 * @param a Time.
 * @param b Time to substract from a.
 *
 * @return Time difference.
 ******************************************************************************/
__STATIC_INLINE uint32_t get_time_diff(uint32_t a,
                                       uint32_t b)
{
  return (a - b);
}

#endif