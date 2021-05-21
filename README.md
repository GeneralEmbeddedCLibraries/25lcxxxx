# Microchip SPI EEPROM 25LCxxxx/25AAxxxx C library
C code for simple interface with EEPROM device via SPI communication for general application usage. 

[Link to device family datasheet](http://ww1.microchip.com/downloads/en/DeviceDoc/22040A.pdf)



## Driver configuration

Before using driver it is mandatory to setup configurations based on used EEPROM chip. Following macros are defining behaviour of a driver and are listed in **_25lcxxxx_cfg.h** file.

| Macros | Description | Range | Default | 
| ------------- | ----------- | ----- | --- |
| _25LCXXXX_CFG_DEBUG_EN | Enable/Disable debug mode | 0-1 | 1
| _25LCXXXX_CFG_ASSERT_EN | Enable/Disable assertions | 0-1 | 1
| _25LCXXXX_CFG_ADDR_BIT_NUM | Number of address bits | 7-17 | 13
| _25LCXXXX_CFG_PAGE_SIZE_BYTE | Size of eeprom page | 16-256 | 32

In order to correctly set **_25LCXXXX_CFG_ADDR_BIT_NUM** and **_25LCXXXX_CFG_PAGE_SIZE_BYTE** for used device following table is in help:

<p align="center">
  <img src=doc/pic/25lcxxxx_device_family_address_bits_page_size.png>
</p>

## Driver API

