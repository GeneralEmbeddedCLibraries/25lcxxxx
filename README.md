# Microchip SPI EEPROM 25LCxxxx/25AAxxxx C library
C code for simple interface with EEPROM device via SPI communication for general application usage. Driver is written in way that makes simple use therefore only six functions are part of API. In addition for writing to chip user don't need to wory about eeprom page boundary crossing as it is all handled by driver itself. 

Driver support whole 25LCxxxx/25AAxxxx device family if correctly configurable. Driver is also platform independent as it proved additional wrapper (interface) functions that user must provide before using driver. 

[Link to device family datasheet](http://ww1.microchip.com/downloads/en/DeviceDoc/22040A.pdf)



## Driver configuration

Before using driver it is mandatory to setup configurations based on used EEPROM chip. Following macros are defining behaviour of a driver and are listed in **_25lcxxxx_cfg.h** file.

| Macros | Description | Range | Default | 
| ------------- | ----------- | ----- | --- |
| _25LCXXXX_CFG_DEBUG_EN | Enable/Disable debug mode | 0-1 | 1
| _25LCXXXX_CFG_ASSERT_EN | Enable/Disable assertions | 0-1 | 1
| _25LCXXXX_CFG_ADDR_BIT_NUM | Number of address bits | 7-17 | 13
| _25LCXXXX_CFG_PAGE_SIZE_BYTE | Size of eeprom page | 16-256 | 32

In order to correctly set **_25LCXXXX_CFG_ADDR_BIT_NUM** and **_25LCXXXX_CFG_PAGE_SIZE_BYTE** for used device following table is in help (taken from "SPI Serial EEPROM Family Data Sheet" document):

<p align="center">
  <img src=doc/pic/25lcxxxx_device_family_address_bits_page_size.png>
</p>

## Driver interface
Beside configurations user shall provide some interface function between embedded platform and driver itself. Therefore in interface files **_25lcxxxx_if.c** in **USER_CODE_BEGIN/END** sections platform specific code must be implemented.

For easier implementation there are provided template files (in "template" folder).

Functions to adopt for used plaftorm are:
 - _25lcxxxx_status_t **_25lcxxxx_if_init**(void);
 - _25lcxxxx_status_t **_25lcxxxx_if_transmit**(const uint8_t * p_data, const uint32_t size, const spi_cs_action_t cs_action);
 - _25lcxxxx_status_t **_25lcxxxx_if_receive**(uint8_t * p_data, const uint32_t size, const spi_cs_action_t cs_action);
 - uint32_t **25lcxxxx_if_get_sys_time_ms**(void);

 #### Example of interface files for STM32 using HAL library

```C

    // FROM _25lcxxxx_if.c file

    ////////////////////////////////////////////////////////////////////////////////
    /**
    *		Write bytes via SPI to device
    *
    * @note	User shall provide definition of that function based on used platform!
    *
    * @param[in] 	p_data		- Pointer to transmit data
    * @param[in] 	size		- Size of transmit data
    * @return 		status 		- Status of operation
    */
    ////////////////////////////////////////////////////////////////////////////////
    _25lcxxxx_status_t _25lcxxxx_if_transmit(const uint8_t * p_data, const uint32_t size, const spi_cs_action_t cs_action)
    {
        _25lcxxxx_status_t status = e25LCXXXX_OK;

        // USER CODE BEGIN...

        if ( eSPI_OK != spi_2_transmit( eSPI2_CH_EEPROM, (uint8_t*) p_data, size, cs_action ))
        {
            status = e25LCXXXX_ERROR_SPI;
        }

        // USER CODE END...

        return status;
    }


    // Where spi_2_transmit is defined inside spi.c file as: 

    ////////////////////////////////////////////////////////////////////////////////
    /**
    *		Transmit bytes via display SPI
    *
    * @param[in]	ch			- Channel of SPI 2
    * @param[in]	p_data		- Pointer to transmit data
    * @param[in]	size 		- Number of data to transmit
    * @param[in]	cs_action 	- Action for CS pin of SPI2 channel ch
    * @return		status 		- Status of transmittion
    */
    ////////////////////////////////////////////////////////////////////////////////
    spi_status_t spi_2_transmit(const spi_2_ch_t ch, uint8_t * p_data, const uint32_t size, const spi_cs_action_t cs_action)
    {
        spi_status_t status = eSPI_OK;

        // Is init
        if ( true == gb_spi2_is_init )
        {
            // Check if available
            if ( osOK == osMutexAcquire( g_spi2_mutex_id, SPI_2_MUTEX_TIMEOUT ))
            {
                // Set CS low if demanded
                if ( cs_action & eSPI_CS_LOW_ON_ENTRY )
                {
                    gpio_set( g_spi2_cs_pins[ch], eGPIO_LOW );
                }

                // Transmit data
                if ( HAL_OK != HAL_SPI_Transmit( &hspi2, p_data, size, SPI_2_TX_RX_TIMEOUT_MS ))
                {
                    status = eSPI_ERROR;
                }

                // Set CS high if demanded
                if ( cs_action & eSPI_CS_HIGH_ON_EXIT )
                {
                    gpio_set( g_spi2_cs_pins[ch], eGPIO_HIGH );
                }

                // Release mutex
                osMutexRelease( g_spi2_mutex_id );
            }
        }
        else
        {
            status = eSPI_ERROR;
        }

        return status;
    }
```


## Driver API
 - _25lcxxxx_status_t **_25lcxxxx_init**(void);
 - _25lcxxxx_status_t **_25lcxxxx_deinit**(void);
 - const bool **_25lcxxxx_is_init**(void);
 - _25lcxxxx_status_t **_25lcxxxx_write**(const uint32_t addr, const uint32_t size, const uint8_t * const p_data);
 - _25lcxxxx_status_t **_25lcxxxx_read**(const uint32_t addr, const uint32_t size, uint8_t * const p_data);
 - _25lcxxxx_status_t **_25lcxxxx_set_protection**(const _25lcxxxx_protect_t prot_opt);


 ## Driver usage

 ### 1. Initialization/Deinitialization

Initialization of driver can only be done if it is not initialized, otherwise program will jump into assert if those are enabled. Same goes for de-init routines but with opposite logic. 

```C
    // Initialize EEPROM driver
    if ( e25LCXXXX_OK != _25lcxxxx_init())
    {
        // Initialization failed... 
        // Further actions here...
    }


    // De-initialize EEPROM driver
    if ( e25LCXXXX_OK != _25lcxxxx_deinit())
    {
        // De-initialization failed... 
        // Further actions here...
    }
```

 ### 2. Read from EEPROM

 NOTE: **Before using _25lcxxxx_read function driver must be initialized!**

```C
    /*
    *   Read 16 bytes from address 0x100
    */
    uint8_t data[16] = {0};

    // Initialize EEPROM driver
    if ( e25LCXXXX_OK != _25lcxxxx_read( 0x100, 16, &data ))
    {
        // Reading failed... 
        // Further actions here...
    }
    else
    {
        // Reading OK
        // Read values from EEPROM stored in "data" 
    }
```

 ### 3. Write to EEPROM

 NOTE: **Before using _25lcxxxx_write function driver must be initialized!**

```C
    /*
    *   Write 3 bytes to address 0x23
    */
    uint8_t data[3] = { 0, 1, 2 };

    // Initialize EEPROM driver
    if ( e25LCXXXX_OK != _25lcxxxx_write( 0x23, 3, &data ))
    {
        // Write failed... 
        // Further actions here...
    }
    else
    {
        // Writing OK
    }
```