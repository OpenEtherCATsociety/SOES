/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

/** \file
 * \brief
 * Modular Device Profile (MDP) module for EtherCAT
 *
 * Implementation according to ETG.5001.1 S (D) V0.9.0
 * Part 1: General MDP Device Model
 *
 * This module provides support for Modular Device Profile, allowing
 * EtherCAT devices to expose modular structures with configurable slots.
 *
 * TODO: ETG.5001.3 - MDP Fieldbus Gateway Profile Specification
 * - Add support for Fieldbus Gateway profiles:
 *   * EtherCAT MainDevices profile
 *   * Profibus DP profile
 *   * CAN profile
 *   * CANopen profile
 *   * DeviceNet profile
 * - Implement protocol-specific objects for each fieldbus type
 * - Add gateway-specific configuration objects (gateway status, protocol selection, etc.)
 * - Add gateway-specific status objects (connection status per protocol, error counters, etc.)
 *
 * TODO: ETG.5001.4 - MDP Safety Module Specification
 * - Add support for FSoE (Fail Safe over EtherCAT) profiles:
 *   * FSoE Digital I/O connection profile
 *   * FSoE Safety Drive Profile
 *   * FSoE MainDevice profile
 * - Implement safety-related objects (safety state, safety configuration, etc.)
 * - Add safety validation and error handling
 * - Implement safety communication protocol requirements
 */

#include "esc_mdp.h"
#include <string.h>
#include "esc_coe.h"

mdp_module_t mdp_slots[MAX_MDP_SLOTS];
uint8_t mdp_active_modules_count = 0;

uint8_t MDP_get_device_subtype(void) {
    /* Read Device Type from Object 0x1000 subindex 0x00 and extract AddInfo
     * According to ETG.5001.1 Clause 4.2.1.2: Object 0x1000: Device Type
     * The device type object contains the device profile information.
     * - Bits 0-15: ProfileNo (shall be 5001 = 0x1389 for the modular device profile)
     * - Bits 16-31: AddInfo (shall contain the module profile number defined by ETG)
     *   If the module profile number is 0, the device supports no standardized module profile
     *   or multiple module profiles.
     * 
     * This function returns the AddInfo field (module profile number) for MDP devices.
     * Return value:
     * - 0x01, 0x02, 0x03: Device subtype identifiers (if AddInfo matches DEVICE_TYPE_* constants)
     * - 0x00: AddInfo = 0 (valid MDP device, supports no standardized module profile or multiple profiles)
     * - 0xFF: Not an MDP device (ProfileNo != 0x1389) or Object 0x1000 not found
     */
    int32_t nidx = SDO_findobject(0x1000);
    if (nidx < 0) {
        return 0xFF; /* Object 0x1000 not found */
    }
    
    const _objectlist *objlist = &DeviceAreaobjects[nidx];
    if (objlist->objdesc == NULL) {
        return 0xFF; /* Object 0x1000 has no description */
    }
    
    /* Read Device Type value from Object 0x1000 subindex 0x00 */
    uint32_t device_type_value = OBJ_VALUE_FETCH(device_type_value, objlist->objdesc[0]);
    
    /* Check if ProfileNo matches MDP (0x1389 = 5001) */
    uint16_t profileno = (uint16_t)(device_type_value & 0xFFFF);
    if (profileno != 0x1389) {
        return 0xFF; /* Not an MDP device */
    }

    /* Extract AddInfo (upper 16 bits) - module profile number */
    uint16_t addinfo = (uint16_t)((device_type_value >> 16) & 0xFFFF);

    /* According to ETG.5001.1, AddInfo = 0 is valid:
     * "If the module profile number is 0, the device supports no standardized module profile
     *  or multiple module profiles."
     * 
     * If AddInfo matches device subtype identifiers, return them.
     * Otherwise, return AddInfo value (which may be 0 or other module profile numbers).
     */
    if (addinfo == DEVICE_TYPE_FIELDBUS_GATEWAY ||
        addinfo == DEVICE_TYPE_MODULAR_DEVICE ||
        addinfo == DEVICE_TYPE_MODULE_DEVICE) {
        return (uint8_t)addinfo;
    }
    
    /* AddInfo = 0 is valid for MDP devices (no standardized module profile or multiple profiles) */
    if (addinfo == 0) {
        return 0;
    }
    
    /* Other AddInfo values may be valid module profile numbers, but we only support
     * device subtype identifiers (0x01, 0x02, 0x03) for Information Area validation.
     * Return 0xFF to indicate unsupported device subtype for our validation purposes.
     */
    return 0xFF;
}

