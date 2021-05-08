////////////////////////////////////////////////////////////////////////////////
/**
*@file      25lcxxxx_regdef.h
*@brief     Register description of 25LCxxxx devices
*@author    Ziga Miklosic
*@date      08.05.2021
*@version	V1.0.0
*/
////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup 25LCXXXX_REGEDEF
* @{ <!-- BEGIN GROUP -->
*
* 	Register definitions of 25LCXXXX.
*/
////////////////////////////////////////////////////////////////////////////////

#ifndef _25LCXXXX_REGDEF_H_
#define _25LCXXXX_REGDEF_H_

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

/**
 * 	25LCxxxx/25AAxxxx Instruction Set
 */
typedef enum
{
	e25LCXXXX_ISA_READ		= 0x03U,		/**<Read data from memory array begging at selected address */
	e25LCXXXX_ISA_WRITE		= 0x02U,		/**<Write data to memory array begging at selected address */
	e25LCXXXX_ISA_WREN		= 0x06U,		/**<Set write enable latch - enable write operation */
	e25LCXXXX_ISA_WRDI		= 0x04U,		/**<Reset write enable latch - disable write operation */
	e25LCXXXX_ISA_RDST		= 0x05U,		/**<Read STATUS register */
	e25LCXXXX_ISA_WDST		= 0x01U,		/**<Write STATUS register */

	// Additional instructions for 25XX512 and 25XX11024

	e25LCXXXX_ISA_PE		= 0x42U,		/**<Page erase - erase one page in memory array */
	e25LCXXXX_ISA_SE		= 0xD8U,		/**<Sector erase - erase one sector in memory array */
	e25LCXXXX_ISA_CE		= 0xC7U,		/**<Chip erase - erase all sectors in memory array */
	e25LCXXXX_ISA_RDID		= 0xABU,		/**<Relase from Deep power-down and read electronic signature */
	e25LCXXXX_ISA_DPD		= 0xB9U,		/**<Depp Power-down mode */

} _25lcxxxx_isa_t;

/**
 * 	Status register
 */
typedef struct
{
	uint8_t wip 	: 1;	/**<Write-In-Progress */
	uint8_t wel 	: 1; 	/**<Write Enable Lactch */
	uint8_t bp		: 2;	/**<Block protection */
	uint8_t	res 	: 3;	/**<Reserved */
	uint8_t wpen	: 1;	/**<Write Protect enable */
} _25lcxxx_status_reg_bits_t;

typedef union
{
	uint8_t u;						/**<Unsigned access */
	_25lcxxx_status_reg_bits_t b;	/**<Bitfield access */
} _25lcxxxx_status_reg_t;

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


#endif // _25LCXXXX_REGDEF_H_
