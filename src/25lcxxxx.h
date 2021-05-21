////////////////////////////////////////////////////////////////////////////////
/**
*@file      25lcxxxx.h
*@brief    	API for 25LCxxxx EEPROM device
*@author    Ziga Miklosic
*@date      08.05.2021
*@version	V1.0.0
*/
////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup 25LCXXXX_API
* @{ <!-- BEGIN GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////

#ifndef _25LCXXXX_H_
#define _25LCXXXX_H_

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

/**
 * 	Status
 */
typedef enum
{
	e25LCXXXX_OK			= 0,		/**<Normal operation */
	e25LCXXXX_ERROR			= 0x01,		/**<General error */
	e25LCXXXX_ERROR_SPI		= 0x02,		/**<SPI error */
	e25LCXXXX_ERROR_INIT	= 0x04,		/**<Initialisation error */
	e25LCXXXX_ERROR_ADDR	= 0x08,		/**<Invalid memory address */
} _25lcxxxx_status_t;

/**
 * 	Write protection options
 */
typedef enum
{
	e25LCXXXX_PROTECT_NONE = 0,		/**<All sectors not protected */
	e25LCXXXX_PROTECT_UPPER_1_4,	/**<Upper 1/4 sectors protected (Sector 3)*/
	e25LCXXXX_PROTECT_UPPER_1_2,	/**<Upper 1/2 sectors protected (Sector 2 & 3)*/
	e25LCXXXX_PROTECT_UPPER_ALL,	/**<All sectors protected (Sector 0, 1, 2 & 3)*/
} _25lcxxxx_protect_t;

////////////////////////////////////////////////////////////////////////////////
// Functions Prototypes
////////////////////////////////////////////////////////////////////////////////
_25lcxxxx_status_t _25lcxxxx_init			(void);
_25lcxxxx_status_t _25lcxxxx_deinit			(void);
_25lcxxxx_status_t _25lcxxxx_write			(const uint32_t addr, const uint32_t size, const uint8_t * const p_data);
_25lcxxxx_status_t _25lcxxxx_read			(const uint32_t addr, const uint32_t size, uint8_t * const p_data);
_25lcxxxx_status_t _25lcxxxx_set_protection	(const _25lcxxxx_protect_t prot_opt);

#endif // _25LCXXXX_H_
