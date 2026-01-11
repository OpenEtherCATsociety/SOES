/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

/** \file
 * \brief
 * Object Dictionary Areas for Modular Device Profile (MDP)
 *
 * Implementation according to ETG.5001.1 S (D) V0.9.0
 * Part 1: General MDP Device Model
 *
 * This module provides object dictionary definitions for Device Area (0xF000-0xFFFF)
 * and dynamic dictionary management for Module Areas (0x6000-0xBFFF) according to
 * ETG.5001.1 Modular Device Profile specification.
 */

#include "esc_coe.h"
#include "device_obj_areas.h"
#include "options.h"
#ifdef USE_MDP
#include "esc_mdp.h"
#include <stdlib.h>

/* External MDP variables */
extern mdp_module_t mdp_slots[];
extern uint8_t mdp_active_modules_count;

/* ============================================================================
 * ETG.5001.1 Clause 4.2.3: Device Area (0xF000-0xFFFF)
 * Object Dictionary definitions for Device Area objects
 * ============================================================================ */

/* String constants for object names */
static const char acNameARR[] = "Number of Entries";

/* 0xF000: Modular Device Profile */
static const char acNameF000[]    = "Module Device Profile";
static const char acNameF000_00[] = "Max SubIndex";
static const char acNameF000_01[] = "Index Distance";
static const char acNameF000_02[] = "Maximum Number of Modules";
static const char acNameF000_03[] = "General configuration";
static const char acNameF000_04[] = "General information";
static const char acNameF000_05[] = "Module PDO Group of device";

/* 0xF002: Detect Modules Command */
static const char acNameF002[]    = "Detect Modules Command";
static const char acNameF002_00[] = "Max SubIndex";
static const char acNameF002_01[] = "Command";

/* 0xF00E: Module PDO Group Mapping Alignment PDO Number */
static const char acNameF00E[]    = "Module PDO Group Mapping Alignment PDO Number";
static const char acNameF00E_0x[] = "Alignment PDO";

/* 0xF00F: Module PDO Group Mapping Alignment */
static const char acNameF00F[]    = "Module PDO Group Mapping Alignment";
static const char acNameF00F_0x[] = "Alignment";

/* 0xF010-0xF01F: Module Profile List */
static const char acNameF010[]    = "Module Profile List";
static const char acNameF010_0x[] = "Profile";

/* 0xF020-0xF02F: Address list of the configured Modules */
static const char acNameF020[]    = "Address list of the configured Modules";
static const char acNameF020_0x[] = "Module address";

/* 0xF030-0xF03F: Module Ident list of the configured Modules */
static const char acNameF030[]    = "Module Ident list of the configured Modules";
static const char acNameF030_0x[] = "Module ident";

/* 0xF040-0xF04F: Address list of detected Modules */
static const char acNameF040[]    = "Address list of detected Modules";
static const char acNameF040_0x[] = "Module address";

/* 0xF050-0xF05F: Module Ident list of detected Modules */
static const char acNameF050[]    = "Module Ident list of detected Modules";
static const char acNameF050_0x[] = "Module ident";

/* 0xF100-0xF10F: Device Status */
static const char acNameF100[]    = "Device Status";
static const char acNameF100_00[] = "Max SubIndex";
static const char acNameF100_01[] = "Status";

/* 0xF110-0xF11F: Device Diagnosis */
static const char acNameF110[]    = "Device Diagnosis";
static const char acNameF110_00[] = "Max SubIndex";
static const char acNameF110_01[] = "Diagnosis";

/* 0xF120-0xF12F: Device Statistics */
static const char acNameF120[]    = "Device Statistics";
static const char acNameF120_00[] = "Max SubIndex";
static const char acNameF120_01[] = "Statistics";

/* 0xF200-0xF20F: Device Control */
static const char acNameF200[]    = "Device Control";
static const char acNameF200_00[] = "Max SubIndex";
static const char acNameF200_01[] = "Control";

/* 0xF600-0xF6FF: Input Area of device itself */
static const char acNameF600[]    = "Input Area of device itself";
static const char acNameF600_00[] = "Max SubIndex";
static const char acNameF600_01[] = "Input Data";

/* 0xF700-0xF7FF: Output Area of device itself */
static const char acNameF700[]    = "Output Area of device itself";
static const char acNameF700_00[] = "Max SubIndex";
static const char acNameF700_01[] = "Output Data";

/* 0xF800-0xF8FF: Device Configuration Parameter Area */
static const char acNameF800[]    = "Device Configuration Parameter Area";
static const char acNameF800_00[] = "Max SubIndex";
static const char acNameF800_01[] = "Configuration Parameter";

