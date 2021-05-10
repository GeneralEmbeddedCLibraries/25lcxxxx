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
#include "../../25lcxxxx_cfg.h"


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

/**
 * 	Initialization guard
 */
static bool gb_is_init = false;

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

	// Enable write latch
	status |= _25lcxxxx_write_enable();

	if ( e25LCXXXX_OK == status )
	{
		gb_is_init = true;
	}

	_25LCXXXX_ASSERT( e25LCXXXX_OK == status );

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
	_25lcxxxx_status_t 	status			= e25LCXXXX_OK;
	uint32_t			working_addr	= 0UL;

	// Check for init
	_25LCXXXX_ASSERT( true == gb_is_init );

	// Invalid input
	_25LCXXXX_ASSERT( 0UL == size);

	// Check address boundary
	if (( addr + size ) <= _25LCXXXX_MAX_ADDR )
	{
		// Calculate how many sectors takes write request
		const uint32_t sector_num = _25lcxxxx_get_number_of_sectors( addr, size );

		// Write to all sectors
		for ( i = 0; i < sector_num; i++ )
		{
			// Calculate how many bytes till page boundary
			//bytes_till_boundary = 0;

			// Calculate new address
			_25lcxxxx_write_command( working_addr );

			_25lcxxxx_if_transmit( p_data, bytes_till_boundary, eSPI_CS_HIGH_ON_EXIT );

		}
	}
	else
	{
		status = e25LCXXXX_ERROR_ADDR;
	}

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

	// Check for init
	_25LCXXXX_ASSERT( true == gb_is_init );

	// Invalid input
	_25LCXXXX_ASSERT( 0UL == size);

	// Check address boundary
	if (( addr + size ) <= _25LCXXXX_MAX_ADDR )
	{

	}
	else
	{
		status = e25LCXXXX_ERROR_ADDR;
	}


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
	_25lcxxxx_status_t 		status 	= e25LCXXXX_OK;
	const _25lcxxxx_isa_t	cmd		= e25LCXXXX_ISA_WREN;

	status = _25lcxxxx_if_transmit( &cmd, 1, ( eSPI_CS_HIGH_ON_EXIT | eSPI_CS_LOW_ON_ENTRY ));

	return status;
}


static _25lcxxxx_status_t _25lcxxxx_write_disable(void)
{
	_25lcxxxx_status_t 	status 	= e25LCXXXX_OK;
	const _25lcxxxx_isa_t		cmd		= e25LCXXXX_ISA_WRDI;

	status = _25lcxxxx_if_transmit( &cmd, 1, ( eSPI_CS_HIGH_ON_EXIT | eSPI_CS_LOW_ON_ENTRY ));

	return status;
}


static _25lcxxxx_status_t _25lcxxxx_read_status(_25lcxxxx_status_reg_t * const p_status_reg)
{
	_25lcxxxx_status_t 	status 	= e25LCXXXX_OK;
	const _25lcxxxx_isa_t		cmd		= e25LCXXXX_ISA_RDST;

	status = _25lcxxxx_if_transmit( &cmd, 1, eSPI_CS_LOW_ON_ENTRY );
	status |= _25lcxxxx_if_receive((uint8_t*) p_status_reg, 1, eSPI_CS_HIGH_ON_EXIT );

	return status;
}

static _25lcxxxx_status_t _25lcxxxx_write_status(const _25lcxxxx_status_reg_t * const p_status_reg)
{
	_25lcxxxx_status_t 		status 	= e25LCXXXX_OK;
	const _25lcxxxx_isa_t	cmd		= e25LCXXXX_ISA_WDST;

	status = _25lcxxxx_if_transmit( &cmd, 1, eSPI_CS_LOW_ON_ENTRY );
	status |= _25lcxxxx_if_transmit((uint8_t*) p_status_reg, 1, eSPI_CS_HIGH_ON_EXIT );

	return status;
}

static uint32_t _25lcxxxx_get_number_of_sectors(const uint32_t addr, const uint32_t size)
{
	uint32_t sector_num = 0UL;



	return sector_num;
}


typedef struct
{
	uint8_t 	cmd;
	uint32_t 	addr;
} _25lcxxxx_rw_cmd_t;

static void _25lcxxxx_assemble_rw_cmd(_25lcxxxx_rw_cmd_t * const p_frame, const _25lcxxxx_isa_t rw_cmd, const uint32_t addr)
{
	// Wtch for address lenght here....

	#if ( 8 == _25LCXXXX_CFG_ADDR_BIT_NUM )

	#elif ( 9 == _25LCXXXX_CFG_ADDR_BIT_NUM )

	#elif ( 12 == _25LCXXXX_CFG_ADDR_BIT_NUM )

	#endif
}


static _25lcxxxx_status_t _25lcxxxx_send_write_command(const uint32_t addr)
{
	_25lcxxxx_status_t 		status 	= e25LCXXXX_OK;
	const _25lcxxxx_isa_t	cmd		= e25LCXXXX_ISA_WRITE;


	return status;
}



static _25lcxxxx_status_t _25lcxxxx_send_read_command(const uint32_t addr)
{
	_25lcxxxx_status_t 			status 	= e25LCXXXX_OK;
	_25lcxxxx_rw_cmd_t	cmd		= { 0 };

	// Assemble read/write command

	status = _25lcxxxx_if_transmit( &cmd, size, cs_action)

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
