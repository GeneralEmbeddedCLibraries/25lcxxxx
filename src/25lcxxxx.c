////////////////////////////////////////////////////////////////////////////////
/**
*@file      25lcxxxx.c
*@brief     API for 25LCxxxx EEPROM device
*@author    Ziga Miklosic
*@date      08.05.2021
*@version	V1.0.0
*/
////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup 25LCXXXX_API
* @{ <!-- BEGIN GROUP -->
*
* 	API functions for EEPROM device
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "25lcxxxx.h"
#include "25lcxxxx_regdef.h"
#include "../../25lcxxxx_if.h"


////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////
static _25lcxxxx_status_t _25lcxxxx_write_enable 	(void);
static _25lcxxxx_status_t _25lcxxxx_read_status		();

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*		Initialize EEPROM device
*
* @return 	status - Status of initialization
*/
////////////////////////////////////////////////////////////////////////////////
_25lcxxxx_status_t _25lcxxxx_init(void)
{
	_25lcxxxx_status_t status = e25LCXXXX_OK;

	// Initialize app interface
	status = _25lcxxxx_if_init();

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Write byte(s) to EEPROM
*
* @param[in]	addr	- Start address of write
* @param[in]	size	- Size of bytes to write
* @param[in]	p_data	- Pointer to write data
* @return 		status 	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
_25lcxxxx_status_t _25lcxxxx_write(const uint32_t addr, const uint32_t size, const uint8_t * const p_data)
{
	_25lcxxxx_status_t status = e25LCXXXX_OK;


	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Read byte(s) from EEPROM
*
* @param[in]	addr	- Start address of write
* @param[in]	size	- Size of bytes to write
* @param[out]	p_data	- Pointer to read data
* @return 		status 	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
_25lcxxxx_status_t _25lcxxxx_read(const uint32_t addr, const uint32_t size, uint8_t * const p_data)
{
	_25lcxxxx_status_t status = e25LCXXXX_OK;


	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup 25LCxxxx/25AAxxx
* @{ <!-- BEGIN GROUP -->
*
* 	Kernel functions of SPI EEPROM device 25LCxxxx/25AAxxxx
*/
////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