/* 0xFB00-0xFBFF: Device Command */
static const char acNameFB00[]    = "Device Command";
static const char acNameFB00_00[] = "Max SubIndex";
static const char acNameFB00_01[] = "Command";

/* Forward declarations for MDP data access functions */
static uint16_t mdp_get_profile_list_entry(uint8_t subindex);
static uint16_t mdp_get_config_address_entry(uint8_t subindex);
static uint32_t mdp_get_config_ident_entry(uint8_t subindex);
static uint16_t mdp_get_detect_address_entry(uint8_t subindex);
static uint32_t mdp_get_detect_ident_entry(uint8_t subindex);
static uint16_t mdp_get_alignment_pdo_entry(uint8_t subindex);
static uint16_t mdp_get_alignment_entry(uint8_t subindex);

/* ============================================================================
 * ETG.5001.1 Clause 4.2.3.2: Object 0xF000: Modular Device Profile
 * Contains device-level MDP parameters
 * ============================================================================ */
const _objd SDOF000[] =
{
  /* Subindex 0: Number of Entries (Max SubIndex) */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameF000_00, 5, NULL},
  /* Subindex 1: Index Distance - Distance between module indices */
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acNameF000_01, MDP_INDEX_STEP, NULL},
  /* Subindex 2: Maximum Number of Modules */
  {0x02, DTYPE_UNSIGNED16, 16, ATYPE_RO, acNameF000_02, MAX_MDP_SLOTS, NULL},
  /* Subindex 3: General configuration */
  {0x03, DTYPE_UNSIGNED32, 32, ATYPE_RO, acNameF000_03, 0, NULL},
  /* Subindex 4: General information */
  {0x04, DTYPE_UNSIGNED32, 32, ATYPE_RO, acNameF000_04, 0, NULL},
  /* Subindex 5: Module PDO Group of device */
  {0x05, DTYPE_UNSIGNED16, 16, ATYPE_RO, acNameF000_05, 0, NULL},
};

/* ============================================================================
 * ETG.5001.1 Clause 4.2.3.3: Object 0xF002: Detect Modules Command
 * Command object for module detection
 * ============================================================================ */
const _objd SDOF002[] = {
  /* Subindex 0: Number of Entries */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameF002_00, 1, NULL},
  /* Subindex 1: Command - Write value to trigger module detection */
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_WO, acNameF002_01, 0, NULL},
};

/* ============================================================================
 * ETG.5001.1 Clause 4.2.3.4: Object 0xF00E: Module PDO Group Mapping Alignment PDO Number
 * Array of PDO numbers for alignment
 * Note: Subindexes 1-255 are accessed dynamically via callback
 * ============================================================================ */
const _objd SDOF00E[] = {
  /* Subindex 0: Number of Entries */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameARR, MAX_MDP_SLOTS, NULL},
  /* Subindexes 1-N: Access via MDP_read_access callback */
};

/* ============================================================================
 * ETG.5001.1 Clause 4.2.3.5: Object 0xF00F: Module PDO Group Mapping Alignment
 * Array of alignment values
 * Note: Subindexes 1-255 are accessed dynamically via callback
 * ============================================================================ */
const _objd SDOF00F[] = {
  /* Subindex 0: Number of Entries */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameARR, MAX_MDP_SLOTS, NULL},
  /* Subindexes 1-N: Access via MDP_read_access callback */
};

/* ============================================================================
 * ETG.5001.1 Clause 4.2.3.6: Module Profile List (0xF010...0xF01F)
 * Profile information of the modules
 * Note: Subindexes 1-255 are accessed dynamically via callback
 * ============================================================================ */
const _objd SDOF010[] = {
  /* Subindex 0: Number of Entries */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameARR, MAX_MDP_SLOTS, NULL},
  /* Subindexes 1-N: Access via MDP_read_access callback */
};

/* ============================================================================
 * ETG.5001.1 Clause 4.2.3.7: Configured Address List (0xF020...0xF02F)
 * List of addresses for configured modules
 * Note: Subindexes 1-255 are accessed dynamically via callback
 * ============================================================================ */
const _objd SDOF020[] = {
  /* Subindex 0: Number of Entries */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameARR, MAX_MDP_SLOTS, NULL},
  /* Subindexes 1-N: Access via MDP_read_access callback */
};

