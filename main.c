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

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "bsp.h"
#include "spidrv.h"

SPIDRV_HandleData_t handleData;
SPIDRV_Handle_t handle = &handleData;

void write(int count, void *buffer){
	Ecode_t errno = SPIDRV_MTransmitB(handle, buffer, count);
}
void read(int count, void *buffer){
	Ecode_t errno = SPIDRV_MReceiveB(handle, buffer, count);
}
void
void SPI_Init(){
	SPIDRV_Init_t initData = SPIDRV_MASTER_USART2;
	SPIDRV_Init(handle, &initData);
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

	/* Chip errata */
	CHIP_Init();

	printf("Starting\n");

	while (1) {
	 printf("Hello from soc-empty world!\n");
	}

}


