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

/**
 *  Highest memory address
 */
#define _25LCXXXX_MAX_ADDR		((uint32_t) (( 1UL << _25LCXXXX_CFG_ADDR_BIT_NUM ) - 1UL ))


////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////
static _25lcxxxx_status_t _25lcxxxx_write_enable 		(void);
static _25lcxxxx_status_t _25lcxxxx_write_disable 		(void);
static _25lcxxxx_status_t _25lcxxxx_read_status			(_25lcxxxx_status_reg_t * const p_status_reg);
static _25lcxxxx_status_t _25lcxxxx_write_status		(const _25lcxxxx_status_reg_t * const p_status_reg);

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


static _25lcxxxx_status_t _25lcxxxx_write_enable(void)
{
	_25lcxxxx_status_t 	status 	= e25LCXXXX_OK;
	const uint8_t		cmd		= e25LCXXXX_ISA_WREN;

	status = _25lcxxxx_if_transmit( &cmd, 1, ( eSPI_CS_HIGH_ON_EXIT | eSPI_CS_LOW_ON_ENTRY ));

	return status;
}


static _25lcxxxx_status_t _25lcxxxx_write_disable(void)
{
	_25lcxxxx_status_t 	status 	= e25LCXXXX_OK;
	const uint8_t		cmd		= e25LCXXXX_ISA_WRDI;

	status = _25lcxxxx_if_transmit( &cmd, 1, ( eSPI_CS_HIGH_ON_EXIT | eSPI_CS_LOW_ON_ENTRY ));

	return status;
}


static _25lcxxxx_status_t _25lcxxxx_read_status(_25lcxxxx_status_reg_t * const p_status_reg)
{
	_25lcxxxx_status_t 	status 	= e25LCXXXX_OK;
	const uint8_t		cmd		= e25LCXXXX_ISA_RDST;

	status = _25lcxxxx_if_transmit( &cmd, 1, eSPI_CS_LOW_ON_ENTRY );
	status |= _25lcxxxx_if_receive((uint8_t*) p_status_reg, 1, eSPI_CS_HIGH_ON_EXIT );

	return status;
}

static _25lcxxxx_status_t _25lcxxxx_write_status(const _25lcxxxx_status_reg_t * const p_status_reg)
{
	_25lcxxxx_status_t 	status 	= e25LCXXXX_OK;
	const uint8_t		cmd		= e25LCXXXX_ISA_WDST;

	status = _25lcxxxx_if_transmit( &cmd, 1, eSPI_CS_LOW_ON_ENTRY );
	status |= _25lcxxxx_if_transmit((uint8_t*) p_status_reg, 1, eSPI_CS_HIGH_ON_EXIT );

	return status;
}




// TODO: Remove only testing

static _25lcxxxx_status_reg_t g_status_reg = { .u = 0 };

void _25lcxxxx_hndl_test(void)
{
	static cnt = 0;

	switch (cnt)
	{
		case 0:
			cnt++;

			_25lcxxxx_read_status( &g_status_reg );

			break;

		case 1:
			cnt++;

			_25lcxxxx_write_enable();
			break;

		case 2:
			cnt++;

			_25lcxxxx_read_status( &g_status_reg );

			break;

		case 3:
			cnt++;

			_25lcxxxx_write_disable();
			break;

		case 4:
			cnt ++;

			_25lcxxxx_read_status( &g_status_reg );

			break;

		case 5:
			cnt = 0;
			break;

		default:
			break;
	}


}


////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
