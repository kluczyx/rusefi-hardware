#include "can_hw.h"
#include "io_pins.h"
#include "can.h"
#include "test_logic.h"
#include "can/can_common.h"
#include "global.h"
#include "terminal_util.h"

extern BaseSequentialStream *chp;
extern OutputMode outputMode;

static const CANConfig cancfg = {
  CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
  CAN_BTR_SJW(0) | CAN_BTR_TS2(1) |
  CAN_BTR_TS1(8) | CAN_BTR_BRP(6)
};

#define BENCH_HEADER ((int)bench_test_magic_numbers_e::BENCH_HEADER)

static bool isGoodCanPackets = true;
static bool hasReceivedAnalog = false;
static bool hasReceivedBoardId = false;
static CounterStatus counterStatus;
static int outputCount = 0;
static int dcOutputsCount = 0;
static int lowSideOutputCount = 0;

extern bool globalEverythingHappy;
extern bool isMuted;

static void canPacketError(const char *msg, ...) {
    setErrorLedAndRedText();
	chprintf(chp, " *********************************************** \r\n");

	va_list vl;
	va_start(vl, msg);
	chvprintf(chp, msg, vl);
	va_end(vl);

	chprintf(chp, " *********************************************** \r\n");
	setNormalText();

	isGoodCanPackets = false;
	globalEverythingHappy = false;
}

void startNewCanTest() {
    isGoodCanPackets = true;
    hasReceivedAnalog = false;
    hasReceivedBoardId = false;
    currentBoard = nullptr;
    dcOutputsCount = outputCount = 0;
    lowSideOutputCount = 0;
    // reset
	counterStatus = CounterStatus();
}

bool isHappyCanTest() {
    return isGoodCanPackets && hasReceivedAnalog;
}

bool checkDigitalInputCounterStatus() {
	if (currentBoard == nullptr) {
		setErrorLedAndRedText();
		chprintf(chp, "* UNKNOWN BOARD ID while trying to check digital input event counter!\r\n");
	    setNormalText();
		return false;
	}

	bool isHappy = true;
	
	for (auto & evtCnt : counterStatus.eventCounters) {
		if (!currentBoard->eventExpected[evtCnt.canFrameIndex])
			continue;
		if (!evtCnt.nonZero) {
		    setErrorLedAndRedText();
			chprintf(chp, "* ZERO %s event counter!\r\n", evtCnt.name);
			setNormalText();
		}
		isHappy = isHappy && evtCnt.nonZero;
	}
	
	for (auto & btnCnt : counterStatus.buttonCounters) {
		if (!currentBoard->buttonExpected[btnCnt.canFrameIndex])
			continue;
		if (!btnCnt.nonZero) {
		    setErrorLedAndRedText();
			chprintf(chp, "* ZERO %s button counter!\r\n", btnCnt.name);
			setNormalText();
		}
		isHappy = isHappy && btnCnt.nonZero;
	}

	return isHappy;
}

int getDigitalOutputStepsCount() {
	return outputCount;
}

int getDigitalDcOutputStepsCount() {
	return dcOutputsCount;
}

int getLowSideOutputCount() {
	return lowSideOutputCount;
}

static bool wasBoardDetectError = false;
int numSecondsSinceReset;