/* ============================================================================
 * ETG.5001.1 Clause 4.2.3.8: Configured Module Ident List (0xF030...0xF03F)
 * List of module identifiers currently configured in slots
 * ============================================================================ */
const _objd SDOF030[] =
{
  /* Subindex 0: Number of Entries */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameARR, MAX_MDP_SLOTS, NULL},
  /* Subindexes 1-N: Access via MDP_read_access callback */
};

/* ============================================================================
 * ETG.5001.1 Clause 4.2.3.9: Detected Address List (0xF040...0xF04F)
 * List of addresses for detected modules
 * Note: Subindexes 1-255 are accessed dynamically via callback
 * ============================================================================ */
const _objd SDOF040[] = {
  /* Subindex 0: Number of Entries */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameARR, MAX_MDP_SLOTS, NULL},
  /* Subindexes 1-N: Access via MDP_read_access callback */
};

/* ============================================================================
 * ETG.5001.1 Clause 4.2.3.10: Detected Module Ident List (0xF050...0xF05F)
 * List of module identifiers for detected modules
 * ============================================================================ */
const _objd SDOF050[] =
{
  /* Subindex 0: Number of Entries */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameARR, MAX_MDP_SLOTS, NULL},
  /* Subindexes 1-N: Access via MDP_read_access callback */
};

/* ============================================================================
 * ETG.5001.1 Clause 4.2.3.11: Device Status (0xF100...0xF10F)
 * Device status information, TxPDO mappable
 * ============================================================================ */
const _objd SDOF100[] =
{
  /* Subindex 0: Number of Entries */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameF100_00, 1, NULL},
  /* Subindex 1: Status - Device status value */
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO | ATYPE_TXPDO, acNameF100_01, 0, NULL},
};

/* ============================================================================
 * ETG.5001.1 Clause 4.2.3.12: Device Diagnosis (0xF110...0xF11F)
 * Device diagnosis information
 * ============================================================================ */
const _objd SDOF110[] = {
  /* Subindex 0: Number of Entries */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameF110_00, 1, NULL},
  /* Subindex 1: Diagnosis - Device diagnosis value */
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acNameF110_01, 0, NULL},
};

/* ============================================================================
 * ETG.5001.1 Clause 4.2.3.13: Device Statistics (0xF120...0xF12F)
 * Device statistics information
 * ============================================================================ */
const _objd SDOF120[] = {
  /* Subindex 0: Number of Entries */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameF120_00, 1, NULL},
  /* Subindex 1: Statistics - Device statistics value */
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, acNameF120_01, 0, NULL},
};

/* ============================================================================
 * ETG.5001.1 Clause 4.2.3.14: Device Control (0xF200...0xF20F)
 * Device control information, RxPDO mappable
 * ============================================================================ */
const _objd SDOF200[] =
{
  /* Subindex 0: Number of Entries */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameF200_00, 1, NULL},
  /* Subindex 1: Control - Device control value */
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RW | ATYPE_RXPDO, acNameF200_01, 0, NULL},
};

/* ============================================================================
 * ETG.5001.1 Clause 4.2.3.15: Device Input Data (0xF600...0xF6FF)
 * Input Area of device itself, TxPDO mappable
 * ============================================================================ */
const _objd SDOF600[] = {
  /* Subindex 0: Number of Entries */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameF600_00, 1, NULL},
  /* Subindex 1: Input Data - Device input data */
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO | ATYPE_TXPDO, acNameF600_01, 0, NULL},
};

/* ============================================================================
 * ETG.5001.1 Clause 4.2.3.16: Device Output Data (0xF700...0xF7FF)
 * Output Area of device itself, RxPDO mappable
 * ============================================================================ */
const _objd SDOF700[] = {
  /* Subindex 0: Number of Entries */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameF700_00, 1, NULL},
  /* Subindex 1: Output Data - Device output data */
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RW | ATYPE_RXPDO, acNameF700_01, 0, NULL},
};

/* ============================================================================
 * ETG.5001.1 Clause 4.2.3.17: Device Configuration Data (0xF800...0xF8FF)
 * Device Configuration Parameter Area
 * ============================================================================ */
const _objd SDOF800[] = {
  /* Subindex 0: Number of Entries */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameF800_00, 1, NULL},
  /* Subindex 1: Configuration Parameter - Device configuration parameter */
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RW, acNameF800_01, 0, NULL},
};

/* ============================================================================
 * ETG.5001.1 Clause 4.2.3.18: Device Command (0xFB00...0xFBFF)
 * Device Command area
 * ============================================================================ */
