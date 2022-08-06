/*
 * sent.h
 *
 *  Created on: 16 May 2022
 *      Author: alexv
 */

#pragma once

// Sent channel1 - PA6
#define HAL_SENT_CH1_LINE_PORT GPIOA
#define HAL_SENT_CH1_LINE_PIN 6

#define HAL_SENT_CH1_LINE               PAL_LINE(HAL_SENT_CH1_LINE_PORT, HAL_SENT_CH1_LINE_PIN)

// Sent channel2 - PA7
#define HAL_SENT_CH2_LINE_PORT GPIOA
#define HAL_SENT_CH2_LINE_PIN 7

#define HAL_SENT_CH2_LINE               PAL_LINE(HAL_SENT_CH2_LINE_PORT, HAL_SENT_CH2_LINE_PIN)

#define     CLOCK_TICK 72   // System clock value in MHz

#if SENT_DEV == SENT_SILABS_SENS
#define     SENT_TICK 5 // 5 us

#define     SENT_SYNC_VAL_TICKS 56

#define     SENT_0_VAL_TICKS 12
#define     SENT_1_VAL_TICKS 13
#define     SENT_2_VAL_TICKS 14
#define     SENT_3_VAL_TICKS 15
#define     SENT_4_VAL_TICKS 16
#define     SENT_5_VAL_TICKS 17
#define     SENT_6_VAL_TICKS 18
#define     SENT_7_VAL_TICKS 19
#define     SENT_8_VAL_TICKS 20
#define     SENT_9_VAL_TICKS 21
#define     SENT_10_VAL_TICKS 22
#define     SENT_11_VAL_TICKS 23
#define     SENT_12_VAL_TICKS 24
#define     SENT_13_VAL_TICKS 25
#define     SENT_14_VAL_TICKS 26
#define     SENT_15_VAL_TICKS 27

#define     SENT_TICKS_DELTA 208// 2.9 us * 72

#define     SENT_56_TICKS (56 * SENT_TICK * CLOCK_TICK)

#define     SENT_12_TICKS (12 * SENT_TICK * CLOCK_TICK)
#define     SENT_13_TICKS (13 * SENT_TICK * CLOCK_TICK)
#define     SENT_14_TICKS (14 * SENT_TICK * CLOCK_TICK)
#define     SENT_15_TICKS (15 * SENT_TICK * CLOCK_TICK)
#define     SENT_16_TICKS (16 * SENT_TICK * CLOCK_TICK)
#define     SENT_17_TICKS (17 * SENT_TICK * CLOCK_TICK)
#define     SENT_18_TICKS (18 * SENT_TICK * CLOCK_TICK)
#define     SENT_19_TICKS (19 * SENT_TICK * CLOCK_TICK)
#define     SENT_20_TICKS (20 * SENT_TICK * CLOCK_TICK)
#define     SENT_21_TICKS (21 * SENT_TICK * CLOCK_TICK)
#define     SENT_22_TICKS (22 * SENT_TICK * CLOCK_TICK)
#define     SENT_23_TICKS (23 * SENT_TICK * CLOCK_TICK)
#define     SENT_24_TICKS (24 * SENT_TICK * CLOCK_TICK)
#define     SENT_25_TICKS (25 * SENT_TICK * CLOCK_TICK)
#define     SENT_26_TICKS (26 * SENT_TICK * CLOCK_TICK)
#define     SENT_27_TICKS (27 * SENT_TICK * CLOCK_TICK)

// SYNC 56 ticks - 280 us
#define     SENT_56_TICKS_MAX SENT_56_TICKS + SENT_TICKS_DELTA
#define     SENT_56_TICKS_MIN SENT_56_TICKS - SENT_TICKS_DELTA

// 0
#define     SENT_12_TICKS_MAX SENT_12_TICKS + SENT_TICKS_DELTA
#define     SENT_12_TICKS_MIN SENT_12_TICKS - SENT_TICKS_DELTA

// 1
#define     SENT_13_TICKS_MAX SENT_13_TICKS + SENT_TICKS_DELTA
#define     SENT_13_TICKS_MIN SENT_13_TICKS - SENT_TICKS_DELTA

// 2
#define     SENT_14_TICKS_MAX SENT_14_TICKS + SENT_TICKS_DELTA
#define     SENT_14_TICKS_MIN SENT_14_TICKS - SENT_TICKS_DELTA

// 3
#define     SENT_15_TICKS_MAX SENT_15_TICKS + SENT_TICKS_DELTA
#define     SENT_15_TICKS_MIN SENT_15_TICKS - SENT_TICKS_DELTA

// 4
#define     SENT_16_TICKS_MAX SENT_16_TICKS + SENT_TICKS_DELTA
#define     SENT_16_TICKS_MIN SENT_16_TICKS - SENT_TICKS_DELTA

// 5
#define     SENT_17_TICKS_MAX SENT_17_TICKS + SENT_TICKS_DELTA
#define     SENT_17_TICKS_MIN SENT_17_TICKS - SENT_TICKS_DELTA

// 6
#define     SENT_18_TICKS_MAX SENT_18_TICKS + SENT_TICKS_DELTA
#define     SENT_18_TICKS_MIN SENT_18_TICKS - SENT_TICKS_DELTA

// 7
#define     SENT_19_TICKS_MAX SENT_19_TICKS + SENT_TICKS_DELTA
#define     SENT_19_TICKS_MIN SENT_19_TICKS - SENT_TICKS_DELTA

// 8
#define     SENT_20_TICKS_MAX SENT_20_TICKS + SENT_TICKS_DELTA
#define     SENT_20_TICKS_MIN SENT_20_TICKS - SENT_TICKS_DELTA

// 9
#define     SENT_21_TICKS_MAX SENT_21_TICKS + SENT_TICKS_DELTA
#define     SENT_21_TICKS_MIN SENT_21_TICKS - SENT_TICKS_DELTA

// 10
#define     SENT_22_TICKS_MAX SENT_22_TICKS + SENT_TICKS_DELTA
#define     SENT_22_TICKS_MIN SENT_22_TICKS - SENT_TICKS_DELTA

// 11
#define     SENT_23_TICKS_MAX SENT_23_TICKS + SENT_TICKS_DELTA
#define     SENT_23_TICKS_MIN SENT_23_TICKS - SENT_TICKS_DELTA

// 12
#define     SENT_24_TICKS_MAX SENT_24_TICKS + SENT_TICKS_DELTA
#define     SENT_24_TICKS_MIN SENT_24_TICKS - SENT_TICKS_DELTA

// 13
#define     SENT_25_TICKS_MAX SENT_25_TICKS + SENT_TICKS_DELTA
#define     SENT_25_TICKS_MIN SENT_25_TICKS - SENT_TICKS_DELTA

// 14
#define     SENT_26_TICKS_MAX SENT_26_TICKS + SENT_TICKS_DELTA
#define     SENT_26_TICKS_MIN SENT_26_TICKS - SENT_TICKS_DELTA

// 15
#define     SENT_27_TICKS_MAX SENT_27_TICKS + SENT_TICKS_DELTA
#define     SENT_27_TICKS_MIN SENT_27_TICKS - SENT_TICKS_DELTA
#endif

uint8_t SENT_GetTickValue(uint16_t dwt_val);
