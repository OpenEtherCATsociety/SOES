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
 */

#include "esc_mdp.h"
#include <string.h>
#include "esc_coe.h"

mdp_module_t mdp_slots[MAX_MDP_SLOTS];
uint8_t mdp_active_modules_count = 0;

void MDP_init(void) {
    memset(mdp_slots, 0, sizeof(mdp_slots));
    mdp_active_modules_count = 0;
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
                                config_read_cb, config_write_cb, NULL, NULL, NULL);
}

int MDP_add_module_full(uint8_t slot, uint32_t module_id,
                        void *in, uint16_t in_size,
                        void *out, uint16_t out_size,
                        void *config, uint16_t config_size,
                        void *diagnosis, uint16_t diagnosis_size,
                        void *service, uint16_t service_size,
                        uint32_t (*config_read_cb)(uint8_t slot, uint8_t subindex, void *data, uint16_t *size),
                        uint32_t (*config_write_cb)(uint8_t slot, uint8_t subindex, void *data, uint16_t size),
                        uint32_t (*diagnosis_read_cb)(uint8_t slot, uint8_t subindex, void *data, uint16_t *size),
                        uint32_t (*service_read_cb)(uint8_t slot, uint8_t subindex, void *data, uint16_t *size),
                        uint32_t (*service_write_cb)(uint8_t slot, uint8_t subindex, void *data, uint16_t size)) {
    if (slot >= MAX_MDP_SLOTS) return -1;

    mdp_slots[slot].module_ident = module_id;
    mdp_slots[slot].input_data = in;
    mdp_slots[slot].input_size = in_size;
    mdp_slots[slot].output_data = out;
    mdp_slots[slot].output_size = out_size;
    mdp_slots[slot].config_data = config;
    mdp_slots[slot].config_size = config_size;
    mdp_slots[slot].diagnosis_data = diagnosis;
    mdp_slots[slot].diagnosis_size = diagnosis_size;
    mdp_slots[slot].service_data = service;
    mdp_slots[slot].service_size = service_size;
    mdp_slots[slot].config_read_callback = config_read_cb;
    mdp_slots[slot].config_write_callback = config_write_cb;
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
            if (mdp_slots[slot].config_data || mdp_slots[slot].config_read_callback) {
                temp_u8 = 1; // At least subindex 1 exists
                data_ptr = &temp_u8; data_len = 1;
            } else {
                *abort_code = 0x06020000; return 0; // No configuration available
            }
        }
        else if (subindex == 0x01) {
            // Configuration data
            if (mdp_slots[slot].config_read_callback) {
                // Use callback if available
                uint16_t callback_size = *size;
                uint32_t callback_abort = mdp_slots[slot].config_read_callback((uint8_t)slot, subindex, ptr, &callback_size);
                if (callback_abort != 0) {
                    *abort_code = callback_abort;
                    return 0;
                }
                *size = callback_size;
                return 1; // Callback handled the data
            } else if (mdp_slots[slot].config_data) {
                data_ptr = (uint8_t*)mdp_slots[slot].config_data;
                data_len = (mdp_slots[slot].config_size + 7) / 8;
            } else {
                *abort_code = 0x06020000; return 0;
            }
        }
        else {
            *abort_code = 0x06020000; return 0;
        }
    }
    // --- MODULE INFO (0x9nn0) ---
    // ETG.5001.1: Information Area - Scanned information from modules
    else if ((slot = get_slot_from_index(index, MDP_INFO_AREA_START)) >= 0) {
        // ETG.5001.1: 0x9nn0 Sub 1 = Module Profile / Ident
        if (subindex == 0x00) {
            temp_u8 = 1; // Max subindex (at least subindex 1 exists)
            data_ptr = &temp_u8; data_len = 1;
        }
        else if (subindex == 0x01) {
             temp_u32 = mdp_slots[slot].module_ident;
             data_ptr = (uint8_t*)&temp_u32;
             data_len = 4;
        }
        else {
             *abort_code = 0x06020000; return 0;
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
                *abort_code = 0x06020000; return 0; // No diagnosis data available
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
                *abort_code = 0x06020000; return 0;
            }
        }
        else {
            *abort_code = 0x06020000; return 0;
        }
    }
    // --- SERVICE TRANSFER (0xBnnn) ---
    // ETG.5001.1: Service Transfer Area - Service objects
    else if ((slot = get_slot_from_index(index, MDP_SERVICE_AREA_START)) >= 0) {
        if (subindex == 0x00) {
            // Subindex 0: Number of entries
            if (mdp_slots[slot].service_data || mdp_slots[slot].service_read_callback) {
                temp_u8 = 1; // At least subindex 1 exists
                data_ptr = &temp_u8; data_len = 1;
            } else {
                *abort_code = 0x06020000; return 0; // No service data available
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
            } else if (mdp_slots[slot].service_data) {
                data_ptr = (uint8_t*)mdp_slots[slot].service_data;
                data_len = (mdp_slots[slot].service_size + 7) / 8;
            } else {
                *abort_code = 0x06020000; return 0;
            }
        }
        else {
            *abort_code = 0x06020000; return 0;
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
            if (mdp_slots[slot].config_write_callback) {
                // Use callback if available
                uint32_t callback_abort = mdp_slots[slot].config_write_callback((uint8_t)slot, subindex, ptr, size);
                if (callback_abort != 0) {
                    *abort_code = callback_abort;
                    return 0;
                }
                return 1; // Callback handled the write
            } else if (mdp_slots[slot].config_data) {
                uint16_t slot_len = (mdp_slots[slot].config_size + 7) / 8;
                if (size > slot_len) {
                    *abort_code = 0x06070010;
                    return 0;
                }
                memcpy(mdp_slots[slot].config_data, ptr, size);
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
            } else if (mdp_slots[slot].service_data) {
                uint16_t slot_len = (mdp_slots[slot].service_size + 7) / 8;
                if (size > slot_len) {
                    *abort_code = 0x06070010;
                    return 0;
                }
                memcpy(mdp_slots[slot].service_data, ptr, size);
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