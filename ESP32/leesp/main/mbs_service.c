#include "mbs_service.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_netif.h"
#include "esp_eth.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_event_loop.h"
#include "../components/cus_freemodbus/common/include/mbcontroller.h"
#include "modbus_params.h" // for modbus parameters structures

#define TAG "LJH_MBS"

#define CONFIG_MB_UART_RXD 22
#define CONFIG_MB_UART_TXD 23
#define CONFIG_MB_UART_RTS 18

#define MB_PORT_NUM (UART_NUM_2) // Number of UART port used for Modbus connection
#define MB_SLAVE_ADDR (1)		 // The address of device in Modbus network
#define MB_DEV_SPEED (115200)	 // The communication speed of the UART

// Note: Some pins on target chip cannot be assigned for UART communication.
// Please refer to documentation for selected board and target to configure pins using Kconfig.

// Defines below are used to define register start address for each type of Modbus registers
#define MB_REG_DISCRETE_INPUT_START (0x0000)
#define MB_REG_INPUT_START (0x0000)
#define MB_REG_HOLDING_START (0x0000)
#define MB_REG_COILS_START (0x0000)

#define MB_PAR_INFO_GET_TOUT (10) // Timeout for get parameter info
#define MB_CHAN_DATA_MAX_VAL (6)
#define MB_CHAN_DATA_OFFSET (0.2f)
#define MB_READ_MASK (MB_EVENT_INPUT_REG_RD | MB_EVENT_HOLDING_REG_RD | MB_EVENT_DISCRETE_RD | MB_EVENT_COILS_RD)

#define MB_WRITE_MASK (MB_EVENT_HOLDING_REG_WR | MB_EVENT_COILS_WR)

#define MB_READ_WRITE_MASK (MB_READ_MASK | MB_WRITE_MASK)

static const char *SLAVE_TAG = "SLAVE_TEST";

static portMUX_TYPE param_lock = portMUX_INITIALIZER_UNLOCKED;

// Set register values into known state
static void setup_reg_data(void)
{
	// Define initial state of parameters
	discrete_reg_params.discrete_input1 = 1;
	discrete_reg_params.discrete_input3 = 1;
	discrete_reg_params.discrete_input5 = 1;
	discrete_reg_params.discrete_input7 = 1;

	holding_reg_params.holding_data0 = 1.34;
	holding_reg_params.holding_data1 = 2.56;
	holding_reg_params.holding_data2 = 3.78;
	holding_reg_params.holding_data3 = 4.90;

	coil_reg_params.coils_port0 = 0x55;
	coil_reg_params.coils_port1 = 0xAA;

	input_reg_params.input_data0 = 1.12;
	input_reg_params.input_data1 = 2.34;
	input_reg_params.input_data2 = 3.56;
	input_reg_params.input_data3 = 4.78;
}

