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


/**
 * 	Read/Write memory command
 */
typedef union
{
	struct
	{
		uint8_t 	cmd;		/**<Command part of frame */
		uint8_t 	addr[3];	/**<Address part of frame */
	} field;
	uint32_t u;					/**<Unsigned access */
}_25lcxxxx_rw_cmd_t;

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
static _25lcxxxx_status_t 	_25lcxxxx_write_enable 				(void);
static _25lcxxxx_status_t 	_25lcxxxx_write_disable 			(void);
static _25lcxxxx_status_t 	_25lcxxxx_read_status				(_25lcxxxx_status_reg_t * const p_status_reg);
static _25lcxxxx_status_t 	_25lcxxxx_write_status				(const _25lcxxxx_status_reg_t * const p_status_reg);
static _25lcxxxx_status_t 	_25lcxxxx_read_command				(const uint32_t addr);
static _25lcxxxx_status_t 	_25lcxxxx_write_command				(const uint32_t addr);
static void 				_25lcxxxx_assemble_rw_cmd			(_25lcxxxx_rw_cmd_t * const p_frame, const _25lcxxxx_isa_t rw_cmd, const uint32_t addr);
static uint32_t 			_25lcxxxx_calc_num_of_sectors		(const uint32_t addr, const uint32_t size);
static uint32_t 			_25lcxxxx_calc_transfer_size		(const uint32_t addr, const uint32_t size);

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
	_25lcxxxx_status_t 	status				= e25LCXXXX_OK;
	uint32_t			working_addr		= addr;
	uint32_t			working_size		= size;
	uint32_t			bytes_to_transfer	= 0UL;
	uint32_t			data_offset			= 0UL;

	// Check for init
	_25LCXXXX_ASSERT( true == gb_is_init );

	// Invalid inputs
	_25LCXXXX_ASSERT( 0UL == size);
	_25LCXXXX_ASSERT( addr <= _25LCXXXX_MAX_ADDR );

	// Check address boundary
	if (( addr + size ) <= _25LCXXXX_MAX_ADDR )
	{
		// Calculate how many sectors takes write request
		const uint32_t num_of_sectors = _25lcxxxx_calc_num_of_sectors( addr, size );

		// Write to all sectors
		for (uint32_t i = 0; i < num_of_sectors; i++ )
		{
			bytes_to_transfer = _25lcxxxx_calc_transfer_size( working_addr, working_size );

			// Send write command
			status = _25lcxxxx_write_command( working_addr );

			// Send data payload
			status |= _25lcxxxx_if_transmit( ( p_data + data_offset ), bytes_to_transfer, eSPI_CS_HIGH_ON_EXIT );

			// Increment address & written data offset
			data_offset += bytes_to_transfer;
			working_addr += bytes_to_transfer;
			working_size -= bytes_to_transfer;
		}

		// All bytes shall be tranfered
		_25LCXXXX_ASSERT( 0UL == working_size );
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

	// Invalid inputs
	_25LCXXXX_ASSERT( 0UL == size);
	_25LCXXXX_ASSERT( addr <= _25LCXXXX_MAX_ADDR );

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

static uint32_t _25lcxxxx_calc_num_of_sectors(const uint32_t addr, const uint32_t size)
{
	uint32_t sector_num = 0UL;
	uint32_t k_a 		= 0UL;
	uint32_t k_s		= 0UL;

	// Calculate address & size factors of sector size
	k_a = (uint32_t) ( addr / _25LCXXXX_CFG_PAGE_SIZE_BYTE );
	k_s = (uint32_t) (( addr + size ) / _25LCXXXX_CFG_PAGE_SIZE_BYTE );

	// Based on address & size factors number of sectors can be calculated
	sector_num = ( k_s - k_a ) + 1UL;

	return sector_num;
}

static uint32_t _25lcxxxx_calc_transfer_size(const uint32_t addr, const uint32_t size)
{
	uint32_t bytes_to_transfer 	= 0UL;
	uint32_t bytes_in_sector 	= 0UL;

	// Calculate how many bytes till page boundary
	bytes_in_sector = _25LCXXXX_CFG_PAGE_SIZE_BYTE - ( addr % _25LCXXXX_CFG_PAGE_SIZE_BYTE );

	// Bytes will not pass page boundary
	if ( size < bytes_in_sector )
	{
		bytes_to_transfer = size;
	}

	// Bytes will pass page boundary - limit to boundary
	else
	{
		bytes_to_transfer = size;
	}

	return bytes_to_transfer;
}




static void _25lcxxxx_assemble_rw_cmd(_25lcxxxx_rw_cmd_t * const p_frame, const _25lcxxxx_isa_t rw_cmd, const uint32_t addr)
{
	_25LCXXXX_ASSERT( NULL != p_frame );

	p_frame->u 				= 0UL;
	p_frame->field.cmd 		= rw_cmd;
	p_frame->field.addr[0]	= (( addr << 16U ) 	& 0xFFU );
	p_frame->field.addr[1]	= (( addr << 8U ) 	& 0xFFU );
	p_frame->field.addr[2]	= ( addr 			& 0xFFU );
}


static _25lcxxxx_status_t _25lcxxxx_write_command(const uint32_t addr)
{
	_25lcxxxx_status_t 	status 	= e25LCXXXX_OK;
	_25lcxxxx_rw_cmd_t	cmd		= { .u = 0 };

	// Assemble command
	_25lcxxxx_assemble_rw_cmd( &cmd, e25LCXXXX_ISA_WRITE, addr );

	// Send command
	// NOTE: Based on number of address bits command is being divided!
	#if ( _25LCXXXX_CFG_ADDR_BIT_NUM <= 8)
		status = _25lcxxxx_if_transmit((uint8_t*) cmd.u, 2, eSPI_CS_LOW_ON_ENTRY );

	#elif ( 9 == _25LCXXXX_CFG_ADDR_BIT_NUM )

		// 9 bit address specialty
		if (( addr & 0x100U ) == 0x100U )
		{
			cmd.field.cmd |= ( 0x80U );
		}
		else
		{
			cmd.field.cmd &= ~( 0x80U );
		}

		status = _25lcxxxx_if_transmit((uint8_t*) cmd.u, 2, eSPI_CS_LOW_ON_ENTRY );

	#elif ( _25LCXXXX_CFG_ADDR_BIT_NUM <= 16 )
		status = _25lcxxxx_if_transmit((uint8_t*) cmd.u, 3, eSPI_CS_LOW_ON_ENTRY );

	#elif ( _25LCXXXX_CFG_ADDR_BIT_NUM <= 24 )
		status = _25lcxxxx_if_transmit((uint8_t*) cmd.u, 4, eSPI_CS_LOW_ON_ENTRY );

	#endif

	return status;
}



static _25lcxxxx_status_t _25lcxxxx_read_command(const uint32_t addr)
{
	_25lcxxxx_status_t 		status 	= e25LCXXXX_OK;
	_25lcxxxx_rw_cmd_t		cmd		= { .u = 0 };

	// Assemble command
	_25lcxxxx_assemble_rw_cmd( &cmd, e25LCXXXX_ISA_READ, addr );

	// Send command
	// NOTE: Based on number of address bits command is being divided!
	#if ( _25LCXXXX_CFG_ADDR_BIT_NUM <= 8)
		status = _25lcxxxx_if_transmit((uint8_t*) cmd.u, 2, eSPI_CS_LOW_ON_ENTRY );

	#elif ( 9 == _25LCXXXX_CFG_ADDR_BIT_NUM )

		// 9 bit address specialty
		if (( addr & 0x100U ) == 0x100U )
		{
			cmd.field.cmd |= ( 0x80U );
		}
		else
		{
			cmd.field.cmd &= ~( 0x80U );
		}

		status = _25lcxxxx_if_transmit((uint8_t*) cmd.u, 2, eSPI_CS_LOW_ON_ENTRY );

	#elif ( _25LCXXXX_CFG_ADDR_BIT_NUM <= 16 )
		status = _25lcxxxx_if_transmit((uint8_t*) cmd.u, 3, eSPI_CS_LOW_ON_ENTRY );

	#elif ( _25LCXXXX_CFG_ADDR_BIT_NUM <= 24 )
		status = _25lcxxxx_if_transmit((uint8_t*) cmd.u, 4, eSPI_CS_LOW_ON_ENTRY );

	#endif

	return status;
}




// TODO: Remove only testing

static _25lcxxxx_status_reg_t g_status_reg = { .u = 0 };

void _25lcxxxx_hndl_test(void)
{
	static uint32_t cnt = 0;

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