void MDP_init(void) {
    memset(mdp_slots, 0, sizeof(mdp_slots));
    mdp_active_modules_count = 0;
    
    /* Validate device subtype during initialization */
    uint8_t device_subtype = MDP_get_device_subtype();
    if (device_subtype == 0xFF) {
        /* Not an MDP device or Object 0x1000 not found - this is a warning, not an error
         * as Object 0x1000 might be defined in application-specific code
         */
    }
    else if (device_subtype == 0) {
        /* AddInfo = 0: Device supports no standardized module profile or multiple module profiles
         * This is valid according to ETG.5001.1 Clause 4.2.1.2
         */
    }
    /* Device subtype validation implemented:
     * - Device subtype is validated in MDP_get_device_subtype() (checks supported MDP subtypes)
     * - Information Area availability validation is implemented in:
     *   * MDP_read_access() - blocks access to 0x9nn0 for DEVICE_TYPE_MODULE_DEVICE
     *   * device_obj_areas_init_module_dict() - prevents registration of Information Area for DEVICE_TYPE_MODULE_DEVICE
     */
}

int MDP_add_module(uint8_t slot, uint32_t module_id, void *in, uint16_t in_size, void *out, uint16_t out_size) {
    return MDP_add_module_with_config(slot, module_id, in, in_size, out, out_size, NULL, 0, NULL, NULL);
}

int MDP_add_module_with_config(uint8_t slot, uint32_t module_id, 
                                void *in, uint16_t in_size, 
                                void *out, uint16_t out_size,
                                void *config, uint16_t config_size,
                                uint32_t (*config_read_cb)(uint8_t slot, uint8_t subindex, void *data, uint16_t *size),
                                uint32_t (*config_write_cb)(uint8_t slot, uint8_t subindex, void *data, uint16_t size)) {
    return MDP_add_module_full(slot, module_id, in, in_size, out, out_size,
                                config, config_size, NULL, 0, NULL, 0,
                                0, 0, 0, 0, 0, /* Information Area fields default to 0 */
                                config_read_cb, config_write_cb, NULL, NULL, NULL);
}

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
                        uint32_t (*service_write_cb)(uint8_t slot, uint8_t subindex, void *data, uint16_t size)) {
    if (slot >= MAX_MDP_SLOTS) return -1;

    /* Device subtype validation for Information Area is implemented in device_obj_areas_init_module_dict()
     * According to ETG.5001.1, Information Area (0x9nn0) validation:
     * - Information Area registration is blocked for DEVICE_TYPE_MODULE_DEVICE in device_obj_areas_init_module_dict()
     * - Information Area access is blocked for DEVICE_TYPE_MODULE_DEVICE in MDP_read_access()
     * Device subtype is checked via MDP_get_device_subtype() which reads AddInfo (bits 16-31) from Object 0x1000
     */
    mdp_slots[slot].module_ident = module_id;
    mdp_slots[slot].input_data = in;
    mdp_slots[slot].input_size = in_size;
    mdp_slots[slot].output_data = out;
    mdp_slots[slot].output_size = out_size;
    mdp_slots[slot].configuration_data = config;
    mdp_slots[slot].configuration_size = config_size;
    mdp_slots[slot].diagnosis_data = diagnosis;
    mdp_slots[slot].diagnosis_size = diagnosis_size;
    mdp_slots[slot].service_transfer = service;
    mdp_slots[slot].service_size = service_size;
    mdp_slots[slot].device_type = device_type;
    mdp_slots[slot].vendor_id = vendor_id;
    mdp_slots[slot].product_code = product_code;
    mdp_slots[slot].revision = revision;
    mdp_slots[slot].serial_number = serial_number;
    mdp_slots[slot].configuration_read_callback = config_read_cb;
    mdp_slots[slot].configuration_write_callback = config_write_cb;
    mdp_slots[slot].diagnosis_read_callback = diagnosis_read_cb;
    mdp_slots[slot].service_read_callback = service_read_cb;
    mdp_slots[slot].service_write_callback = service_write_cb;

    // Update active modules count
    if (slot >= mdp_active_modules_count) {
        mdp_active_modules_count = slot + 1;
    }
    return 0;
}

// Helper function to calculate slot number from index
// ETG.5001.1: Each module occupies MDP_OBJECTS_PER_MODULE objects in each area
// Module 0: base+0x0 to base+(MDP_OBJECTS_PER_MODULE-1), 
// Module 1: base+MDP_INDEX_STEP to base+(MDP_INDEX_STEP+MDP_OBJECTS_PER_MODULE-1), etc.
static int get_slot_from_index(uint16_t index, uint16_t base_addr) {
    if (index < base_addr) return -1;
    
    // Check if index is within the area range (each area is 0x1000 bytes)
    uint16_t area_end = base_addr + 0x1000;
    if (index >= area_end) return -1;
    
    // Calculate offset from base address
    uint16_t offset = index - base_addr;
    
    // Check if offset is aligned to module boundary (must be multiple of MDP_INDEX_STEP)
    if (offset % MDP_INDEX_STEP != 0) return -1;
    
    // Calculate slot number
    int slot = offset / MDP_INDEX_STEP;
    
    // Check bounds and module existence
    if (slot >= MAX_MDP_SLOTS || mdp_slots[slot].module_ident == 0) return -1;
    
    return slot;
}

uint32_t MDP_read_access(uint16_t index, uint8_t subindex, uint16_t *size, void *ptr, uint32_t *abort_code) {
    int slot;
    uint8_t *data_ptr = 0;
    uint16_t data_len = 0;

    // Временные переменные для отправки простых типов данных
    static uint32_t temp_u32;
    static uint16_t temp_u16;
    static uint8_t  temp_u8;

    // -------------------------------------------------------------------------
    // ETG.5001.1 Clause 5.1: Device Area (0xF0xx)
    // Обязательные объекты для описания структуры MDP устройства
    // -------------------------------------------------------------------------

    // 0xF000: Modular Device Profile
    // ETG.5001.1: Contains device-level MDP parameters
    if (index == 0xF000) {
        if (subindex == 0) {
            // Subindex 0: Number of entries (Max subindex)
            temp_u8 = 2; // Max subindex = 2 (subindexes 0, 1, 2)
            data_ptr = &temp_u8; data_len = 1;
        }
        else if (subindex == 1) {
            // Subindex 1: Index distance
            // Distance between module indices (typically 0x0010)
            temp_u16 = MDP_INDEX_STEP; // 0x0010
            data_ptr = (uint8_t*)&temp_u16; data_len = 2;
        }
        else if (subindex == 2) {
            // Subindex 2: Max number of modules
            // Maximum number of module slots supported by this device
            temp_u16 = MAX_MDP_SLOTS;
            data_ptr = (uint8_t*)&temp_u16; data_len = 2;
        }
        else {
            *abort_code = 0x06020000; // Sub-index does not exist
            return 0;
        }
    }
    // 0xF030: Configured Module Ident List
    // ETG.5001.1: List of module identifiers currently configured in slots
    // Subindex 0: Number of entries (number of slots)
    // Subindex 1..N: Module identifier for slot (N-1), 0 if slot is empty
    else if (index == 0xF030) {
        if (subindex == 0) {
            // Subindex 0: Number of entries (number of slots)
            temp_u8 = MAX_MDP_SLOTS;
            data_ptr = &temp_u8; data_len = 1;
        }
        else if (subindex >= 1 && subindex <= MAX_MDP_SLOTS) {
            // Subindex N: Module identifier for slot (N-1)
            // Returns 0 if slot is empty (not configured)
            slot = subindex - 1;
            temp_u32 = mdp_slots[slot].module_ident; // 0 if slot is empty
            data_ptr = (uint8_t*)&temp_u32; data_len = 4;
        }
        else {
            *abort_code = 0x06020000; // Sub-index does not exist
            return 0;
        }
    }

    // -------------------------------------------------------------------------
    // ETG.5001.1 Clause 5.2: Module Area
    // Module-specific objects organized by slot number
    // Index format: Base + (Slot * Index Distance)
    // -------------------------------------------------------------------------

    // --- INPUTS (0x6nn0) ---
    // ETG.5001.1: Module input data area
    // Subindex 0: Number of entries (Max subindex)
    // Subindex 1: Input data (read-only from master perspective)
    else if ((slot = get_slot_from_index(index, MDP_INPUT_AREA_START)) >= 0) {
        if (subindex == 0x00) {
            // Subindex 0: Number of entries
            // Returns 1 if input data exists, 0 otherwise
            temp_u8 = (mdp_slots[slot].input_data) ? 1 : 0;
            data_ptr = &temp_u8; data_len = 1;
        }
        else if (subindex == 0x01 && mdp_slots[slot].input_data) {
            // Subindex 1: Input data
            // Returns the input data for this module slot
            data_ptr = (uint8_t*)mdp_slots[slot].input_data;
            data_len = (mdp_slots[slot].input_size + 7) / 8; // Convert bits to bytes
        } else {
             *abort_code = 0x06020000; // Sub-index does not exist
             return 0;
        }
    }
    // --- OUTPUTS (0x7nn0) ---
    // ETG.5001.1: Module output data area
    // Subindex 0: Number of entries (Max subindex)
    // Subindex 1: Output data (writable by master)
    else if ((slot = get_slot_from_index(index, MDP_OUTPUT_AREA_START)) >= 0) {
        if (subindex == 0x00) {
            // Subindex 0: Number of entries
            // Returns 1 if output data exists, 0 otherwise
            temp_u8 = (mdp_slots[slot].output_data) ? 1 : 0;
            data_ptr = &temp_u8; data_len = 1;
        }
        else if (subindex == 0x01 && mdp_slots[slot].output_data) {
            // Subindex 1: Output data
            // Returns the output data for this module slot
            data_ptr = (uint8_t*)mdp_slots[slot].output_data;
            data_len = (mdp_slots[slot].output_size + 7) / 8; // Convert bits to bytes
        } else {
             *abort_code = 0x06020000; // Sub-index does not exist
             return 0;
        }
    }
    // --- CONFIGURATION (0x8nn0) ---
    else if ((slot = get_slot_from_index(index, MDP_CONFIG_AREA_START)) >= 0) {
        // ETG.5001.1: Configuration area for module-specific parameters
        if (subindex == 0x00) {
            // Max Subindex
            if (mdp_slots[slot].configuration_data || mdp_slots[slot].configuration_read_callback) {
                temp_u8 = 1; // At least subindex 1 exists
                data_ptr = &temp_u8; data_len = 1;
            } else {
                *abort_code = 0x06020000;
                return 0; // No configuration available
            }
        }
        else if (subindex == 0x01) {
            // Configuration data
            if (mdp_slots[slot].configuration_read_callback) {
                // Use callback if available
                uint16_t callback_size = *size;
                uint32_t callback_abort = mdp_slots[slot].configuration_read_callback((uint8_t)slot, subindex, ptr, &callback_size);
                if (callback_abort != 0) {
                    *abort_code = callback_abort;
                    return 0;
                }
                *size = callback_size;
                return 1; // Callback handled the data
            } else if (mdp_slots[slot].configuration_data) {
                data_ptr = (uint8_t*)mdp_slots[slot].configuration_data;
                data_len = (mdp_slots[slot].configuration_size + 7) / 8;
            } else {
                *abort_code = 0x06020000;
                return 0;
            }
        }
        else {
            *abort_code = 0x06020000;
            return 0;
        }
    }
    // --- MODULE INFO (0x9nn0) ---
    // ETG.5001.1: Information Area - Scanned information from modules
    else if ((slot = get_slot_from_index(index, MDP_INFO_AREA_START)) >= 0) {
        /* Validate Information Area availability based on device subtype
         * According to ETG.5001.1, Information Area (0x9nn0) should only be available for:
         * - DEVICE_TYPE_FIELDBUS_GATEWAY (0x01)
         * - DEVICE_TYPE_MODULAR_DEVICE (0x02)
         * - AddInfo = 0 (device supports no standardized module profile or multiple profiles)
         * Information Area is NOT available for DEVICE_TYPE_MODULE_DEVICE (0x03)
         * Device subtype is read from AddInfo field (bits 16-31) of Object 0x1000: Device Type
         */
        uint8_t device_subtype = MDP_get_device_subtype();
        if (device_subtype == DEVICE_TYPE_MODULE_DEVICE) {
            *abort_code = 0x06020000; /* Object does not exist */
            return 0;
        }
        /* If device_subtype == 0xFF (not MDP) or device_subtype == 0 (AddInfo = 0),
         * Information Area is available (default behavior for non-Module Device types)
         */
        // ETG.5001.1: Information Area subindexes
        // Subindex 0: Number of Entries (Max SubIndex)
        // Subindex 1: Module Profile / Ident
        // Subindex 4: Device Type
        // Subindex 5: Vendor ID
        // Subindex 6: Product Code
        // Subindex 7: Revision
        // Subindex 8: Serial Number
        if (subindex == 0x00) {
            // Calculate max subindex based on available fields
            // At minimum, subindex 1 (module_ident) exists
            uint8_t max_subindex = 1;
            if (mdp_slots[slot].device_type != 0 ||
                mdp_slots[slot].vendor_id != 0 ||
                mdp_slots[slot].product_code != 0 ||
                mdp_slots[slot].revision != 0 ||
                mdp_slots[slot].serial_number != 0) {
                max_subindex = 8; // All Information Area fields available
            }
            temp_u8 = max_subindex;
            data_ptr = &temp_u8; data_len = 1;
        }
        else if (subindex == 0x01) {
            // Subindex 1: Module Profile / Ident
            temp_u32 = mdp_slots[slot].module_ident;
            data_ptr = (uint8_t*)&temp_u32;
            data_len = 4;
        }
        else if (subindex == 0x04) {
            // Subindex 4: Device Type
            temp_u32 = mdp_slots[slot].device_type;
            data_ptr = (uint8_t*)&temp_u32;
            data_len = 4;
        }
        else if (subindex == 0x05) {
            // Subindex 5: Vendor ID
            temp_u32 = mdp_slots[slot].vendor_id;
            data_ptr = (uint8_t*)&temp_u32;
            data_len = 4;
        }
        else if (subindex == 0x06) {
            // Subindex 6: Product Code
            temp_u32 = mdp_slots[slot].product_code;
            data_ptr = (uint8_t*)&temp_u32;
            data_len = 4;
        }
        else if (subindex == 0x07) {
            // Subindex 7: Revision
            temp_u32 = mdp_slots[slot].revision;
            data_ptr = (uint8_t*)&temp_u32;
            data_len = 4;
        }
        else if (subindex == 0x08) {
            // Subindex 8: Serial Number
            temp_u32 = mdp_slots[slot].serial_number;
            data_ptr = (uint8_t*)&temp_u32;
            data_len = 4;
        }
        else {
            // Subindexes 2-3 are reserved, subindexes > 8 are not defined
            *abort_code = 0x06020000;
            return 0;
        }
    }
    // --- DIAGNOSIS (0xAnnn) ---
    // ETG.5001.1: Diagnosis Area - Diagnostic, status, statistic information
    else if ((slot = get_slot_from_index(index, MDP_DIAGNOSIS_AREA_START)) >= 0) {
        if (subindex == 0x00) {
            // Subindex 0: Number of entries
            if (mdp_slots[slot].diagnosis_data || mdp_slots[slot].diagnosis_read_callback) {
                temp_u8 = 1; // At least subindex 1 exists
                data_ptr = &temp_u8; data_len = 1;
            } else {
                *abort_code = 0x06020000;
                return 0; // No diagnosis data available
            }
        }
        else if (subindex == 0x01) {
            // Subindex 1: Diagnosis data
            if (mdp_slots[slot].diagnosis_read_callback) {
                // Use callback if available
                uint16_t callback_size = *size;
                uint32_t callback_abort = mdp_slots[slot].diagnosis_read_callback((uint8_t)slot, subindex, ptr, &callback_size);
                if (callback_abort != 0) {
                    *abort_code = callback_abort;
                    return 0;
                }
                *size = callback_size;
                return 1; // Callback handled the data
            } else if (mdp_slots[slot].diagnosis_data) {
                data_ptr = (uint8_t*)mdp_slots[slot].diagnosis_data;
                data_len = (mdp_slots[slot].diagnosis_size + 7) / 8;
            } else {
                *abort_code = 0x06020000;
                return 0;
            }
        }
        else {
            *abort_code = 0x06020000;
            return 0;
        }
    }
    // --- SERVICE TRANSFER (0xBnnn) ---
    // ETG.5001.1: Service Transfer Area - Service objects
    else if ((slot = get_slot_from_index(index, MDP_SERVICE_AREA_START)) >= 0) {
        if (subindex == 0x00) {
            // Subindex 0: Number of entries
            if (mdp_slots[slot].service_transfer || mdp_slots[slot].service_read_callback) {
                temp_u8 = 1; // At least subindex 1 exists
                data_ptr = &temp_u8; data_len = 1;
            } else {
                *abort_code = 0x06020000;
                return 0; // No service data available
            }
        }
        else if (subindex == 0x01) {
            // Subindex 1: Service data
            if (mdp_slots[slot].service_read_callback) {
                // Use callback if available
                uint16_t callback_size = *size;
                uint32_t callback_abort = mdp_slots[slot].service_read_callback((uint8_t)slot, subindex, ptr, &callback_size);
                if (callback_abort != 0) {
                    *abort_code = callback_abort;
                    return 0;
                }
                *size = callback_size;
                return 1; // Callback handled the data
            } else if (mdp_slots[slot].service_transfer) {
                data_ptr = (uint8_t*)mdp_slots[slot].service_transfer;
                data_len = (mdp_slots[slot].service_size + 7) / 8;
            } else {
                *abort_code = 0x06020000;
                return 0;
            }
        }
        else {
            *abort_code = 0x06020000;
            return 0;
        }
    }

    // Копирование данных в буфер ответа
    if (data_ptr) {
        if (ptr && *size >= data_len) {
            memcpy(ptr, data_ptr, data_len);
            *size = data_len; // Возвращаем реальный размер
            return 1; // Success
        } else {
            *abort_code = 0x06070010; // Data type length too high / mismatch
            return 0;
        }
    }

    // Если объект не перехвачен MDP логикой
    return 0;
}