static void receiveBoardStatus(const uint8_t msg[CAN_FRAME_SIZE]) {
	numSecondsSinceReset = (msg[2] << 16) | (msg[3] << 8) | msg[4];
	if (hasReceivedBoardId) {
	    return;
	}
	hasReceivedBoardId = true;

	int boardId = (msg[0] << 8) | msg[1];
	int engineType = (msg[5] << 8) | msg[6];

	if (outputMode.displayCanReceive) {
	    chprintf(chp, "       CAN RX BoardStatus: BoardID=%d numSecs=%d\r\n", boardId, numSecondsSinceReset);
	}
	if (currentBoard == nullptr) {
		for (size_t boardIdx = 0; boardIdx < getBoardsCount(); boardIdx++) {
			BoardConfig &c = getBoardConfigs()[boardIdx];
			for (int boardRev = 0; c.boardIds[boardRev] > 0; boardRev++) {
				if (boardId == c.boardIds[boardRev]) {
					currentBoard = &c;
					currentBoardRev = boardRev;
					chprintf(chp, " * Board detected: %s rev.%c\r\n", currentBoard->boardName, 'A' + currentBoardRev);

					if (c.desiredEngineConfig != -1 && c.desiredEngineConfig != engineType) {
					    sendCanTxMessage((int)bench_test_packet_ids_e::IO_CONTROL, { BENCH_HEADER, (int)bench_test_io_control_e::CAN_BENCH_SET_ENGINE_TYPE, c.desiredEngineConfig });

					    chprintf(chp, " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
					    chprintf(chp, " !!!!!!!!!!!!!!!!!!!!!!!!!!! changing engine type !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
					    chprintf(chp, " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
					}
				}
			}
		}
	}
	if (currentBoard == nullptr && !wasBoardDetectError) {
		canPacketError("Error! Couldn't detect, unknown board!\r\n");
		wasBoardDetectError = true;
	}
}

static void receiveOutputMetaInfo(const uint8_t msg[CAN_FRAME_SIZE]) {
	if (msg[0] == BENCH_HEADER) {
		outputCount = msg[2];
		lowSideOutputCount = msg[3];
		dcOutputsCount = msg[4];
    	if (outputMode.displayCanReceive && !isMuted) {
    	    chprintf(chp, "       CAN ECU says: total=%d outputs of which low side=%d also %d DC\r\n", outputCount, lowSideOutputCount, dcOutputsCount);
    	}
	}
}

static void receiveRawAnalog(const uint8_t msg[CAN_FRAME_SIZE], size_t offset) {
	// wait for the BoardStatus package first
	if (currentBoard == nullptr)
		return;
	hasReceivedAnalog = true;


	for (size_t byteIndex = 0; byteIndex < CAN_FRAME_SIZE; byteIndex++) {
        size_t ch = offset + byteIndex;
		// channel not used for this board
		if (currentBoard->channels[ch].name == nullptr)
			continue;
		float voltage = getVoltageFrom8Bit(msg[byteIndex]) * currentBoard->channels[ch].mulCoef;

#if 0
		setGreenText();
		chprintf(chp, " ************* %s analog %d %d voltage=%f\r\n",
		    currentBoard->channels[ch].name,
		    offset,
		    byteIndex,
		    voltage);
		setNormalText();
#endif

		// check if in acceptable range for this board
		if (voltage < currentBoard->channels[ch].acceptMin || voltage > currentBoard->channels[ch].acceptMax) {
			canPacketError(" * BAD channel %d (%s): voltage %f (raw %d) not in range (%f..%f)\r\n",
				ch, currentBoard->channels[ch].name, voltage, msg[byteIndex],
				currentBoard->channels[ch].acceptMin, currentBoard->channels[ch].acceptMax);
		}
	}
}

static void receiveEventCounters(const uint8_t msg[CAN_FRAME_SIZE]) {
	for (auto & evtCnt : counterStatus.eventCounters) {
		evtCnt.nonZero = evtCnt.nonZero || (msg[evtCnt.canFrameIndex] > 0);
	}
}

static void receiveButtonCounters(const uint8_t msg[CAN_FRAME_SIZE]) {
	for (auto & btnCnt : counterStatus.buttonCounters) {
		btnCnt.nonZero = btnCnt.nonZero || (msg[btnCnt.canFrameIndex] > 0);
	}
}

static void printRxFrame(const CANRxFrame& frame, const char *msg) {
    if (!outputMode.displayCanReceive || isMuted) {
        return;
    }
		chprintf(chp, "                          Processing %s ID=%x/l=%x %x %x %x %x %x %x %x %x\r\n",
		msg,
		        CAN_EID(frame),
		        frame.DLC,
				frame.data8[0], frame.data8[1],
				frame.data8[2], frame.data8[3],
				frame.data8[4], frame.data8[5],
				frame.data8[6], frame.data8[7]);
}

void processCanRxMessage(const CANRxFrame& frame) {
#if 0
		setGreenText();
		chprintf(chp, " ************* GOT CAN %x\r\n",
		    CAN_EID(frame));
		setNormalText();
#endif

	if (CAN_EID(frame) == (int)bench_test_packet_ids_e::BOARD_STATUS) {
          printRxFrame(frame, "BENCH_TEST_BOARD_STATUS");
		receiveBoardStatus(frame.data8);
	} else if (CAN_EID(frame) == (int)bench_test_packet_ids_e::RAW_ANALOG_1) {
	    printRxFrame(frame, "BENCH_TEST_RAW_ANALOG_1");
		receiveRawAnalog(frame.data8, 0);
	} else if (CAN_EID(frame) == (int)bench_test_packet_ids_e::RAW_ANALOG_2) {
	    printRxFrame(frame, "BENCH_TEST_RAW_ANALOG_2");
        receiveRawAnalog(frame.data8, 8);
	} else if (CAN_EID(frame) == (int)bench_test_packet_ids_e::EVENT_COUNTERS) {
	    printRxFrame(frame, "BENCH_TEST_EVENT_COUNTERS");
	    receiveEventCounters(frame.data8);
	} else if (CAN_EID(frame) == (int)bench_test_packet_ids_e::BUTTON_COUNTERS) {
	    printRxFrame(frame, "BENCH_TEST_BUTTON_COUNTERS");
	    receiveButtonCounters(frame.data8);
	} else if (CAN_EID(frame) == (int)bench_test_packet_ids_e::IO_META_INFO) {
	    printRxFrame(frame, "BENCH_TEST_IO_META_INFO");
	    receiveOutputMetaInfo(frame.data8);
	}
}

void sendCanPinState(uint8_t pinIdx, bool isSet) {
	sendCanTxMessage((int)bench_test_packet_ids_e::IO_CONTROL, { BENCH_HEADER, (uint8_t)(isSet ? (int)bench_test_io_control_e::CAN_QC_OUTPUT_CONTROL_SET : (int)bench_test_io_control_e::CAN_QC_OUTPUT_CONTROL_CLEAR), pinIdx });
}

void sendCanDcState(uint8_t dcIndex, bool isSet) {
	sendCanTxMessage((int)bench_test_packet_ids_e::IO_CONTROL, { BENCH_HEADER, (int)bench_test_io_control_e::CAN_QC_ETB, dcIndex, isSet });
}

void setOutputCountRequest() {
	sendCanTxMessage((int)bench_test_packet_ids_e::IO_CONTROL, { BENCH_HEADER, (int)bench_test_io_control_e::CAN_BENCH_GET_COUNT });
}

static THD_WORKING_AREA(can_rx_wa, THREAD_STACK);
static THD_FUNCTION(can_rx, p) {
  CANRxFrame rxmsg;

  (void)p;
  while (true) {
    msg_t result = canReceiveTimeout(&CAND1, CAN_ANY_MAILBOX, &rxmsg, TIME_INFINITE);
	if (result != MSG_OK) {
		continue;
	}

    processCanRxMessage(rxmsg);
  }
}

void initCan() {
  palSetPadMode(CAN_PORT,CAN_PIN_RX, PAL_MODE_ALTERNATE(EFI_CAN_AF));
  palSetPadMode(CAN_PORT,CAN_PIN_TX, PAL_MODE_ALTERNATE(EFI_CAN_AF));

  canStart(&CAND1, &cancfg);

  initCanHw();
  chThdCreateStatic(can_rx_wa, sizeof(can_rx_wa), NORMALPRIO + 7,
                    can_rx, NULL);
}
