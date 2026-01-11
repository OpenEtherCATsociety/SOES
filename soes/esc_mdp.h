/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

/** \file
 * \brief
 * Header file for Modular Device Profile (MDP) module
 *
 * Implementation according to ETG.5001.1 S (D) V0.9.0
 * Part 1: General MDP Device Model
 *
 * TODO: ETG.5001.3 - MDP Fieldbus Gateway Profile Specification
 * - Add support for Fieldbus Gateway profiles (EtherCAT MainDevices, Profibus DP, CAN, CANopen, DeviceNet)
 * - Implement protocol-specific objects for each fieldbus type
 * - Add gateway-specific configuration and status objects
 *
 * TODO: ETG.5001.4 - MDP Safety Module Specification
 * - Add support for FSoE (Fail Safe over EtherCAT) profiles
 * - Implement FSoE Digital I/O connection profile
 * - Implement FSoE Safety Drive Profile
 * - Implement FSoE MainDevice profile
 * - Add safety-related objects and validation
 */

#ifndef SOES_MDP_H
#define SOES_MDP_H
#include <stdint.h>
#include "cc.h" // SOES compiler configuration
#include "options.h" // For MDP configuration values (MDP_OBJECTS_PER_MODULE, MDP_INDEX_STEP, MAX_MDP_SLOTS)
#include "esc.h" // For DEVICE_TYPE_* definitions

/** \defgroup MDP_ADDRESSING MDP Addressing Offsets
 * @{
 * 
 * MDP Addressing according to ETG.5001.1 Table 3: Object Dictionary Structure
 * 
 * Object Area of the Modules (0x2000 - 0xEFFF):
 * - Each module occupies 16 objects (0x0-0xF) in each function-specific area
 * - Module 0: 0x6000-0x600F, 0x7000-0x700F, 0x8000-0x800F, 0x9000-0x900F, etc.
 * - Module 1: 0x6010-0x601F, 0x7010-0x701F, 0x8010-0x801F, 0x9010-0x901F, etc.
 * - Index distance between modules: 0x0010 (16 objects)
 */
#define MDP_MANUFACTURER_AREA_START  0x2000  /**< Manufacturer Specific Area (0x2000 - 0x5FFF) */
#define MDP_INPUT_AREA_START         0x6000  /**< Input Area (0x6000 - 0x6FFF) - Objects mappable to TxPDOs */
#define MDP_OUTPUT_AREA_START        0x7000  /**< Output Area (0x7000 - 0x7FFF) - Objects mappable to RxPDOs */
#define MDP_CONFIG_AREA_START        0x8000  /**< Configuration Area (0x8000 - 0x8FFF) - Configuration and setting objects */
#define MDP_INFO_AREA_START         0x9000  /**< Information Area (0x9000 - 0x9FFF) - Scanned information from modules */
#define MDP_DIAGNOSIS_AREA_START     0xA000  /**< Diagnosis Area (0xA000 - 0xAFFF) - Diagnostic, status, statistic info */
#define MDP_SERVICE_AREA_START       0xB000  /**< Service Transfer Area (0xB000 - 0xBFFF) - Service objects */
/** Reserved Area: 0xC000 - 0xEFFF */

/** Object Area of the Device (0xF000 - 0xFFFF) */
#define MDP_DEVICE_AREA_START        0xF000  /**< Device Area - Parameters belonging to the device */
/** @} */

/** MDP Index Structure (ETG.5001.1)
 * 
 * Each module can occupy several objects in the function specific areas.
 * The standard defines 16 objects per module in a specific area, but this
 * number may be adapted to the device requirements.
 * 
 * Module 0: 0x6000-0x600F, 0x7000-0x700F, 0x8000-0x800F, 0x9000-0x900F, etc.
 * Module 1: 0x6010-0x601F, 0x7010-0x701F, 0x8010-0x801F, 0x9010-0x901F, etc.
 * 
 * In the following it will be assumed that there are 16 objects per module
 * and up to 255 modules available.
 * 
 * Configuration values are defined in options.h and can be overridden
 * in ecat_options.h according to device requirements.
 */


/**
 * @brief Module data structure (ETG.5001.1)
 * Each module can have data in multiple object dictionary areas.
 *
//// @param module_ident Module identifier (for 0x9nn0 sub 1)

//// @param device_type Device Type (0x9nn0 sub 4)
//// @param vendor_id Vendor ID (0x9nn0 sub 5)
//// @param product_code Product Code (0x9nn0 sub 6)
//// @param revision Revision (0x9nn0 sub 7)
//// @param serial_number Serial Number (0x9nn0 sub 8)

//// @param input_data Pointer to input data buffer (0x6nn0 sub 1-255, TxPDO). Each module may have up to 16 Input Data objects.
//// @param output_data Pointer to output data buffer (0x7nn0 sub 1-255, RxPDO). Each module may have up to 16 Output Data objects.
//// @param configuration_data Pointer to configuration data buffer (0x8nn0). Each module may have up to 16 Configuration Data objects.
//// @param information_data Pointer to information data buffer (0x9nn0). Each module may have up to 16 Information Data objects.
//// @param diagnosis_data Pointer to diagnosis data buffer (0xAnnn). Each module may have up to 16 Diagnosis Data objects.
//// @param service_transfer Pointer to service data buffer (0xBnnn)

//// @param input_size Input data size in bits
//// @param output_size Output data size in bits
//// @param configuration_size Configuration data size in bits
//// @param diagnosis_size Diagnosis data size in bits
//// @param service_size Service data size in bits

//// @param config_read_callback Callback for reading configuration data
//// @param config_write_callback Callback for writing configuration data
//// @param diagnosis_read_callback Callback for reading diagnosis data
//// @param service_read_callback Callback for reading service data
//// @param service_write_callback Callback for writing service data

//// @param txpdo_read_callback Callback for custom TxPDO access (subindex 1-255)
//// @param rxpdo_write_callback Callback for custom RxPDO access (subindex 1-255)
 */
typedef struct {
    uint32_t module_ident;

    uint32_t device_type;
    uint32_t vendor_id;
    uint32_t product_code;
    uint32_t revision;
    uint32_t serial_number;

    void     *input_data;
    void     *output_data;
    void     *configuration_data;
    void     *information_data;
    void     *diagnosis_data;
    void     *service_transfer;

    uint16_t input_size;
    uint16_t output_size;
    uint16_t configuration_size;
    uint16_t information_size;
    uint16_t diagnosis_size;
    uint16_t service_size;

    uint32_t (*configuration_read_callback)(uint8_t slot, uint8_t subindex, void *data, uint16_t *size);
    uint32_t (*configuration_write_callback)(uint8_t slot, uint8_t subindex, void *data, uint16_t size);
    uint32_t (*diagnosis_read_callback)(uint8_t slot, uint8_t subindex, void *data, uint16_t *size);
    uint32_t (*service_read_callback)(uint8_t slot, uint8_t subindex, void *data, uint16_t *size);
    uint32_t (*service_write_callback)(uint8_t slot, uint8_t subindex, void *data, uint16_t size);

    uint32_t (*txpdo_read_callback)(uint8_t slot, uint8_t pdo_number, void *data, uint16_t *size);
    uint32_t (*rxpdo_write_callback)(uint8_t slot, uint8_t pdo_number, void *data, uint16_t size);
} mdp_module_t;

// Глобальный массив слотов
extern mdp_module_t mdp_slots[MAX_MDP_SLOTS];
extern uint8_t mdp_active_modules_count;

// API Functions

//// @brief Get device subtype from Object Dictionary (Object 0x1000)
////
//// Reads the Device Type value from Object 0x1000 subindex 0x00 and extracts the AddInfo field.
//// According to ETG.5001.1 Clause 4.2.1.2: Object 0x1000: Device Type
//// - Bits 0-15: ProfileNo (shall be 5001 = 0x1389 for the modular device profile)
//// - Bits 16-31: AddInfo (shall contain the module profile number defined by ETG)
////   If the module profile number is 0, the device supports no standardized module profile
////   or multiple module profiles.
////
//// This function validates that ProfileNo = 0x1389 (MDP) and returns the AddInfo value.
////
//// @return Device subtype:
////   - 0x01, 0x02, 0x03: Device subtype identifiers (DEVICE_TYPE_FIELDBUS_GATEWAY, DEVICE_TYPE_MODULAR_DEVICE, DEVICE_TYPE_MODULE_DEVICE)
////   - 0x00: AddInfo = 0 (valid MDP device, supports no standardized module profile or multiple profiles)
////   - 0xFF: Not an MDP device (ProfileNo != 0x1389) or Object 0x1000 not found

uint8_t MDP_get_device_subtype(void);

void MDP_init(void);

// Basic module addition (Input/Output areas only)
int MDP_add_module(uint8_t slot, uint32_t module_id, void *in, uint16_t in_size, void *out, uint16_t out_size);

// Extended module addition with configuration support
int MDP_add_module_with_config(uint8_t slot, uint32_t module_id, 
                                void *in, uint16_t in_size, 
                                void *out, uint16_t out_size,
                                void *config, uint16_t config_size,
                                uint32_t (*config_read_cb)(uint8_t slot, uint8_t subindex, void *data, uint16_t *size),
                                uint32_t (*config_write_cb)(uint8_t slot, uint8_t subindex, void *data, uint16_t size));

// Full module addition with all areas (for advanced usage)
int MDP_add_module_full(uint8_t slot, uint32_t module_id,
                        void *in, uint16_t in_size,
                        void *out, uint16_t out_size,
                        void *config, uint16_t config_size,
                        void *diagnosis, uint16_t diagnosis_size,
                        void *service, uint16_t service_size,
                        uint32_t device_type, uint32_t vendor_id, uint32_t product_code,
                        uint32_t revision, uint32_t serial_number,
                        uint32_t (*config_read_cb)(uint8_t slot, uint8_t subindex, void *data, uint16_t *size),
                        uint32_t (*config_write_cb)(uint8_t slot, uint8_t subindex, void *data, uint16_t size),
                        uint32_t (*diagnosis_read_cb)(uint8_t slot, uint8_t subindex, void *data, uint16_t *size),
                        uint32_t (*service_read_cb)(uint8_t slot, uint8_t subindex, void *data, uint16_t *size),
                        uint32_t (*service_write_cb)(uint8_t slot, uint8_t subindex, void *data, uint16_t size));

// Функция для перехвата SDO запросов из esc_coe.c
// Возвращает размер данных или ошибку SDO
uint32_t MDP_read_access(uint16_t index, uint8_t subindex, uint16_t *size, void *ptr, uint32_t *abort_code);
uint32_t MDP_write_access(uint16_t index, uint8_t subindex, uint16_t size, void *ptr, uint32_t *abort_code);

#endif //SOES_MDP_H