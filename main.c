/***************************************************************************//**
 * @file
 * @brief Silicon Labs Empty Example Project
 *
 * This example demonstrates the bare minimum needed for a Blue Gecko C application
 * that allows Over-the-Air Device Firmware Upgrading (OTA DFU). The application
 * starts advertising after boot and restarts advertising after a connection is closed.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

/* Board headers */
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"

/* Device initialization header */
#include "hal-config.h"

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif

/* Application header */
#include "app.h"

#include <stdio.h>
#include "retargetswo.h"
#include <BMA400.h>
#include <i2cspm.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "bsp.h"
/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

#ifndef MAX_ADVERTISERS
#define MAX_ADVERTISERS 1
#endif

#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4
#endif

uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

/* Bluetooth stack configuration parameters (see "UG136: Silicon Labs Bluetooth C Application Developer's Guide" for details on each parameter) */
static gecko_configuration_t config = { .config_flags = 0, /* Check flag options from UG136 */
#if defined(FEATURE_LFXO) || defined(PLFRCO_PRESENT) || defined(LFRCO_PRESENT)
		.sleep.flags = SLEEP_FLAGS_DEEP_SLEEP_ENABLE, /* Sleep is enabled */
#else
		.sleep.flags = 0,
#endif
		.bluetooth.max_connections = MAX_CONNECTIONS, /* Maximum number of simultaneous connections */
		.bluetooth.max_advertisers = MAX_ADVERTISERS, /* Maximum number of advertisement sets */
		.bluetooth.heap = bluetooth_stack_heap, /* Bluetooth stack memory for connection management */
		.bluetooth.heap_size = sizeof(bluetooth_stack_heap), /* Bluetooth stack memory for connection management */
#if defined(FEATURE_LFXO)
		.bluetooth.sleep_clock_accuracy = 100, /* Accuracy of the Low Frequency Crystal Oscillator in ppm. *
		 * Do not modify if you are using a module                  */
#elif defined(PLFRCO_PRESENT) || defined(LFRCO_PRESENT)
		.bluetooth.sleep_clock_accuracy = 500, /* In case of internal RCO the sleep clock accuracy is 500 ppm */
#endif
		.gattdb = &bg_gattdb_data, /* Pointer to GATT database */
		.ota.flags = 0, /* Check flag options from UG136 */
		.ota.device_name_len = 3, /* Length of the device name in OTA DFU mode */
		.ota.device_name_ptr = "OTA", /* Device name in OTA DFU mode */
		.pa.config_enable = 1, /* Set this to be a valid PA config */
#if defined(FEATURE_PA_INPUT_FROM_VBAT)
		.pa.input = GECKO_RADIO_PA_INPUT_VBAT, /* Configure PA input to VBAT */
#else
		.pa.input = GECKO_RADIO_PA_INPUT_DCDC, /* Configure PA input to DCDC */
#endif // defined(FEATURE_PA_INPUT_FROM_VBAT)
		.rf.flags = GECKO_RF_CONFIG_ANTENNA, /* Enable antenna configuration. */
		.rf.antenna = GECKO_RF_ANTENNA, /* Select antenna path! */
};

/**
 * @brief  Main function
 */

static I2C_TransferReturn_TypeDef i2cReadByte(I2C_TypeDef *i2c, uint16_t addr,
		uint8_t command, uint8_t *val) {
	I2C_TransferSeq_TypeDef seq;
	I2C_TransferReturn_TypeDef sta;
	uint8_t i2c_write_data[1];
	uint8_t i2c_read_data[1];

	seq.addr = addr;
	seq.flags = I2C_FLAG_WRITE_READ;
	/* Select command to issue */
	i2c_write_data[0] = command;
	seq.buf[0].data = i2c_write_data;
	seq.buf[0].len = 1;
	/* Select location/length of data to be read */
	seq.buf[1].data = i2c_read_data;
	seq.buf[1].len = 1;

	sta = I2CSPM_Transfer(i2c, &seq);
	if (sta != i2cTransferDone) {
		return sta;
	}
	if (NULL != val) {
		*val = i2c_read_data[0];
	}
	return sta;
}
int main(void) {

	/* Initialize device */
	initMcu();
	/* Initialize board */
	initBoard();
	/* Initialize application */
	initApp();
	initVcomEnable();
	RETARGET_SwoInit();

	I2C_TransferReturn_TypeDef sta;

	EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_DEFAULT;
	CMU_HFXOInit_TypeDef hfxoInit = CMU_HFXOINIT_DEFAULT;

	/* Chip errata */
	CHIP_Init();

	/* Init DCDC regulator and HFXO with kit specific parameters */
	//EMU_DCDCInit(&dcdcInit);
	//CMU_HFXOInit(&hfxoInit);

	/* Switch HFCLK to HFXO and disable HFRCO */
	//CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
	//CMU_OscillatorEnable(cmuOsc_HFRCO, false, false);

	// Enable GPIO clock source
	//CMU_ClockEnable(cmuClock_GPIO, true);

	/* Setup SysTick Timer for 1 msec interrupts  */
	//if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) {
	//	while (1)
			;
//	}

	printf("Starting\n");

	UDELAY_Calibrate();
	I2CSPM_Init_TypeDef i2cInit = I2CSPM_INIT_DEFAULT;
	I2CSPM_Init(&i2cInit);

	uint8_t Ascale = AFS_2G,
	SR = SR_200Hz, power_Mode = lowpower_Mode, OSR = osr0, acc_filter =
			acc_filt2;
	float offset[3];        // accel bias offsets
	uint8_t c = getChipID();  // Read CHIP_ID register for BMA400
	s_BMA400 BMA400_ = BMA400(0, 0);
	float aRes = getAres(&BMA400_, Ascale);
	//int c = 0x90;
	if (c == 0x90) { // check if all I2C sensors with WHO_AM_I have acknowledged

		printf("BMA400 is online...!\n");
		resetBMA400();                   // software reset before initialization
		delay(100);
		selfTestBMA400();                            // perform sensor self test
		resetBMA400();                   // software reset before initialization
		delay(1000);                        // give some time to read the screen
		CompensationBMA400(&BMA400_, Ascale, SR, normal_Mode, OSR, acc_filter,
				offset); // quickly estimate offset bias in normal mode
		initBMA400(Ascale, SR, power_Mode, OSR, acc_filter); // Initialize sensor in desired mode for application

	} else {
		if (c != 0x90)
			printf(" BMA400 not functioning %f!\n", c);
	}

	/*while (1) {
	 printf("Hello from soc-empty world!\n");
	 }*/
	/* Start application */
	// appMain(&config);
}

/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */
