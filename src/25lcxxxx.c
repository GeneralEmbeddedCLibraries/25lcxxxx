////////////////////////////////////////////////////////////////////////////////
/**
*@file      25lcxxxx.c
*@brief     API for 25LCxxxx EEPROM device
*@author    Ziga Miklosic
*@date      08.05.2021
*@version	V1.0.1
*/
////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup _25LCXXXX_API
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
#define _25LCXXXX_MAX_ADDR					((uint32_t) (( 1UL << _25LCXXXX_CFG_ADDR_BIT_NUM ) - 1UL ))

/**
 * 	Write in progress wait timeout
 *
 * 	Unit: miliseconds
 */
#define _25LCXXXX_WAIT_WRITE_TIMEOUT_MS		( 5UL )

/**
 * 	Erase value
 */
#define _25LCXXX_ERASE_VALUE				((uint8_t)( 0xFFU ))

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
static bool					_25lcxxxx_read_wip_flag				(void);
static bool					_25lcxxxx_read_wel_flag				(void);
static _25lcxxxx_status_t	_25lcxxxx_wait_for_write_process	(const uint32_t timeout);

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
	_25lcxxxx_status_t 	status 		= e25LCXXXX_OK;
	bool				wel_flag	= false;

	// Check for init
	_25LCXXXX_ASSERT( false == gb_is_init );

	// Initialize app interface
	status = _25lcxxxx_if_init();

	// Enable write latch
	status |= _25lcxxxx_write_enable();

	// Read WEL flag
	wel_flag = _25lcxxxx_read_wel_flag();

	if (	( e25LCXXXX_OK == status )
		&& 	( true == wel_flag ))
	{
		gb_is_init = true;
	}

	_25LCXXXX_ASSERT( e25LCXXXX_OK == status );
	_25LCXXXX_ASSERT( true == gb_is_init );

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		De-Initialize EEPROM device
*
* @return 	status - Status of deinitialization
*/
////////////////////////////////////////////////////////////////////////////////
_25lcxxxx_status_t _25lcxxxx_deinit	(void)
{
	_25lcxxxx_status_t 		status 		= e25LCXXXX_OK;
	_25lcxxxx_status_reg_t	stat_reg	= { .u = 0 };

	// Check for init
	_25LCXXXX_ASSERT( true == gb_is_init );

	// Disable write latch
	status |= _25lcxxxx_write_disable();

	// Verify enable write latch
	status |= _25lcxxxx_read_status( &stat_reg );

	if (	( e25LCXXXX_OK == status )
		&& 	( false == stat_reg.b.wel ))
	{
		gb_is_init = false;
	}

	_25LCXXXX_ASSERT( e25LCXXXX_OK == status );
	_25LCXXXX_ASSERT( false == gb_is_init );

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Is device initialized
*
* @return 	gb_is_init - Global initialization flag
*/
////////////////////////////////////////////////////////////////////////////////
const bool _25lcxxxx_is_init(void)
{
	return (const bool) gb_is_init;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Write byte(s) to EEPROM
*
* @note		Need to be check for page boundary unless it wrap around!
*
* @param[in]	addr	- Start address of write
* @param[in]	size	- Size of bytes to write
* @param[in]	p_data	- Pointer to write data
* @return 		status 	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
_25lcxxxx_status_t _25lcxxxx_write(const uint32_t addr, const uint32_t size, const uint8_t * const p_data)
{
	_25lcxxxx_status_t 		status				= e25LCXXXX_OK;
	uint32_t				working_addr		= addr;
	uint32_t				working_size		= size;
	uint32_t				bytes_to_transfer	= 0UL;
	uint32_t				data_offset			= 0UL;

	// Check for init
	_25LCXXXX_ASSERT( true == gb_is_init );

	// Invalid inputs
	_25LCXXXX_ASSERT( size > 0 );
	_25LCXXXX_ASSERT(( addr + size - 1 ) <= _25LCXXXX_MAX_ADDR );

	// Calculate how many sectors takes write request
	const uint32_t num_of_sectors = _25lcxxxx_calc_num_of_sectors( addr, size );

	// Write to all sectors
	for (uint32_t i = 0; i < num_of_sectors; i++ )
	{
		// Calculate bytes to transfer till end of page
		bytes_to_transfer = _25lcxxxx_calc_transfer_size( working_addr, working_size );

		// Send write command
		status = _25lcxxxx_write_command( working_addr );

		if ( e25LCXXXX_OK == status )
		{
			// Send data payload
			status = _25lcxxxx_if_transmit( ( p_data + data_offset ), bytes_to_transfer, eSPI_CS_HIGH_ON_EXIT );

			// Increment address & written data offset
			data_offset += bytes_to_transfer;
			working_addr += bytes_to_transfer;
			working_size -= bytes_to_transfer;
		}
		else
		{
			break;
		}
	}

	// All bytes shall be transfered
	_25LCXXXX_ASSERT( 0UL == working_size );

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Erase byte(s) from EEPROM
*
* @brief	This function erase number of bytes from eeprom device starting
* 			from addr parameter. Erase value is defined by "_25LCXXX_ERASE_VALUE"
* 			macro.
*
*
* @param[in]	addr	- Start address of write
* @param[in]	size	- Size of bytes to write
* @return 		status 	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
_25lcxxxx_status_t _25lcxxxx_erase(const uint32_t addr, const uint32_t size)
{
	_25lcxxxx_status_t 	status		 	= e25LCXXXX_OK;
	uint8_t				erase_data[32] 	= { 0 };

	_25LCXXXX_ASSERT( true == gb_is_init );
	_25LCXXXX_ASSERT( size < 32 );

	// Prepare erase data
	for ( uint8_t i = 0; i < 32; i++ )
	{
		erase_data[i] = _25LCXXX_ERASE_VALUE;
	}

	// Erase memory
	status = _25lcxxxx_write( addr, size, (uint8_t*) erase_data );

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
	_25lcxxxx_status_t 	status 		= e25LCXXXX_OK;
	bool				wip_flag	= true;

	// Check for init
	_25LCXXXX_ASSERT( true == gb_is_init );

	// Invalid inputs
	_25LCXXXX_ASSERT( size > 0 );
	_25LCXXXX_ASSERT(( addr + size - 1 ) <= _25LCXXXX_MAX_ADDR );

	// Get WIP flag
	// NOTE: Reading not possible when write in progress!
	wip_flag = _25lcxxxx_read_wip_flag();

	// Check if write in progress
	if ( false == wip_flag )
	{
		// Send read command
		status = _25lcxxxx_read_command( addr );

		// Send data payload
		status |= _25lcxxxx_if_receive( p_data, size, eSPI_CS_HIGH_ON_EXIT );
	}
	else
	{
		status = e25LCXXXX_ERROR;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set protection
*
* @pre	Chip WP pin must be set to low (0V) in order for protection to be
* 		enabled in first place. If that condition is not fullfiled device
* 		will not be write protected.
*
* @param[in]	prot_opt	- Protection options
* @return 		status 		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
_25lcxxxx_status_t _25lcxxxx_set_protection(const _25lcxxxx_protect_t prot_opt)
{
	_25lcxxxx_status_t 		status 		= e25LCXXXX_OK;
	_25lcxxxx_status_reg_t	stat_reg	= { .u = 0 };

	// Check for init
	_25LCXXXX_ASSERT( true == gb_is_init );

	// Set protection level
	stat_reg.b.bp = prot_opt;

	// Write to device
	status = _25lcxxxx_write_status( &stat_reg );

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup KERNEL_FUNCTIONS
* @{ <!-- BEGIN GROUP -->
*
* 	Kernel functions of SPI EEPROM device 25LCxxxx/25AAxxxx
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*		Enable write latch
*
* @return 		status 	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static _25lcxxxx_status_t _25lcxxxx_write_enable(void)
{
	_25lcxxxx_status_t 		status 	= e25LCXXXX_OK;
	const _25lcxxxx_isa_t	cmd		= e25LCXXXX_ISA_WREN;

	status = _25lcxxxx_if_transmit( &cmd, 1, ( eSPI_CS_HIGH_ON_EXIT | eSPI_CS_LOW_ON_ENTRY ));

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Disable write latch
*
* @return 		status 	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static _25lcxxxx_status_t _25lcxxxx_write_disable(void)
{
	_25lcxxxx_status_t 	status 	= e25LCXXXX_OK;
	const _25lcxxxx_isa_t		cmd		= e25LCXXXX_ISA_WRDI;

	status = _25lcxxxx_if_transmit( &cmd, 1, ( eSPI_CS_HIGH_ON_EXIT | eSPI_CS_LOW_ON_ENTRY ));

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Read status register from device
*
* @param[out]	p_status_reg	- Pointer to status register
* @return 		status 			- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static _25lcxxxx_status_t _25lcxxxx_read_status(_25lcxxxx_status_reg_t * const p_status_reg)
{
	_25lcxxxx_status_t 	status 	= e25LCXXXX_OK;
	const _25lcxxxx_isa_t		cmd		= e25LCXXXX_ISA_RDST;

	status = _25lcxxxx_if_transmit( &cmd, 1, eSPI_CS_LOW_ON_ENTRY );
	status |= _25lcxxxx_if_receive((uint8_t*) p_status_reg, 1, eSPI_CS_HIGH_ON_EXIT );

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Write to device status register
*
* @param[in]	p_status_reg	- Pointer to status register
* @return 		status 			- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static _25lcxxxx_status_t _25lcxxxx_write_status(const _25lcxxxx_status_reg_t * const p_status_reg)
{
	_25lcxxxx_status_t 		status 	= e25LCXXXX_OK;
	const _25lcxxxx_isa_t	cmd		= e25LCXXXX_ISA_WDST;

	status = _25lcxxxx_if_transmit( &cmd, 1, eSPI_CS_LOW_ON_ENTRY );
	status |= _25lcxxxx_if_transmit((uint8_t*) p_status_reg, 1, eSPI_CS_HIGH_ON_EXIT );

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Calculate number of sectors
*
*	This function calculates how many pages consumes area with a
*	start of address and with a size of size argument. It is being
*	used in write function.
*
* @param[in]	addr		- Start address of area
* @param[in]	size		- Size of area
* @return 		sector_num	- Number of sectors in that area
*/
////////////////////////////////////////////////////////////////////////////////
static uint32_t _25lcxxxx_calc_num_of_sectors(const uint32_t addr, const uint32_t size)
{
	uint32_t sector_num = 0UL;
	uint32_t k_a 		= 0UL;
	uint32_t k_s		= 0UL;

	// Calculate address & size factors of sector size
	k_a = (uint32_t) ( addr / _25LCXXXX_CFG_PAGE_SIZE_BYTE );
	k_s = (uint32_t) (( addr + size - 1UL ) / _25LCXXXX_CFG_PAGE_SIZE_BYTE );

	// Based on address & size factors number of sectors can be calculated
	sector_num = ( k_s - k_a ) + 1UL;

	return sector_num;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Calculate transfer size of data in bytes
*
*	This function caclulated based on address and size how many bytes are
*	in current page. In other words what amount of bytes can be written until
*	page boundary is hit. It is being used for write function.
*
* @param[in]	addr	- Start address
* @param[in]	size	- Size of bytes
* @return 		status 	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static uint32_t _25lcxxxx_calc_transfer_size(const uint32_t addr, const uint32_t size)
{
	uint32_t bytes_to_transfer 	= 0UL;
	uint32_t bytes_in_sector 	= 0UL;

	// Calculate how many bytes till page boundary
	bytes_in_sector = _25LCXXXX_CFG_PAGE_SIZE_BYTE - ( addr % _25LCXXXX_CFG_PAGE_SIZE_BYTE );

	// Bytes will not pass page boundary
	if ( size <= bytes_in_sector )
	{
		bytes_to_transfer = size;
	}

	// Bytes will pass page boundary - limit to boundary
	else
	{
		bytes_to_transfer = bytes_in_sector;
	}

	return bytes_to_transfer;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Assemble read/write command
*
* @note For know this function has been tested only on 25LC640 device!
*
* @param[out]	p_frame		- Pointer to cmd frame
* @param[in]	rw_cmd		- Device command for read or write
* @param[in]	addr		- Start address of read or write
* @return 		void
*/
////////////////////////////////////////////////////////////////////////////////
static void _25lcxxxx_assemble_rw_cmd(_25lcxxxx_rw_cmd_t * const p_frame, const _25lcxxxx_isa_t rw_cmd, const uint32_t addr)
{
	_25LCXXXX_ASSERT( NULL != p_frame );

	p_frame->u 				= 0UL;
	p_frame->field.cmd 		= rw_cmd;

	#if ( _25LCXXXX_CFG_ADDR_BIT_NUM < 10 )
		p_frame->field.addr[0]	= ( addr 			& 0xFFU );

	#elif ( _25LCXXXX_CFG_ADDR_BIT_NUM < 16 )
		p_frame->field.addr[0]	= (( addr >> 8U ) 	& 0xFFU );
		p_frame->field.addr[1]	= ( addr 			& 0xFFU );
	#else
		p_frame->field.addr[0]	= (( addr >> 16U ) 	& 0xFFU );
		p_frame->field.addr[1]	= (( addr >> 8U ) 	& 0xFFU );
		p_frame->field.addr[2]	= ( addr 			& 0xFFU );

	#endif
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Send write command to device
*
*	This function first check if writing to device is allowed by looking into
*	Write-In-Progress (WIP) flag in status register. Only when this flag is
*	cleared in less that "_25LCXXXX_WAIT_WRITE_TIMEOUT_MS" command for
*	write is being send.
*
* @param[in]	addr		- Start address of write transfer
* @return 		status 		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static _25lcxxxx_status_t _25lcxxxx_write_command(const uint32_t addr)
{
	_25lcxxxx_status_t 	status 	= e25LCXXXX_OK;
	_25lcxxxx_rw_cmd_t	cmd		= { .u = 0 };

	// Wait for previous write process
	status = _25lcxxxx_wait_for_write_process( _25LCXXXX_WAIT_WRITE_TIMEOUT_MS );

	if ( e25LCXXXX_OK == status )
	{
		// Enable write enable latch
		_25lcxxxx_write_enable();

		// Assemble command
		_25lcxxxx_assemble_rw_cmd( &cmd, e25LCXXXX_ISA_WRITE, addr );

		// Send command
		// NOTE: Based on number of address bits command is being divided!
		#if ( _25LCXXXX_CFG_ADDR_BIT_NUM <= 8)
			status = _25lcxxxx_if_transmit((uint8_t*) &cmd.u, 2, eSPI_CS_LOW_ON_ENTRY );

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

			status = _25lcxxxx_if_transmit((uint8_t*) &cmd.u, 2, eSPI_CS_LOW_ON_ENTRY );

		#elif ( _25LCXXXX_CFG_ADDR_BIT_NUM <= 16 )
			status = _25lcxxxx_if_transmit((uint8_t*) &cmd.u, 3, eSPI_CS_LOW_ON_ENTRY );

		#elif ( _25LCXXXX_CFG_ADDR_BIT_NUM <= 24 )
			status = _25lcxxxx_if_transmit((uint8_t*) &cmd.u, 4, eSPI_CS_LOW_ON_ENTRY );

		#endif
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Send read command to device
*
* 	@note	Before reading from device it is important to check for
* 			Write-In-Progress (WIP) flag in status register.
*
* @param[in]	addr		- Start address of write transfer
* @return 		status 		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static _25lcxxxx_status_t _25lcxxxx_read_command(const uint32_t addr)
{
	_25lcxxxx_status_t 		status 	= e25LCXXXX_OK;
	_25lcxxxx_rw_cmd_t		cmd		= { .u = 0 };

	// Assemble command
	_25lcxxxx_assemble_rw_cmd( &cmd, e25LCXXXX_ISA_READ, addr );

	// Send command
	// NOTE: Based on number of address bits command is being divided!
	#if ( _25LCXXXX_CFG_ADDR_BIT_NUM <= 8)
		status = _25lcxxxx_if_transmit((uint8_t*) &cmd.u, 2, eSPI_CS_LOW_ON_ENTRY );

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

		status = _25lcxxxx_if_transmit((uint8_t*) &cmd.u, 2, eSPI_CS_LOW_ON_ENTRY );

	#elif ( _25LCXXXX_CFG_ADDR_BIT_NUM <= 16 )
		status = _25lcxxxx_if_transmit((uint8_t*) &cmd.u, 3, eSPI_CS_LOW_ON_ENTRY );

	#elif ( _25LCXXXX_CFG_ADDR_BIT_NUM <= 24 )
		status = _25lcxxxx_if_transmit((uint8_t*) &cmd.u, 4, eSPI_CS_LOW_ON_ENTRY );

	#endif

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Read WIP flag from device
*
* @return 		wip - State of Write-In-Progress
*/
////////////////////////////////////////////////////////////////////////////////
static bool _25lcxxxx_read_wip_flag(void)
{
	bool 					wip 		= false;
	_25lcxxxx_status_reg_t	stat_reg	= { .u = 0 };

	if ( e25LCXXXX_OK == _25lcxxxx_read_status( & stat_reg ))
	{
		wip = (bool) ( stat_reg.b.wip );
	}

	return wip;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Read WEL flag from device
*
* @return 		wel - State of Write-Enable-Latch
*/
////////////////////////////////////////////////////////////////////////////////
static bool _25lcxxxx_read_wel_flag(void)
{
	bool wel = false;
	_25lcxxxx_status_reg_t	stat_reg	= { .u = 0 };

	if ( e25LCXXXX_OK == _25lcxxxx_read_status( & stat_reg ))
	{
		wel = (bool) ( stat_reg.b.wel );
	}

	return wel;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Wait for device to write to internal memory array
*
*	This function blocks for maximum time of timeout and continuously check
*	for WIP flag every 1ms.
*
* @param[in]	timeout 	- Timeout in miliseconds
* @return 		status 		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static _25lcxxxx_status_t _25lcxxxx_wait_for_write_process(const uint32_t timeout)
{
	_25lcxxxx_status_t 	status 		= e25LCXXXX_OK;
	uint32_t			tick_prev	= 0UL;
	uint32_t			tick		= 0UL;
	uint32_t 			safe_cnt	= 1000000UL;
	bool				wip_flag	= false;
	uint8_t				timeout_cnt = 0U;

	// Read WIP flag
	wip_flag = _25lcxxxx_read_wip_flag();

	// Write is in progress - check for timeout amount of time
	if ( true == wip_flag )
	{
		// Get current tick
		tick = _25lcxxxx_if_get_sys_time_ms();
		tick_prev = tick;

		// Continuously check for WIP
		while ( safe_cnt > 0 )
		{
			// Get current timetick
			tick = _25lcxxxx_if_get_sys_time_ms();

			// Read every 1ms
			if ( (uint32_t) ( tick - tick_prev ) >= 1UL )
			{
				tick_prev = tick;
				timeout_cnt++;

				// Read WIP flag
				wip_flag = _25lcxxxx_read_wip_flag();

				if ( false == wip_flag )
				{
					break;
				}
			}

			if ( timeout_cnt >= timeout )
			{
				status = e25LCXXXX_ERROR;
				break;
			}

			// Decrement safety counter
			safe_cnt--;
		}
	}

	_25LCXXXX_ASSERT( safe_cnt > 0 );

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
