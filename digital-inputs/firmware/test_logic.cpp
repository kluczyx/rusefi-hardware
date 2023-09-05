
#include "global.h"
#include "adc.h"
#include "test_digital_outputs.h"
#include "test_logic.h"
#include "can.h"
#include "terminal_util.h"
#include "board_id/boards_id.h"
#include "board_id/boards_dictionary.h"
#include "board_id/qc_stim_meta.h"

#define COUNT 48
// 7.5% accuracy
#define ANALOG_L (1.0f - 0.075f)
#define ANALOG_H (1.0f + 0.075f)
// 10% for low voltage
#define ANALOG_H_FOR_LOW_VOLTAGE (1.0f + 0.12f)

#define HELLEN_R 4700
#define ALPHA2CH_R 2700
#define PROTEUS_R 2700

#define IAT_VALUE(r) (5.0f * 1000/(1000+r))
#define CLT_VALUE(r) (5.0f * 2000/(2000+r))

// normal atmospheric pressure is 101.3 kPa
// transfer function taken from https://www.nxp.com/docs/en/data-sheet/MPXH6400A.pdf
#define MAP_MPX6400_VALUE (5.0f * (0.002421 * 101.3 - 0.00842))


extern BaseSequentialStream *chp;
bool haveSeenLow[COUNT];
bool haveSeenHigh[COUNT];

constexpr int cycleDurationMs = 100;
constexpr int cycleCount = 4;