uint32_t MDP_write_access(uint16_t index, uint8_t subindex, uint16_t size, void *ptr, uint32_t *abort_code) {
    int slot;

    // --- OUTPUTS (0x7nn0) ---
    if ((slot = get_slot_from_index(index, MDP_OUTPUT_AREA_START)) >= 0) {
        if (subindex == 0x01 && mdp_slots[slot].output_data) {
            uint16_t slot_len = (mdp_slots[slot].output_size + 7) / 8;
            if (size > slot_len) {
                 *abort_code = 0x06070010;
                 return 0;
            }
            memcpy(mdp_slots[slot].output_data, ptr, size);
            return 1;
        }
        else {
            *abort_code = 0x06020000; // Invalid subindex
            return 0;
        }
    }
    
    // --- CONFIGURATION (0x8nn0) ---
    else if ((slot = get_slot_from_index(index, MDP_CONFIG_AREA_START)) >= 0) {
        if (subindex == 0x01) {
            if (mdp_slots[slot].configuration_write_callback) {
                // Use callback if available
                uint32_t callback_abort = mdp_slots[slot].configuration_write_callback((uint8_t)slot, subindex, ptr, size);
                if (callback_abort != 0) {
                    *abort_code = callback_abort;
                    return 0;
                }
                return 1; // Callback handled the write
            } else if (mdp_slots[slot].configuration_data) {
                uint16_t slot_len = (mdp_slots[slot].configuration_size + 7) / 8;
                if (size > slot_len) {
                    *abort_code = 0x06070010;
                    return 0;
                }
                memcpy(mdp_slots[slot].configuration_data, ptr, size);
                return 1;
            } else {
                *abort_code = 0x06020000; // No configuration available
                return 0;
            }
        }
        else {
            *abort_code = 0x06020000; // Invalid subindex
            return 0;
        }
    }
    
    // --- SERVICE TRANSFER (0xBnnn) ---
    // ETG.5001.1: Service Transfer Area - Service objects (may be writable)
    else if ((slot = get_slot_from_index(index, MDP_SERVICE_AREA_START)) >= 0) {
        if (subindex == 0x01) {
            if (mdp_slots[slot].service_write_callback) {
                // Use callback if available
                uint32_t callback_abort = mdp_slots[slot].service_write_callback((uint8_t)slot, subindex, ptr, size);
                if (callback_abort != 0) {
                    *abort_code = callback_abort;
                    return 0;
                }
                return 1; // Callback handled the write
            } else if (mdp_slots[slot].service_transfer) {
                uint16_t slot_len = (mdp_slots[slot].service_size + 7) / 8;
                if (size > slot_len) {
                    *abort_code = 0x06070010;
                    return 0;
                }
                memcpy(mdp_slots[slot].service_transfer, ptr, size);
                return 1;
            } else {
                *abort_code = 0x06020000; // No service data available
                return 0;
            }
        }
        else {
            *abort_code = 0x06020000; // Invalid subindex
            return 0;
        }
    }

    // Device Area (0xF0xx) objects are typically Read-Only
    // Diagnosis Area (0xAnnn) is typically Read-Only
    // Configuration changes should be done through module-specific configuration objects

    return 0; // Not handled by MDP
}