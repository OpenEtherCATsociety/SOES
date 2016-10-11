#ifndef _sii_eeprom_h_
#define _sii_eeprom_h_

extern const uint8_t _binary_sii_eeprom_bin_start;
extern const uint8_t _binary_sii_eeprom_bin_end;

#define SII_EE_DEFLT (&_binary_sii_eeprom_bin_start)
#define SII_EE_DEFLT_SIZE (&_binary_sii_eeprom_bin_end - &_binary_sii_eeprom_bin_start)

#endif
