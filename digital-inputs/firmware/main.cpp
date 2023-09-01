/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * big picture:
 * For each test pad we can do pull-up, pull-down and floating state (by turning mux off)
 * each of those tests should be executed twice:
 * first time while external device informs us that logic level is ON
 * second time while external device informs us that logic level is OFF
 */

#include "global.h"
#include "usbconsole.h"
#include "usbcfg.h"
#include "test_digital_inputs.h"
#include "test_digital_outputs.h"
#include "adc.h"
#include "test_logic.h"
#include "efilib.h"
#include "can.h"
#include "terminal_util.h"

BaseSequentialStream *chp = (BaseSequentialStream *)&EFI_CONSOLE_USB_DEVICE;

OutputMode outputMode;

/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */
static THD_WORKING_AREA(waThread1, THREAD_STACK);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {

//    palSetLine(LINE_LED1);
//    chThdSleepMilliseconds(50);

    palSetLine(LED_BLUE);
    chThdSleepMilliseconds(50);

//    palSetLine(LINE_LED3);
//    chThdSleepMilliseconds(200);

//    palClearLine(LINE_LED1);
//    chThdSleepMilliseconds(50);
    palClearLine(LED_BLUE);
    chThdSleepMilliseconds(50);
//    palClearLine(LINE_LED3);
//    chThdSleepMilliseconds(200);
  }
}

bool globalEverythingHappy;

static THD_WORKING_AREA(consoleThread, THREAD_STACK);
static void ConsoleThread(void*) {
	static int executionCounter = 0;

	while (true) {
	    globalEverythingHappy = true;
	    startNewCanTest();

        int currentIndex = 0;
		bool isGoodDigitalOutputs = testEcuDigitalOutputs(currentIndex);
		currentIndex += getDigitalOutputStepsCount();

		bool isGoodDigitalInputs = testEcuDigitalInputs(currentIndex);
		bool isHappyCounterStatus = checkCounterStatus();
		bool isAllGood = isGoodDigitalOutputs && isGoodDigitalInputs && isHappyCanTest() && isHappyCounterStatus;

		executionCounter++;

		if (isAllGood) {
		    setGreenText();
			chprintf(chp, " *********************************************** \r\n");
			chprintf(chp, " ************* ALL GOOD ************************ \r\n", executionCounter);
			chprintf(chp, " *********************************************** \r\n");
			setNormalText();
			palSetLine(LED_GREEN);
			palClearLine(LED_RED);
		    chThdSleepMilliseconds(5000);
		} else {
		    setRedText();
			chprintf(chp, " ************* SOMETHING BAD SEE ABOVE ************************ \r\n", executionCounter);
			setNormalText();
			palClearLine(LED_GREEN);
			palSetLine(LED_RED);
    		chThdSleepMilliseconds(5000);
		}

        // todo: assert 'rusEFI update seconds > specified XXX number'

	}
}

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  initAnalogInputs();
  initCan();
  initStimDigitalInputs();
  initStimDigitalOutputs();
  usb_serial_start();

  /*
   * Creates the example thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO + 1, Thread1, NULL);

  chThdCreateStatic(consoleThread, sizeof(consoleThread), NORMALPRIO, ConsoleThread, nullptr);


  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (true) {
    if (palReadLine(LINE_BUTTON)) {
    }
    chThdSleepMilliseconds(500);
  }
}
