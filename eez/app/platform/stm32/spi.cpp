/*
 * EEZ PSU Firmware
 * Copyright (C) 2015-present, Envox d.o.o.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "eez/app/psu.h"
#include "spi.h"

namespace eez {
namespace app {

bool g_spiInitialized;
SPI_HandleTypeDef MCP23S08_SPI;
SPI_HandleTypeDef DAC8552_SPI;
SPI_HandleTypeDef ADS1120_SPI;
SPI_HandleTypeDef *g_activeSpiHandle;
SPIClass SPI;

void initSPI() {
	if (!g_spiInitialized) {
		DAC8552_SPI.Instance = SPI4;
		DAC8552_SPI.Init.Mode = SPI_MODE_MASTER;
		DAC8552_SPI.Init.Direction = SPI_DIRECTION_2LINES;
		DAC8552_SPI.Init.DataSize = SPI_DATASIZE_8BIT;
		DAC8552_SPI.Init.CLKPolarity = SPI_POLARITY_LOW;
		DAC8552_SPI.Init.CLKPhase = SPI_PHASE_2EDGE;
		DAC8552_SPI.Init.NSS = SPI_NSS_SOFT;
		DAC8552_SPI.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
		DAC8552_SPI.Init.FirstBit = SPI_FIRSTBIT_MSB;
		DAC8552_SPI.Init.TIMode = SPI_TIMODE_DISABLE;
		DAC8552_SPI.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
		DAC8552_SPI.Init.CRCPolynomial = 10;

		ADS1120_SPI.Instance = SPI4;
		ADS1120_SPI.Init.Mode = SPI_MODE_MASTER;
		ADS1120_SPI.Init.Direction = SPI_DIRECTION_2LINES;
		ADS1120_SPI.Init.DataSize = SPI_DATASIZE_8BIT;
		ADS1120_SPI.Init.CLKPolarity = SPI_POLARITY_LOW;
		ADS1120_SPI.Init.CLKPhase = SPI_PHASE_2EDGE;
		ADS1120_SPI.Init.NSS = SPI_NSS_SOFT;
		ADS1120_SPI.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
		ADS1120_SPI.Init.FirstBit = SPI_FIRSTBIT_MSB;
		ADS1120_SPI.Init.TIMode = SPI_TIMODE_DISABLE;
		ADS1120_SPI.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
		ADS1120_SPI.Init.CRCPolynomial = 10;

		MCP23S08_SPI.Instance = SPI4;
		MCP23S08_SPI.Init.Mode = SPI_MODE_MASTER;
		MCP23S08_SPI.Init.Direction = SPI_DIRECTION_2LINES;
		MCP23S08_SPI.Init.DataSize = SPI_DATASIZE_8BIT;
		MCP23S08_SPI.Init.CLKPolarity = SPI_POLARITY_LOW;
		MCP23S08_SPI.Init.CLKPhase = SPI_PHASE_1EDGE;
		MCP23S08_SPI.Init.NSS = SPI_NSS_SOFT;
		MCP23S08_SPI.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
		MCP23S08_SPI.Init.FirstBit = SPI_FIRSTBIT_MSB;
		MCP23S08_SPI.Init.TIMode = SPI_TIMODE_DISABLE;
		MCP23S08_SPI.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
		MCP23S08_SPI.Init.CRCPolynomial = 10;

		HAL_GPIO_WritePin(IOEXP_GPIO_Port, IOEXP_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(ADC_GPIO_Port, ADC_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(DAC_GPIO_Port, DAC_Pin, GPIO_PIN_SET);

		g_spiInitialized = true;
	}
}

void SPI_beginTransaction(SPI_HandleTypeDef& spiHandle) {
	initSPI();

	g_activeSpiHandle = &spiHandle;

	if (HAL_SPI_Init(g_activeSpiHandle) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

//	const SPI_InitTypeDef& init = g_activeSpiHandle->Init;
//	g_activeSpiHandle->State = HAL_SPI_STATE_BUSY;
//	__HAL_SPI_DISABLE(g_activeSpiHandle);
//	WRITE_REG(g_activeSpiHandle->Instance->CR1, (
//		init.Mode |
//		init.Direction |
//		init.DataSize |
//		init.CLKPolarity |
//		init.CLKPhase |
//		(init.NSS & SPI_CR1_SSM) |
//		init.BaudRatePrescaler |
//		init.FirstBit |
//		init.CRCCalculation
//	));
//	WRITE_REG(g_activeSpiHandle->Instance->CR2, (((init.NSS >> 16U) & SPI_CR2_SSOE) | init.TIMode));
//	CLEAR_BIT(g_activeSpiHandle->Instance->I2SCFGR, SPI_I2SCFGR_I2SMOD);
//	g_activeSpiHandle->State = HAL_SPI_STATE_READY;
}

void SPI_endTransaction() {
	g_activeSpiHandle = NULL;
}

void digitalWrite(int pin, int state) {
	if (pin == IO_EXPANDER1 || pin == IO_EXPANDER2) {
		initSPI();
		HAL_GPIO_WritePin(IOEXP_GPIO_Port, IOEXP_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
	} else if (pin == ADC1_SELECT || pin == ADC2_SELECT) {
		initSPI();
		HAL_GPIO_WritePin(ADC_GPIO_Port, ADC_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
	} else if (pin == DAC1_SELECT || pin == DAC2_SELECT) {
		initSPI();
		HAL_GPIO_WritePin(DAC_GPIO_Port, DAC_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
}

uint8_t SPIClass::transfer(uint8_t value) {
	uint8_t result;
	HAL_SPI_TransmitReceive(g_activeSpiHandle, &value, &result, 1, HAL_MAX_DELAY);
	return result;
}

}
} // namespace eez::app