// An example application of Modbus slave. It is based on freemodbus stack.
// See deviceparams.h file for more information about assigned Modbus parameters.
// These parameters can be accessed from main application and also can be changed
// by external Modbus master host.
void mbs_task(void *pvParameter)
{
	mb_param_info_t reg_info;				// keeps the Modbus registers access information
	mb_communication_info_t comm_info;		// Modbus communication parameters
	mb_register_area_descriptor_t reg_area; // Modbus register area descriptor structure

	// Set UART log level
	esp_log_level_set(SLAVE_TAG, ESP_LOG_INFO);
	void *mbc_slave_handler = NULL;

	ESP_ERROR_CHECK(mbc_slave_init(MB_PORT_SERIAL_SLAVE, &mbc_slave_handler)); // Initialization of Modbus controller

	// Setup communication parameters and start stack
#if CONFIG_MB_COMM_MODE_ASCII
	comm_info.mode = MB_MODE_ASCII,
#elif CONFIG_MB_COMM_MODE_RTU
	comm_info.mode = MB_MODE_RTU,
#endif
	comm_info.slave_addr = MB_SLAVE_ADDR;
	comm_info.port = MB_PORT_NUM;
	comm_info.baudrate = MB_DEV_SPEED;
	comm_info.parity = MB_PARITY_NONE;
	ESP_ERROR_CHECK(mbc_slave_setup((void *)&comm_info));

	// The code below initializes Modbus register area descriptors
	// for Modbus Holding Registers, Input Registers, Coils and Discrete Inputs
	// Initialization should be done for each supported Modbus register area according to register map.
	// When external master trying to access the register in the area that is not initialized
	// by mbc_slave_set_descriptor() API call then Modbus stack
	// will send exception response for this register area.
	reg_area.type = MB_PARAM_HOLDING;				// Set type of register area
	reg_area.start_offset = MB_REG_HOLDING_START;	// Offset of register area in Modbus protocol
	reg_area.address = (void *)&holding_reg_params; // Set pointer to storage instance
	reg_area.size = sizeof(holding_reg_params);		// Set the size of register storage instance
	ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

	// Initialization of Input Registers area
	reg_area.type = MB_PARAM_INPUT;
	reg_area.start_offset = MB_REG_INPUT_START;
	reg_area.address = (void *)&input_reg_params;
	reg_area.size = sizeof(input_reg_params);
	ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

	// Initialization of Coils register area
	reg_area.type = MB_PARAM_COIL;
	reg_area.start_offset = MB_REG_COILS_START;
	reg_area.address = (void *)&coil_reg_params;
	reg_area.size = sizeof(coil_reg_params);
	ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

	// Initialization of Discrete Inputs register area
	reg_area.type = MB_PARAM_DISCRETE;
	reg_area.start_offset = MB_REG_DISCRETE_INPUT_START;
	reg_area.address = (void *)&discrete_reg_params;
	reg_area.size = sizeof(discrete_reg_params);
	ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

	setup_reg_data(); // Set values into known state

	// Starts of modbus controller and stack
	ESP_ERROR_CHECK(mbc_slave_start());

	// Set UART pin numbers
	ESP_ERROR_CHECK(uart_set_pin(MB_PORT_NUM, CONFIG_MB_UART_TXD,
								 CONFIG_MB_UART_RXD, CONFIG_MB_UART_RTS,
								 UART_PIN_NO_CHANGE));

	// Set UART driver mode to Half Duplex
	ESP_ERROR_CHECK(uart_set_mode(MB_PORT_NUM, UART_MODE_RS485_HALF_DUPLEX));

	while (1)
	{
		mb_event_group_t event = mbc_slave_check_event(MB_READ_WRITE_MASK);
		const char *rw_str = (event & MB_READ_MASK) ? "READ" : "WRITE";

		if (event & (MB_EVENT_HOLDING_REG_WR | MB_EVENT_HOLDING_REG_RD))
		{

			ESP_ERROR_CHECK(mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
			ESP_LOGI(SLAVE_TAG, "HOLDING %s (%u us), ADDR:%u, TYPE:%u, INST_ADDR:0x%.4x, SIZE:%u",
					 rw_str,
					 (uint32_t)reg_info.time_stamp,
					 (uint32_t)reg_info.mb_offset,
					 (uint32_t)reg_info.type,
					 (uint32_t)reg_info.address,
					 (uint32_t)reg_info.size);
		}
		else if (event & MB_EVENT_INPUT_REG_RD)
		{
			ESP_ERROR_CHECK(mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
			ESP_LOGI(SLAVE_TAG, "INPUT READ (%u us), ADDR:%u, TYPE:%u, INST_ADDR:0x%.4x, SIZE:%u",
					 (uint32_t)reg_info.time_stamp,
					 (uint32_t)reg_info.mb_offset,
					 (uint32_t)reg_info.type,
					 (uint32_t)reg_info.address,
					 (uint32_t)reg_info.size);
		}
		else if (event & MB_EVENT_DISCRETE_RD)
		{
			ESP_ERROR_CHECK(mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
			ESP_LOGI(SLAVE_TAG, "DISCRETE READ (%u us): ADDR:%u, TYPE:%u, INST_ADDR:0x%.4x, SIZE:%u",
					 (uint32_t)reg_info.time_stamp,
					 (uint32_t)reg_info.mb_offset,
					 (uint32_t)reg_info.type,
					 (uint32_t)reg_info.address,
					 (uint32_t)reg_info.size);
		}
		else if (event & (MB_EVENT_COILS_RD | MB_EVENT_COILS_WR))
		{
			ESP_ERROR_CHECK(mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
			ESP_LOGI(SLAVE_TAG, "COILS %s (%u us), ADDR:%u, TYPE:%u, INST_ADDR:0x%.4x, SIZE:%u",
					 rw_str,
					 (uint32_t)reg_info.time_stamp,
					 (uint32_t)reg_info.mb_offset,
					 (uint32_t)reg_info.type,
					 (uint32_t)reg_info.address,
					 (uint32_t)reg_info.size);
		}
		vTaskDelay(1 / portTICK_PERIOD_MS);
	}
	ESP_LOGI(SLAVE_TAG, "Modbus controller destroyed.");
	vTaskDelay(100);
	ESP_ERROR_CHECK(mbc_slave_destroy());
}