BoardConfig boardConfigs[] = {
	{
		.boardName = "Hellen-Honda125K",
		.desiredEngineConfig = -1,
		.boardIds = { BOARD_ID_HONDA125_A, BOARD_ID_HONDA125_B, BOARD_ID_HONDA125_C, BOARD_ID_HONDA125_D, 0 },
		.channels = {
			{ "TPS1_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ "MAP", 1.0f, 0.6f * ANALOG_L, 0.6f * ANALOG_H },
			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ "BATT", 5.835f, 9.0f, 15.0f },
		},
		.eventExpected = {true, false, true, true, false, false, true},
		.buttonExpected = {
		/*BrakePedal todo add wire */true,
		/*ClutchUp*/false,
		/*AcButton*/false},
	},
	{
		.boardName = "Proteus",
		.desiredEngineConfig = libPROTEUS_STIM_QC,
		.boardIds = { STATIC_BOARD_ID_PROTEUS_F4, STATIC_BOARD_ID_PROTEUS_F7, STATIC_BOARD_ID_PROTEUS_H7, 0 },
		.channels = {
			{ "TPS1_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ "MAP", 1.0f, 0.6f * ANALOG_L, 0.6f * ANALOG_H_FOR_LOW_VOLTAGE },
			{ "CLT", 1.0f, CLT_VALUE(PROTEUS_R) * ANALOG_L, CLT_VALUE(PROTEUS_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(PROTEUS_R) * ANALOG_L, IAT_VALUE(PROTEUS_R) * ANALOG_H },
			{ "BATT", 9.2f, 9.0f, 15.0f },
		},
		.eventExpected = {true, true, true, true, false, false, true},
		.buttonExpected = {true, false, false},
	},
	{
		.boardName = "2chan",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_ALPHAX_2CHAN, BOARD_ID_ALPHA2CH_B, BOARD_ID_ALPHA2CH_C, BOARD_ID_ALPHA2CH_D,
			BOARD_ID_ALPHA2CH_E, BOARD_ID_ALPHA2CH_F, BOARD_ID_ALPHA2CH_G, 0 },
		.channels = {
			{ "TPS1_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ "MAP", 1.0f, MAP_MPX6400_VALUE * ANALOG_L, MAP_MPX6400_VALUE * ANALOG_H },	// internal MAP
			{ "CLT", 1.0f, CLT_VALUE(ALPHA2CH_R) * ANALOG_L, CLT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(ALPHA2CH_R) * ANALOG_L, IAT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "BATT", 5.835, 9.0f, 15.0f },
		},
		.eventExpected = {true, true, true, true, true, true, true},
		.buttonExpected = {true, true, true},
	},
	{
		.boardName = "4chan",
		.desiredEngineConfig = libHELLEN_4CHAN_STIM_QC,
		.boardIds = { BOARD_ID_ALPHA4CH_H, BOARD_ID_ALPHA4CH_G, 0 },
		.channels = {
			{ "TPS1_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ "MAP", 1.0f, MAP_MPX6400_VALUE * ANALOG_L, MAP_MPX6400_VALUE * ANALOG_H },	// internal MAP
			{ "CLT", 1.0f, CLT_VALUE(ALPHA2CH_R) * ANALOG_L, CLT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(ALPHA2CH_R) * ANALOG_L, IAT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "BATT", 5.835, 9.0f, 15.0f },
		},
		.eventExpected = {true, true, true, true, true, true, false},
		.buttonExpected = {false, false, false},
	},
	{
		.boardName = "154HYUNDAI",
		.desiredEngineConfig = -1,
		.boardIds = { BOARD_ID_154HYUNDAI_C, BOARD_ID_154HYUNDAI_D, 0 },
		.channels = {
			{ "TPS1_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ "TPS1_2", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ "PPS1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ "PPS2", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ "MAP", 1.0f, MAP_MPX6400_VALUE * ANALOG_L, MAP_MPX6400_VALUE * ANALOG_H },	// internal MAP
			{ "CLT", 1.0f, CLT_VALUE(ALPHA2CH_R) * ANALOG_L, CLT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(ALPHA2CH_R) * ANALOG_L, IAT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "BATT", 5.835, 9.0f, 15.0f },
		},
		.eventExpected = {true, true, true, true, true, true, true},
		.buttonExpected = {true, true, true},
	},
};

BoardConfig *currentBoard = nullptr;
int16_t currentBoardRev = -1;

BoardConfig *getBoardConfigs() {
    return boardConfigs;
}

size_t getBoardsCount() {
    return efi::size(boardConfigs);
}

bool testEcuDigitalOutput(int testLineIndex, bool isLowSide) {
	memset(haveSeenLow, 0, sizeof(haveSeenLow));
	memset(haveSeenHigh, 0, sizeof(haveSeenHigh));

	setOutputAddrIndex(testLineIndex % 16);
	int adcIndex = testLineIndex / 16;

	bool isGood = true;

	for (int i = 0; i < cycleCount
	 //&& isGood
	 ; i++) {
		bool isSet = (i & 1) == 0;
		chprintf(chp, "               sending line=%d value=%d\r\n", index2human(testLineIndex), isSet);
		// toggle the ECU pin for low side mode
		sendCanPinState(testLineIndex, isSet ^ isLowSide);

		int scenarioIndex = 1; // i % 2;
		setScenarioIndex(scenarioIndex);
		// wait for the pin to toggle
		chThdSleepMilliseconds(cycleDurationMs);

		float voltage = getAdcValue(adcIndex);
		// low side sends roughly 2.8 but 5v high side is closer to 1v
		bool isHigh = voltage > 0.7;
		if (isHigh) {
			if (!haveSeenHigh[testLineIndex]) {
				chprintf(chp, "                      ADC says HIGH %d@%d %1.3fv\r\n", index2human(testLineIndex), i, voltage);
			}
			haveSeenHigh[testLineIndex] = true;
		} else {
			if (!haveSeenLow[testLineIndex]) {
				chprintf(chp, "                      ADC says LOW %d@%d %1.3fv\r\n", index2human(testLineIndex), i, voltage);
			}
			haveSeenLow[testLineIndex] = true;
		}

		// chprintf(chp, "scenario=%d: %1.3f V\r\n", scenarioIndex, voltage);

		bool cycleIsGood = (isHigh == isSet);
		if (!cycleIsGood) {
		    setRedText();
			chprintf(chp, "ERROR! Cycle %d@%d FAILED! (set %d, received %d %1.3fv)\r\n", 
				index2human(testLineIndex), i, (isSet ? 1 : 0), (isHigh ? 1 : 0), voltage);
			setNormalText();
		}
		isGood = isGood && cycleIsGood;
	}

	// test is successful if we saw state toggle
	return isGood;
}

size_t totalStepsNumber() {
    return getDigitalInputStepsCount() + getDigitalOutputStepsCount();
}
