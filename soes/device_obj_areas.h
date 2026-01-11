/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

/** \file
 * \brief
 * Header file for Object Dictionary Areas for Modular Device Profile (MDP)
 *
 * Implementation according to ETG.5001.1 S (D) V0.9.0
 * Part 1: General MDP Device Model
 *
 * This module provides functions for dynamically creating and managing
 * object dictionary entries for Modular Device Profile (MDP) slave devices.
 */

#ifndef SOES_DEVICE_OBJ_AREAS_H
#define SOES_DEVICE_OBJ_AREAS_H

#include "options.h"
#include <stdint.h>
#include "esc_coe.h"

#ifdef USE_MDP

/**
 * @brief Registers dynamic dictionary entry for module area
 * 
 * ETG.5001.1 Clause 5.2: Creates object dictionary entry for specified module area.
 * Each module can have entries in 6 different areas according to ETG.5001.1:
 * - Area 0: Input Area (0x6nn0) - Objects mappable to TxPDOs
 * - Area 1: Output Area (0x7nn0) - Objects mappable to RxPDOs
 * - Area 2: Configuration Area (0x8nn0) - Configuration and setting objects
 * - Area 3: Information Area (0x9nn0) - Scanned information from modules
 * - Area 4: Diagnosis Area (0xAnnn) - Diagnostic, status, statistic info
 * - Area 5: Service Transfer Area (0xBnnn) - Service objects
 * 
 * @param slot Module slot number (0..MAX_MDP_SLOTS-1)
 * @param area_type Area type (0=INPUT, 1=OUTPUT, 2=CONFIG, 3=INFO, 4=DIAGNOSIS, 5=SERVICE)
 * @param data_ptr Pointer to module data (may be NULL for Information Area)
 * @param data_size Data size in bits
 * @param access_type Access type (ATYPE_RO, ATYPE_RW, ATYPE_TXPDO, ATYPE_RXPDO, etc.)
 * @return 0 on success, -1 on error
 */
int device_obj_areas_add_module_entry(uint8_t slot, uint8_t area_type, void *data_ptr, uint16_t data_size, uint16_t access_type);

/**
 * @brief Removes dynamic dictionary entry for module area
 * 
 * ETG.5001.1: Removes object dictionary entry for specified module area.
 * Frees associated memory and updates entry counter.
 * 
 * @param slot Module slot number
 * @param area_type Area type (0=INPUT, 1=OUTPUT, 2=CONFIG, 3=INFO, 4=DIAGNOSIS, 5=SERVICE)
 * @return 0 on success, -1 on error (entry not found)
 */
int device_obj_areas_remove_module_entry(uint8_t slot, uint8_t area_type);

/**
 * @brief Removes all dynamic dictionary entries for specified module
 * 
 * ETG.5001.1 Clause 5.2: Removes all 6 area entries (Input, Output, Config, Info, Diagnosis, Service)
 * for the specified module. Used when removing a module completely from the system.
 * 
 * @param slot Module slot number
 * @return Number of removed entries
 */
int device_obj_areas_remove_module_all_entries(uint8_t slot);

/**
 * @brief Initializes all module areas in dictionary based on MDP data
 * 
 * ETG.5001.1 Clause 5.2: Automatically registers all available module areas based on
 * data from mdp_slots[slot] structure. Used for quick dictionary initialization
 * after adding module via MDP_add_module* functions.
 * 
 * This function registers:
 * - Input Area (0x6nn0) if input_data is available
 * - Output Area (0x7nn0) if output_data is available
 * - Configuration Area (0x8nn0) if configuration_data is available
 * - Information Area (0x9nn0) - registered based on device subtype (not for DEVICE_TYPE_MODULE_DEVICE)
 * - Diagnosis Area (0xAnnn) if diagnosis_data is available
 * - Service Area (0xBnnn) if service_transfer is available
 * 
 * @param slot Module slot number
 * @return 0 on success, -1 on error
 */
int device_obj_areas_init_module_dict(uint8_t slot);

/**
 * @brief Gets dynamic dictionary entry by index
 * 
 * ETG.5001.1: Used for searching dynamic entries in object dictionary.
 * Can be integrated with SDO_findobject for dynamic object support.
 * 
 * @param index Object dictionary index (e.g., 0x6000, 0x7000, etc.)
 * @return Pointer to entry or NULL if not found
 */
const _objectlist* device_obj_areas_get_dynamic_entry(uint16_t index);

/**
 * @brief Clears all dynamic dictionary entries
 * 
 * ETG.5001.1: Frees all allocated memory and resets entry counter.
 * Used during deinitialization or complete dictionary rebuild.
 */
void device_obj_areas_clear_all(void);

/**
 * @brief Gets number of dynamic dictionary entries
 * 
 * Returns the current number of active dynamic dictionary entries.
 * 
 * @return Number of active dynamic entries
 */
uint16_t device_obj_areas_get_dynamic_count(void);

/**
 * @brief Extended object search in dictionary (static + dynamic)
 * 
 * ETG.5001.1: Searches object first in static SDOobjects dictionary, then in dynamic.
 * Can be used as extension for SDO_findobject for dynamic module object support.
 * 
 * @param index Object dictionary index
 * @param static_idx Pointer to return static array index (or -1 if not found in static)
 * @return Pointer to _objectlist entry or NULL if not found
 */
const _objectlist* device_obj_areas_find_object_extended(uint16_t index, int32_t *static_idx);

/**
 * @brief Reads array element for Device Area arrays
 * 
 * ETG.5001.1 Clause 4.2.3: Provides virtual access to array elements (0xF010, 0xF020, etc.)
 * Elements are computed dynamically based on mdp_slots data instead of static allocation.
 * This saves memory in embedded systems.
 * 
 * Supported arrays:
 * - 0xF010: Module Profile List
 * - 0xF020: Address list of configured Modules
 * - 0xF030: Module Ident list of configured Modules
 * - 0xF040: Address list of detected Modules
 * - 0xF050: Module Ident list of detected Modules
 * - 0xF00E: Module PDO Group Mapping Alignment PDO Number
 * - 0xF00F: Module PDO Group Mapping Alignment
 * 
 * @param index Object dictionary index (0xF010, 0xF020, 0xF030, 0xF040, 0xF050, 0xF00E, 0xF00F)
 * @param subindex Subindex (element number, 0 = Number of Entries)
 * @param data_ptr Pointer to store read data
 * @param data_size Pointer to data size (input: max size, output: actual size)
 * @return 1 if handled, 0 if not handled
 */
int device_obj_areas_read_array_element(uint16_t index, uint8_t subindex, void *data_ptr, uint16_t *data_size);

/**
 * @brief Writes array element for Device Area arrays
 * 
 * ETG.5001.1 Clause 4.2.3: Provides virtual write access to array elements.
 * Currently supports writing to 0xF030 (Module Ident list of configured Modules).
 * 
 * @param index Object dictionary index
 * @param subindex Subindex (element number)
 * @param data_ptr Pointer to data to write
 * @param data_size Data size
 * @return 1 if handled, 0 if not handled
 */
int device_obj_areas_write_array_element(uint16_t index, uint8_t subindex, void *data_ptr, uint16_t data_size);

#endif /* USE_MDP */

#endif /* SOES_DEVICE_OBJ_AREAS_H */