const _objd SDOFB00[] =
{
  /* Subindex 0: Number of Entries */
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameFB00_00, 1, NULL},
  /* Subindex 1: Command - Device command value */
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_WO, acNameFB00_01, 0, NULL},
};

/* ============================================================================
 * Static Object Dictionary for Device Area (0xF000-0xFFFF)
 * According to ETG.5001.1 Table 3: Object Dictionary Structure
 * ============================================================================ */
const _objectlist DeviceAreaobjects[] =
{
  /* 0xF000: Modular Device Profile - Mandatory for all MDP device types */
  {0xF000, OTYPE_RECORD, 5, 0, acNameF000, SDOF000},
  
  /* 0xF002: Detect Modules Command */
  {0xF002, OTYPE_RECORD, 1, 0, acNameF002, SDOF002},
  
  /* 0xF00E: Module PDO Group Mapping Alignment PDO Number */
  {0xF00E, OTYPE_ARRAY, MAX_MDP_SLOTS, 0, acNameF00E, SDOF00E},
  
  /* 0xF00F: Module PDO Group Mapping Alignment */
  {0xF00F, OTYPE_ARRAY, MAX_MDP_SLOTS, 0, acNameF00F, SDOF00F},
  
  /* 0xF010: Module Profile List - Profile information of the modules */
  {0xF010, OTYPE_ARRAY, MAX_MDP_SLOTS, 0, acNameF010, SDOF010},
  
  /* 0xF020: Address list of the configured Modules */
  {0xF020, OTYPE_ARRAY, MAX_MDP_SLOTS, 0, acNameF020, SDOF020},
  
  /* 0xF030: Module Ident list of the configured Modules */
  {0xF030, OTYPE_ARRAY, MAX_MDP_SLOTS, 0, acNameF030, SDOF030},
  
  /* 0xF040: Address list of detected Modules */
  {0xF040, OTYPE_ARRAY, MAX_MDP_SLOTS, 0, acNameF040, SDOF040},
  
  /* 0xF050: Module Ident list of detected Modules */
  {0xF050, OTYPE_ARRAY, MAX_MDP_SLOTS, 0, acNameF050, SDOF050},
  
  /* 0xF100: Device Status - TxPDO mappable */
  {0xF100, OTYPE_RECORD, 1, 0, acNameF100, SDOF100},
  
  /* 0xF110: Device Diagnosis */
  {0xF110, OTYPE_RECORD, 1, 0, acNameF110, SDOF110},
  
  /* 0xF120: Device Statistics */
  {0xF120, OTYPE_RECORD, 1, 0, acNameF120, SDOF120},
  
  /* 0xF200: Device Control - RxPDO mappable */
  {0xF200, OTYPE_RECORD, 1, 0, acNameF200, SDOF200},
  
  /* 0xF600: Input Area of device itself - TxPDO mappable */
  {0xF600, OTYPE_RECORD, 1, 0, acNameF600, SDOF600},
  
  /* 0xF700: Output Area of device itself - RxPDO mappable */
  {0xF700, OTYPE_RECORD, 1, 0, acNameF700, SDOF700},
  
  /* 0xF800: Device Configuration Parameter Area */
  {0xF800, OTYPE_RECORD, 1, 0, acNameF800, SDOF800},
  
  /* 0xFB00: Device Command */
  {0xFB00, OTYPE_RECORD, 1, 0, acNameFB00, SDOFB00},
  
  /* End marker */
  {0xffff, 0xff, 0xff, 0xff, NULL, NULL}
};

/* ============================================================================
 * Dynamic Object Dictionary Management for Module Areas (0x6000-0xBFFF)
 * ETG.5001.1 Clause 5.2: Module Area
 * ============================================================================ */

/* Maximum number of dynamic dictionary entries */
#define MAX_DYNAMIC_DICT_ENTRIES (MAX_MDP_SLOTS * 6) /* 6 areas per module */

/**
 * @brief Structure for storing dynamic dictionary entries
 * 
 * Stores dynamically created object dictionary entries for module areas.
 * Each module can have entries in 6 different areas (Input, Output, Config, Info, Diagnosis, Service).
 */
typedef struct {
    _objectlist entry;          /**< Object dictionary entry */
    uint8_t slot;               /**< Module slot number */
    uint8_t area_type;          /**< Area type (0=INPUT, 1=OUTPUT, 2=CONFIG, 3=INFO, 4=DIAGNOSIS, 5=SERVICE) */
    _objd *objd_array;          /**< Pointer to dynamically allocated _objd array */
    uint8_t objd_count;         /**< Number of elements in objd_array */
    char **name_strings;        /**< Pointer to array of name strings (for memory deallocation) */
    uint8_t name_count;         /**< Number of name strings */
} dynamic_dict_entry_t;

/* Static array for storing dynamic dictionary entries */
static dynamic_dict_entry_t dynamic_dict_entries[MAX_DYNAMIC_DICT_ENTRIES];
static uint16_t dynamic_dict_count = 0;

/* String constants for module area names */
static char module_input_name[] = "Module Input Area";
static char module_output_name[] = "Module Output Area";
static char module_config_name[] = "Module Configuration Area";
static char module_info_name[] = "Module Information Area";
static char module_diagnosis_name[] = "Module Diagnosis Area";
static char module_service_name[] = "Module Service Area";
static char module_subindex_00[] = "Number of Entries";
static char module_subindex_01[] = "Data";

static uint16_t mdp_get_profile_list_entry(uint8_t subindex) {
    if (subindex == 0 || subindex > MAX_MDP_SLOTS) {
        return 0;
    }
    uint8_t slot = subindex - 1;
    if (slot >= MAX_MDP_SLOTS || mdp_slots[slot].module_ident == 0) {
        return 0;
    }
    /* Return module profile - for now return module_ident, can be extended */
    return (uint16_t)(mdp_slots[slot].module_ident & 0xFFFF);
}

static uint16_t mdp_get_config_address_entry(uint8_t subindex) {
    if (subindex == 0 || subindex > MAX_MDP_SLOTS) {
        return 0;
    }
    uint8_t slot = subindex - 1;
    if (slot >= MAX_MDP_SLOTS || mdp_slots[slot].module_ident == 0) {
        return 0;
    }
    /* Calculate address based on slot number */
    return MDP_INPUT_AREA_START + (slot * MDP_INDEX_STEP);
}

static uint32_t mdp_get_config_ident_entry(uint8_t subindex) {
    if (subindex == 0 || subindex > MAX_MDP_SLOTS) {
        return 0;
    }
    uint8_t slot = subindex - 1;
    if (slot >= MAX_MDP_SLOTS) {
        return 0;
    }
    return mdp_slots[slot].module_ident;
}

static uint16_t mdp_get_detect_address_entry(uint8_t subindex) {
    /* Same as configured for now - can be extended for detection logic */
    return mdp_get_config_address_entry(subindex);
}

static uint32_t mdp_get_detect_ident_entry(uint8_t subindex) {
    /* Same as configured for now - can be extended for detection logic */
    return mdp_get_config_ident_entry(subindex);
}

static uint16_t mdp_get_alignment_pdo_entry(uint8_t subindex) {
    /* Default implementation - can be extended */
    (void)subindex;
    return 0x1A00;
}

static uint16_t mdp_get_alignment_entry(uint8_t subindex) {
    /* Default implementation - can be extended */
    (void)subindex;
    return 0;
}

static _objd* create_module_objd_array(uint8_t slot, uint8_t area_type, 
                                       void *data_ptr, uint16_t data_size,
                                       uint16_t access_type) {
    _objd *objd_array = (_objd*)malloc(2 * sizeof(_objd));
    if (!objd_array) {
        return NULL;
    }
    
    /* Subindex 0: Number of Entries */
    objd_array[0].subindex = 0x00;
    objd_array[0].datatype = DTYPE_UNSIGNED8;
    objd_array[0].bitlength = 8;
    objd_array[0].flags = ATYPE_RO;
    objd_array[0].name = module_subindex_00;
    objd_array[0].value = (data_ptr != NULL) ? 1 : 0;
    objd_array[0].data = NULL;
    
    /* Subindex 1: Data (if data exists) */
    if (data_ptr != NULL) {
        objd_array[1].subindex = 0x01;
        
        /* Determine data type based on size */
        if (data_size <= 8) {
            objd_array[1].datatype = DTYPE_UNSIGNED8;
            objd_array[1].bitlength = 8;
        } else if (data_size <= 16) {
            objd_array[1].datatype = DTYPE_UNSIGNED16;
            objd_array[1].bitlength = 16;
        } else if (data_size <= 32) {
            objd_array[1].datatype = DTYPE_UNSIGNED32;
            objd_array[1].bitlength = 32;
        } else {
            objd_array[1].datatype = DTYPE_OCTET_STRING;
            objd_array[1].bitlength = data_size;
        }
        
        objd_array[1].flags = access_type;
        objd_array[1].name = module_subindex_01;
        objd_array[1].value = 0;
        objd_array[1].data = data_ptr;
    }
    
    return objd_array;
}

static uint16_t calculate_module_index(uint8_t slot, uint8_t area_type) {
    uint16_t base_addr;
    
    switch(area_type) {
        case 0: base_addr = MDP_INPUT_AREA_START; break;      /* 0x6000 */
        case 1: base_addr = MDP_OUTPUT_AREA_START; break;     /* 0x7000 */
        case 2: base_addr = MDP_CONFIG_AREA_START; break;    /* 0x8000 */
        case 3: base_addr = MDP_INFO_AREA_START; break;       /* 0x9000 */
        case 4: base_addr = MDP_DIAGNOSIS_AREA_START; break; /* 0xA000 */
        case 5: base_addr = MDP_SERVICE_AREA_START; break;   /* 0xB000 */
        default: return 0xFFFF;
    }
    
    return base_addr + (slot * MDP_INDEX_STEP);
}

int device_obj_areas_add_module_entry(uint8_t slot, uint8_t area_type,
                                      void *data_ptr, uint16_t data_size,
                                      uint16_t access_type) {
    if (slot >= MAX_MDP_SLOTS || area_type > 5) {
        return -1;
    }
    
    if (dynamic_dict_count >= MAX_DYNAMIC_DICT_ENTRIES) {
        return -1;
    }
    
    /* Check if entry already exists */
    for (uint16_t i = 0; i < dynamic_dict_count; i++) {
        if (dynamic_dict_entries[i].slot == slot && 
            dynamic_dict_entries[i].area_type == area_type) {
            /* Entry exists, update it */
            if (dynamic_dict_entries[i].objd_array) {
                free(dynamic_dict_entries[i].objd_array);
            }
            dynamic_dict_entries[i].objd_array = create_module_objd_array(
                slot, area_type, data_ptr, data_size, access_type);
            if (!dynamic_dict_entries[i].objd_array) {
                return -1;
            }
            dynamic_dict_entries[i].objd_count = (data_ptr != NULL) ? 2 : 1;
            dynamic_dict_entries[i].entry.objdesc = dynamic_dict_entries[i].objd_array;
            dynamic_dict_entries[i].entry.maxsub = (data_ptr != NULL) ? 1 : 0;
            return 0;
        }
    }
    
    /* Create new entry */
    dynamic_dict_entry_t *entry = &dynamic_dict_entries[dynamic_dict_count];
    
    entry->objd_array = create_module_objd_array(slot, area_type, data_ptr, data_size, access_type);
    if (!entry->objd_array) {
        return -1;
    }
    
    entry->slot = slot;
    entry->area_type = area_type;
    entry->objd_count = (data_ptr != NULL) ? 2 : 1;
    entry->name_strings = NULL;
    entry->name_count = 0;
    
    /* Determine area name */
    const char *area_name;
    switch(area_type) {
        case 0: area_name = module_input_name; break;
        case 1: area_name = module_output_name; break;
        case 2: area_name = module_config_name; break;
        case 3: area_name = module_info_name; break;
        case 4: area_name = module_diagnosis_name; break;
        case 5: area_name = module_service_name; break;
        default: area_name = "Unknown";
    }
    
    /* Fill _objectlist structure */
    entry->entry.index = calculate_module_index(slot, area_type);
    entry->entry.objtype = OTYPE_RECORD;
    entry->entry.maxsub = (data_ptr != NULL) ? 1 : 0;
    entry->entry.pad1 = 0;
    entry->entry.name = area_name;
    entry->entry.objdesc = entry->objd_array;
    
    dynamic_dict_count++;
    return 0;
}

int device_obj_areas_remove_module_entry(uint8_t slot, uint8_t area_type) {
    for (uint16_t i = 0; i < dynamic_dict_count; i++) {
        if (dynamic_dict_entries[i].slot == slot && 
            dynamic_dict_entries[i].area_type == area_type) {
            /* Free memory */
            if (dynamic_dict_entries[i].objd_array) {
                free(dynamic_dict_entries[i].objd_array);
            }
            if (dynamic_dict_entries[i].name_strings) {
                for (uint8_t j = 0; j < dynamic_dict_entries[i].name_count; j++) {
                    if (dynamic_dict_entries[i].name_strings[j]) {
                        free(dynamic_dict_entries[i].name_strings[j]);
                    }
                }
                free(dynamic_dict_entries[i].name_strings);
            }
            
            /* Shift remaining entries */
            for (uint16_t j = i; j < dynamic_dict_count - 1; j++) {
                dynamic_dict_entries[j] = dynamic_dict_entries[j + 1];
            }
            
            dynamic_dict_count--;
            return 0;
        }
    }
    return -1;
}

int device_obj_areas_remove_module_all_entries(uint8_t slot) {
    int removed = 0;
    for (uint8_t area_type = 0; area_type < 6; area_type++) {
        if (device_obj_areas_remove_module_entry(slot, area_type) == 0) {
            removed++;
            area_type--; /* Re-check this index after shift */
        }
    }
    return removed;
}

int device_obj_areas_init_module_dict(uint8_t slot) {
    if (slot >= MAX_MDP_SLOTS) {
        return -1;
    }
    
    mdp_module_t *module = &mdp_slots[slot];
    if (module->module_ident == 0) {
        return -1; /* Module not initialized */
    }
    
    int result = 0;
    
    /* Register Input Area (0x6nn0) */
    if (module->input_data && module->input_size > 0) {
        result |= device_obj_areas_add_module_entry(
            slot, 0, module->input_data, module->input_size, ATYPE_RO | ATYPE_TXPDO);
    }
    
    /* Register Output Area (0x7nn0) */
    if (module->output_data && module->output_size > 0) {
        result |= device_obj_areas_add_module_entry(
            slot, 1, module->output_data, module->output_size, ATYPE_RW | ATYPE_RXPDO);
    }
    
    /* Register Configuration Area (0x8nn0) */
    if (module->configuration_data && module->configuration_size > 0) {
        result |= device_obj_areas_add_module_entry(
            slot, 2, module->configuration_data, module->configuration_size, ATYPE_RW);
    }
    
    /* Register Information Area (0x9nn0) */
    /* Validate Information Area registration based on device subtype
     * According to ETG.5001.1, Information Area should only be registered for:
     * - DEVICE_TYPE_FIELDBUS_GATEWAY (0x01)
     * - DEVICE_TYPE_MODULAR_DEVICE (0x02)
     * - AddInfo = 0 (device supports no standardized module profile or multiple profiles)
     * Do NOT register Information Area for DEVICE_TYPE_MODULE_DEVICE (0x03)
     */
    uint8_t device_subtype = MDP_get_device_subtype();
    if (device_subtype != DEVICE_TYPE_MODULE_DEVICE && device_subtype != 0xFF) {
        /* Information Area is available for:
         * - Fieldbus Gateway (0x01)
         * - Modular Device (0x02)
         * - AddInfo = 0 (0x00) - device supports no standardized module profile or multiple profiles
         */
        result |= device_obj_areas_add_module_entry(
            slot, 3, NULL, 0, ATYPE_RO);
    }
    /* Information Area is NOT registered for:
     * - Module Device (0x03)
     * - Non-MDP devices (0xFF)
     */
    
    /* Register Diagnosis Area (0xAnnn) */
    if (module->diagnosis_data && module->diagnosis_size > 0) {
        result |= device_obj_areas_add_module_entry(
            slot, 4, module->diagnosis_data, module->diagnosis_size, ATYPE_RO);
    }
    
    /* Register Service Area (0xBnnn) */
    if (module->service_transfer && module->service_size > 0) {
        result |= device_obj_areas_add_module_entry(
            slot, 5, module->service_transfer, module->service_size, ATYPE_RW);
    }
    
    return (result == 0) ? 0 : -1;
}

const _objectlist* device_obj_areas_get_dynamic_entry(uint16_t index) {
    for (uint16_t i = 0; i < dynamic_dict_count; i++) {
        if (dynamic_dict_entries[i].entry.index == index) {
            return &dynamic_dict_entries[i].entry;
        }
    }
    return NULL;
}

void device_obj_areas_clear_all(void) {
    for (uint16_t i = 0; i < dynamic_dict_count; i++) {
        if (dynamic_dict_entries[i].objd_array) {
            free(dynamic_dict_entries[i].objd_array);
        }
        if (dynamic_dict_entries[i].name_strings) {
            for (uint8_t j = 0; j < dynamic_dict_entries[i].name_count; j++) {
                if (dynamic_dict_entries[i].name_strings[j]) {
                    free(dynamic_dict_entries[i].name_strings[j]);
                }
            }
            free(dynamic_dict_entries[i].name_strings);
        }
    }
    dynamic_dict_count = 0;
}

uint16_t device_obj_areas_get_dynamic_count(void) {
    return dynamic_dict_count;
}

const _objectlist* device_obj_areas_find_object_extended(uint16_t index, int32_t *static_idx) {
    /* Search in static dictionary first */
    int32_t n = 0;
    while (DeviceAreaobjects[n].index < index && DeviceAreaobjects[n].index != 0xffff) {
        n++;
    }
    if (DeviceAreaobjects[n].index == index) {
        if (static_idx) *static_idx = n;
        return &DeviceAreaobjects[n];
    }
    
    /* If not found in static, search in dynamic */
    if (static_idx) *static_idx = -1;
    return device_obj_areas_get_dynamic_entry(index);
}

/* ============================================================================
 * MDP Array Access Functions
 * These functions provide virtual access to array elements via callbacks
 * instead of static allocation, saving memory in embedded systems
 * ============================================================================ */

int device_obj_areas_read_array_element(uint16_t index, uint8_t subindex, 
                                        void *data_ptr, uint16_t *data_size) {
    uint16_t value16;
    uint32_t value32;
    uint8_t value8;
    
    if (subindex == 0) {
        /* Subindex 0: Number of Entries */
        value8 = mdp_active_modules_count;
        if (data_ptr && *data_size >= 1) {
            *(uint8_t*)data_ptr = value8;
            *data_size = 1;
            return 1;
        }
        return 0;
    }
    
    /* Handle array elements based on index */
    switch (index) {
        case 0xF010: /* Module Profile List */
            if (subindex > 0 && subindex <= MAX_MDP_SLOTS) {
                value16 = mdp_get_profile_list_entry(subindex);
                if (data_ptr && *data_size >= 2) {
                *(uint16_t*)data_ptr = value16;
                *data_size = 2;
                return 1;
            }
            }
            break;
            
        case 0xF020: /* Address list of configured Modules */
            if (subindex > 0 && subindex <= MAX_MDP_SLOTS) {
                value16 = mdp_get_config_address_entry(subindex);
                if (data_ptr && *data_size >= 2) {
                *(uint16_t*)data_ptr = value16;
                *data_size = 2;
                return 1;
            }
            }
            break;
            
        case 0xF030: /* Module Ident list of configured Modules */
            if (subindex > 0 && subindex <= MAX_MDP_SLOTS) {
                value32 = mdp_get_config_ident_entry(subindex);
                if (data_ptr && *data_size >= 4) {
                *(uint32_t*)data_ptr = value32;
                *data_size = 4;
                return 1;
            }
            }
            break;
            
        case 0xF040: /* Address list of detected Modules */
            if (subindex > 0 && subindex <= MAX_MDP_SLOTS) {
                value16 = mdp_get_detect_address_entry(subindex);
                if (data_ptr && *data_size >= 2) {
                *(uint16_t*)data_ptr = value16;
                *data_size = 2;
                return 1;
            }
            }
            break;
            
        case 0xF050: /* Module Ident list of detected Modules */
            if (subindex > 0 && subindex <= MAX_MDP_SLOTS) {
                value32 = mdp_get_detect_ident_entry(subindex);
                if (data_ptr && *data_size >= 4) {
                *(uint32_t*)data_ptr = value32;
                *data_size = 4;
                return 1;
            }
            }
            break;
            
        case 0xF00E: /* Module PDO Group Mapping Alignment PDO Number */
            if (subindex > 0 && subindex <= MAX_MDP_SLOTS) {
                value16 = mdp_get_alignment_pdo_entry(subindex);
                if (data_ptr && *data_size >= 2) {
                *(uint16_t*)data_ptr = value16;
                *data_size = 2;
                return 1;
            }
            }
            break;
            
        case 0xF00F: /* Module PDO Group Mapping Alignment */
            if (subindex > 0 && subindex <= MAX_MDP_SLOTS) {
                value16 = mdp_get_alignment_entry(subindex);
                if (data_ptr && *data_size >= 2) {
                *(uint16_t*)data_ptr = value16;
                *data_size = 2;
                return 1;
            }
            }
            break;
    }
    
    return 0; /* Not handled */
}

int device_obj_areas_write_array_element(uint16_t index, uint8_t subindex,
                                         void *data_ptr, uint16_t data_size) {
    /* For now, only 0xF030 (Module Ident list) supports write */
    if (index == 0xF030 && subindex > 0 && subindex <= MAX_MDP_SLOTS) {
        uint8_t slot = subindex - 1;
        if (slot < MAX_MDP_SLOTS && data_size == 4) {
            mdp_slots[slot].module_ident = *(uint32_t*)data_ptr;
            return 1;
        }
    }
    
    return 0; /* Not handled */
}

#endif /* USE_MDP */
