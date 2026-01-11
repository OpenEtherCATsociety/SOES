//
// Created by Mikhail Gladyshev on 28/12/25.
//
#include "esc_coe.h"
#include "utypes.h"
#include "esc_mdp.h"
#include "device_obj_areas.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const char acNameARR[]    = "Number of Entries";

static const char acNameF000[]    = "Module Device Profile";
static const char acNameF000_00[] = "Max SubIndex";
static const char acNameF000_01[] = "Index Distance";
static const char acNameF000_02[] = "Maximum Number of Modules";
static const char acNameF000_03[] = "General configuration";
static const char acNameF000_04[] = "General information";
static const char acNameF000_05[] = "Module PDO Group of device";

static const char acNameF002[]    = "Detect Modules Command";

static const char acNameF00E[]    = "Module PDO Group Mapping Alignment PDO Number";
static const char acNameF00E_0x[]    = "Alignment PDO";

static const char acNameF00F[]    = "Module PDO Group Mapping Alignment";
static const char acNameF00F_0x[]    = "Alignment";

static const char acNameF010[]    = "Module Profile List"; // Profile information of the modules
static const char acNameF010_0x[]    = "Profile";

static const char acNameF020[]    = "Address list of the configured Modules";
static const char acNameF020_0x[]    = "Module address";

static const char acNameF030[]    = "Module Ident list of the configured Modules";
static const char acNameF030_0x[]    = "Module ident";

static const char acNameF040[]    = "Address list of detected Modules";
static const char acNameF040_0x[]    = "Module address";

static const char acNameF050[]    = "Module Ident list of detected Modules";
static const char acNameF050_0x[]    = "Module ident";

static const char acNameF100[]    = "Device Status";      // TxPDO mappable
static const char acNameF110[]    = "Device Diagnosis";
static const char acNameF120[]    = "Device Statistics";

static const char acNameF200[]    = "Device Control";     // RxPDO mappable

static const char acNameF600[]    = "Input Area of device itself";  // TxPDO mappable
static const char acNameF700[]    = "Output Area of device itself"; // RxPDO mappable
static const char acNameF800[]    = "Device Configuration Parameter Area";
static const char acNameFB00[]    = "Device Command";

/* --- Структуры подобъектов (_objd) --- */

// 0xF000: Modular Device Profile (Обязателен для всех типов)
const _objd SDOF000[] =
{
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameF000_00, 5, NULL},//TODO maybe need to delete
  {0x01, DTYPE_UNSIGNED16,  16, ATYPE_RO, acNameF000_01, 16, NULL},
  {0x02, DTYPE_UNSIGNED16,  32, ATYPE_RO, acNameF000_02, 0, NULL},
  {0x03, DTYPE_UNSIGNED32,  32, ATYPE_RO, acNameF000_03, 0, NULL},
  {0x04, DTYPE_UNSIGNED32,  16, ATYPE_RO, acNameF000_04, 0, NULL},
  {0x05, DTYPE_UNSIGNED16,  16, ATYPE_RO, acNameF000_05, 0, NULL},
};
const _objd SDOF002[] = {
  //TODO check what i need here
};

const _objd SDOF00E[] = {
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameARR, 255, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acNameF00E_0x + " num", 0x1A00, NULL},
  //TODO add 254 more elements
};
const _objd SDOF00F[] = {
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameARR, 255, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acNameF00F_0x + " num", 0x1A00, NULL},
  //TODO add 254 more elements
};
const _objd SDOF010[] = {
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameARR, 255, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acNameF010_0x + " num", 0x1A00, NULL},
  //TODO add 254 more elements
};
const _objd SDOF020[] = {
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameARR, 255, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acNameF020_0x + " num",0x1A00, NULL},
  //TODO add 254 more elements
};
const _objd SDOF030[] =
{
  {0x00, DTYPE_UNSIGNED8,   8,  ATYPE_RO, acNameARR, 255, &Obj.Mdp.nConfiguredModules},
  {0x01, DTYPE_UNSIGNED32,  32, ATYPE_RW, acNameF030_0x + " num", 0, &Obj.Mdp.ConfigIdent[0]},
  //TODO add 254 more elements
};
const _objd SDOF040[] = {
  {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, acNameARR, 255, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, acNameF040_0x + " num", 0x1A00, NULL},
  //TODO add 254 more elements
};
const _objd SDOF050[] =
{
  {0x00, DTYPE_UNSIGNED8,   8,  ATYPE_RO, acNameARR, 0, &Obj.Mdp.nDetectedModules},
  {0x01, DTYPE_UNSIGNED32,  32, ATYPE_RO, acNameF050_0x + " num", 0, &Obj.Mdp.DetectIdent[0]},
  //TODO add 254 more elements
};

const _objd SDOF100[] =
{
  //TODO check what i need here. 4.2.3.11 Device Status (0xF100…0xF10F)
};

const _objd SDOF110[] = {
  //TODO check what i need here. 4.2.3.12 Device Diagnosis (0xF110…0xF11F)
};
const _objd SDOF120[] = {
  //TODO check what i need here. 4.2.3.13 Device Statistics (0xF120…0xF12F)
};
const _objd SDOF200[] =
{
  //TODO check what i need here. 4.2.3.14 Device Control (0xF200…0xF20F)
};
const _objd SDOF600[] = {
  //TODO check what i need here. 4.2.3.15 Device Input Data (0xF600…0xF6FF)
};
const _objd SDOF700[] = {
  //TODO check what i need here. 4.2.3.16 Device Output Data (0xF700…0xF7FF)
};
const _objd SDOF800[] = {
  //TODO check what i need here. 4.2.3.17 Device Configuration Data
};
const _objd SDOFB00[] =
{
  //TODO check what i need here. 4.2.3.18 Device Command (0xFB00…0xFBFF)
};

const _objectlist SDOobjects[] =
{
  {0xF000, OTYPE_RECORD, 4, 0, acNameF000, SDOF000}, //Modular Device Profile
  {0xF002, OTYPE_RECORD,    0, 0, acNameF002, SDOF002}, //Detect Modules Command
  {0xF00E, OTYPE_ARRAY,    0, 0, acNameF00E, SDOF00E}, //Module PDO Group Mapping Alignment PDO Number
  {0xF00F, OTYPE_ARRAY,    0, 0, acNameF00F, SDOF00F}, //Module PDO Group Mapping Alignment
  {0xF010, OTYPE_ARRAY,  255, 0, acNameF010, SDOF010}, //Module Profile List Profile information of the modules
  //TODO add here 15 more elements. Last index 0xF01F (16 objects)
  {0xF020, OTYPE_ARRAY,  255, 0, acNameF020, SDOF020}, //Address list of the configured Modules
  //TODO add here 15 more elements. Last index 0xF02F (16 objects)
  {0xF030, OTYPE_ARRAY,  255, 0, acNameF030, SDOF030}, //Module Ident list of the configured Modules
  //TODO add here 15 more elements.Last index 0xF03F (16 objects)
  {0xF040, OTYPE_ARRAY,  255, 0, acNameF040, SDOF040}, //Address list of detected Modules
  //TODO add here 15 more elements. Last index 0xF04F (16 objects)
  {0xF050, OTYPE_ARRAY,  255, 0, acNameF050, SDOF050}, //Module Ident list of detected Modules
  //TODO add here 15 more elements. Last index 0xF05F (16 objects)
  {0xF100, OTYPE_RECORD, 0, 0, acNameF100, SDOF100}, //Device Status (TxPDO mappable)
  //TODO add here 15 more elements. Last index 0xF10F (16 objects)
  {0xF110, OTYPE_RECORD, 0, 0, acNameF110, SDOF110}, //Device Diagnosis
  //TODO add here 15 more elements. Last index 0xF11F (16 objects)
  {0xF120, OTYPE_RECORD, 0, 0, acNameF120, SDOF120}, //Device Statistics
  //TODO add here 15 more elements.Last index 0xF12F (16 objects)
  {0xF200, OTYPE_RECORD, 0, 0, acNameF200, SDOF200}, //Device Control (RxPDO mappable)
  //TODO add here 15 more elements. Last index 0xF20F (16 objects)
  {0xF600, OTYPE_RECORD, 0, 0, acNameF600, SDOF600}, //Input Area of device itself (TxPDO mappable)
  //TODO add here 255 more elements. Last index 0xF6FF (255 objects)
  {0xF700, OTYPE_RECORD, 0, 0, acNameF700, SDOF700}, //Output Area of device itself (RxPDO mappable)
  //TODO add here 255 more elements. Last index 0xF7FF (255 objects)
  {0xF800, OTYPE_RECORD, 0, 0, acNameF800, SDOF800}, //Device Configuration Parameter Area
  //TODO add here 255 more elements. Last index 0xF8FF (255 objects)
  {0xFB00, OTYPE_RECORD,    0, 0, acNameFB00, SDOFB00}, //Device Command
  //TODO add here 255 more elements. Last index 0xFBFF (255 objects)
  {0xffff, 0xff, 0xff, 0xff, NULL, NULL} // End marker
};

/* ============================================================================
 * Динамическое управление словарем объектов для модульных slave устройств
 * ============================================================================ */

/* Максимальное количество динамических записей в словаре */
#define MAX_DYNAMIC_DICT_ENTRIES (MAX_MDP_SLOTS * 6) /* 6 областей на модуль */

/* Структура для хранения динамических записей словаря */
typedef struct {
    _objectlist entry;
    uint8_t slot;           /* Номер слота модуля */
    uint8_t area_type;       /* Тип области (INPUT, OUTPUT, CONFIG, INFO, DIAGNOSIS, SERVICE) */
    _objd *objd_array;       /* Указатель на динамически выделенный массив _objd */
    uint8_t objd_count;      /* Количество элементов в objd_array */
    char **name_strings;     /* Указатель на массив строк имен (для освобождения памяти) */
    uint8_t name_count;      /* Количество строк имен */
} dynamic_dict_entry_t;

/* Статический массив для хранения динамических записей */
static dynamic_dict_entry_t dynamic_dict_entries[MAX_DYNAMIC_DICT_ENTRIES];
static uint16_t dynamic_dict_count = 0;

/* Вспомогательные строки имен для модулей */
static char module_input_name[] = "Module Input Area";
static char module_output_name[] = "Module Output Area";
static char module_config_name[] = "Module Configuration Area";
static char module_info_name[] = "Module Information Area";
static char module_diagnosis_name[] = "Module Diagnosis Area";
static char module_service_name[] = "Module Service Area";
static char module_subindex_00[] = "Number of Entries";
static char module_subindex_01[] = "Data";

/* Функция для создания имени подобъекта (зарезервировано для будущего использования) */
/* static char* create_subindex_name(uint8_t slot, uint8_t area_type, uint8_t subindex) {
    static char name_buffer[64];
    const char *area_name = "";
    
    switch(area_type) {
        case 0: area_name = "Input"; break;
        case 1: area_name = "Output"; break;
        case 2: area_name = "Config"; break;
        case 3: area_name = "Info"; break;
        case 4: area_name = "Diagnosis"; break;
        case 5: area_name = "Service"; break;
    }
    
    if (subindex == 0) {
        snprintf(name_buffer, sizeof(name_buffer), "Module %d %s - Number of Entries", slot, area_name);
    } else {
        snprintf(name_buffer, sizeof(name_buffer), "Module %d %s - Data", slot, area_name);
    }
    
    return name_buffer;
} */

/**
 * @brief Создает динамический массив _objd для области модуля
 * @param slot Номер слота модуля
 * @param area_type Тип области (0=INPUT, 1=OUTPUT, 2=CONFIG, 3=INFO, 4=DIAGNOSIS, 5=SERVICE)
 * @param data_ptr Указатель на данные модуля (может быть NULL)
 * @param data_size Размер данных в битах
 * @param access_type Тип доступа (ATYPE_RO, ATYPE_RW, etc.)
 * @return Указатель на созданный массив _objd или NULL при ошибке
 */
static _objd* create_module_objd_array(uint8_t slot, uint8_t area_type, 
                                       void *data_ptr, uint16_t data_size,
                                       uint16_t access_type) {
    /* Выделяем память для массива _objd (минимум 2 элемента: subindex 0 и 1) */
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
    objd_array[0].value = (data_ptr != NULL) ? 1 : 0; /* Max subindex */
    objd_array[0].data = NULL;
    
    /* Subindex 1: Data (если данные существуют) */
    if (data_ptr != NULL) {
        objd_array[1].subindex = 0x01;
        
        /* Определяем тип данных на основе размера */
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
            /* Для больших данных используем OCTET_STRING */
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

/**
 * @brief Вычисляет индекс объекта словаря для модуля в указанной области
 * @param slot Номер слота модуля
 * @param area_type Тип области (0=INPUT, 1=OUTPUT, 2=CONFIG, 3=INFO, 4=DIAGNOSIS, 5=SERVICE)
 * @return Индекс объекта словаря или 0xFFFF при ошибке
 */
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

/**
 * @brief Регистрирует динамическую запись словаря для модуля
 * @param slot Номер слота модуля
 * @param area_type Тип области (0=INPUT, 1=OUTPUT, 2=CONFIG, 3=INFO, 4=DIAGNOSIS, 5=SERVICE)
 * @param data_ptr Указатель на данные модуля
 * @param data_size Размер данных в битах
 * @param access_type Тип доступа
 * @return 0 при успехе, -1 при ошибке
 */
int device_obj_areas_add_module_entry(uint8_t slot, uint8_t area_type,
                                      void *data_ptr, uint16_t data_size,
                                      uint16_t access_type) {
    if (slot >= MAX_MDP_SLOTS || area_type > 5) {
        return -1;
    }
    
    if (dynamic_dict_count >= MAX_DYNAMIC_DICT_ENTRIES) {
        return -1; /* Превышен лимит динамических записей */
    }
    
    /* Проверяем, не существует ли уже запись для этого слота и области */
    for (uint16_t i = 0; i < dynamic_dict_count; i++) {
        if (dynamic_dict_entries[i].slot == slot && 
            dynamic_dict_entries[i].area_type == area_type) {
            /* Запись уже существует, обновляем её */
            /* Освобождаем старый массив _objd */
            if (dynamic_dict_entries[i].objd_array) {
                free(dynamic_dict_entries[i].objd_array);
            }
            /* Создаем новый массив */
            dynamic_dict_entries[i].objd_array = create_module_objd_array(
                slot, area_type, data_ptr, data_size, access_type);
            if (!dynamic_dict_entries[i].objd_array) {
                return -1;
            }
            dynamic_dict_entries[i].objd_count = (data_ptr != NULL) ? 2 : 1;
            /* Обновляем entry */
            dynamic_dict_entries[i].entry.objdesc = dynamic_dict_entries[i].objd_array;
            dynamic_dict_entries[i].entry.maxsub = (data_ptr != NULL) ? 1 : 0;
            return 0;
        }
    }
    
    /* Создаем новую запись */
    dynamic_dict_entry_t *entry = &dynamic_dict_entries[dynamic_dict_count];
    
    /* Создаем массив _objd */
    entry->objd_array = create_module_objd_array(slot, area_type, data_ptr, data_size, access_type);
    if (!entry->objd_array) {
        return -1;
    }
    
    entry->slot = slot;
    entry->area_type = area_type;
    entry->objd_count = (data_ptr != NULL) ? 2 : 1;
    entry->name_strings = NULL;
    entry->name_count = 0;
    
    /* Определяем имя области */
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
    
    /* Заполняем структуру _objectlist */
    entry->entry.index = calculate_module_index(slot, area_type);
    entry->entry.objtype = OTYPE_RECORD;
    entry->entry.maxsub = (data_ptr != NULL) ? 1 : 0;
    entry->entry.pad1 = 0;
    entry->entry.name = area_name;
    entry->entry.objdesc = entry->objd_array;
    
    dynamic_dict_count++;
    return 0;
}

/**
 * @brief Удаляет динамическую запись словаря для модуля
 * @param slot Номер слота модуля
 * @param area_type Тип области (0=INPUT, 1=OUTPUT, 2=CONFIG, 3=INFO, 4=DIAGNOSIS, 5=SERVICE)
 * @return 0 при успехе, -1 при ошибке
 */
int device_obj_areas_remove_module_entry(uint8_t slot, uint8_t area_type) {
    for (uint16_t i = 0; i < dynamic_dict_count; i++) {
        if (dynamic_dict_entries[i].slot == slot && 
            dynamic_dict_entries[i].area_type == area_type) {
            /* Освобождаем память */
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
            
            /* Сдвигаем оставшиеся записи */
            for (uint16_t j = i; j < dynamic_dict_count - 1; j++) {
                dynamic_dict_entries[j] = dynamic_dict_entries[j + 1];
            }
            
            dynamic_dict_count--;
            return 0;
        }
    }
    return -1; /* Запись не найдена */
}

/**
 * @brief Удаляет все динамические записи словаря для указанного модуля
 * @param slot Номер слота модуля
 * @return Количество удаленных записей
 */
int device_obj_areas_remove_module_all_entries(uint8_t slot) {
    int removed = 0;
    for (uint8_t area_type = 0; area_type < 6; area_type++) {
        if (device_obj_areas_remove_module_entry(slot, area_type) == 0) {
            removed++;
            area_type--; /* Повторно проверяем этот индекс после сдвига */
        }
    }
    return removed;
}

/**
 * @brief Инициализирует все области модуля в словаре на основе данных MDP
 * @param slot Номер слота модуля
 * @return 0 при успехе, -1 при ошибке
 */
int device_obj_areas_init_module_dict(uint8_t slot) {
    if (slot >= MAX_MDP_SLOTS) {
        return -1;
    }
    
    mdp_module_t *module = &mdp_slots[slot];
    if (module->module_ident == 0) {
        return -1; /* Модуль не инициализирован */
    }
    
    int result = 0;
    
    /* Регистрируем Input Area (0x6nn0) */
    if (module->input_data && module->input_size > 0) {
        result |= device_obj_areas_add_module_entry(
            slot, 0, module->input_data, module->input_size, ATYPE_RO | ATYPE_TXPDO);
    }
    
    /* Регистрируем Output Area (0x7nn0) */
    if (module->output_data && module->output_size > 0) {
        result |= device_obj_areas_add_module_entry(
            slot, 1, module->output_data, module->output_size, ATYPE_RW | ATYPE_RXPDO);
    }
    
    /* Регистрируем Configuration Area (0x8nn0) */
    if (module->config_data && module->config_size > 0) {
        result |= device_obj_areas_add_module_entry(
            slot, 2, module->config_data, module->config_size, ATYPE_RW);
    }
    
    /* Регистрируем Information Area (0x9nn0) */
    result |= device_obj_areas_add_module_entry(
        slot, 3, NULL, 0, ATYPE_RO); /* Information всегда доступна */
    
    /* Регистрируем Diagnosis Area (0xAnnn) */
    if (module->diagnosis_data && module->diagnosis_size > 0) {
        result |= device_obj_areas_add_module_entry(
            slot, 4, module->diagnosis_data, module->diagnosis_size, ATYPE_RO);
    }
    
    /* Регистрируем Service Area (0xBnnn) */
    if (module->service_data && module->service_size > 0) {
        result |= device_obj_areas_add_module_entry(
            slot, 5, module->service_data, module->service_size, ATYPE_RW);
    }
    
    return (result == 0) ? 0 : -1;
}

/**
 * @brief Получает динамическую запись словаря по индексу
 * @param index Индекс объекта словаря
 * @return Указатель на запись или NULL если не найдена
 */
const _objectlist* device_obj_areas_get_dynamic_entry(uint16_t index) {
    for (uint16_t i = 0; i < dynamic_dict_count; i++) {
        if (dynamic_dict_entries[i].entry.index == index) {
            return &dynamic_dict_entries[i].entry;
        }
    }
    return NULL;
}

/**
 * @brief Очищает все динамические записи словаря
 */
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

/**
 * @brief Получает количество динамических записей в словаре
 * @return Количество записей
 */
uint16_t device_obj_areas_get_dynamic_count(void) {
    return dynamic_dict_count;
}

/**
 * @brief Расширенный поиск объекта в словаре (статический + динамический)
 * 
 * Ищет объект сначала в статическом словаре SDOobjects, затем в динамическом.
 * Может быть использована как расширение для SDO_findobject.
 * 
 * @param index Индекс объекта словаря
 * @param static_idx Указатель для возврата индекса в статическом массиве (или -1)
 * @return Указатель на _objectlist запись или NULL если не найдена
 */
const _objectlist* device_obj_areas_find_object_extended(uint16_t index, int32_t *static_idx) {
    /* Сначала ищем в статическом словаре */
    int32_t n = 0;
    while (SDOobjects[n].index < index && SDOobjects[n].index != 0xffff) {
        n++;
    }
    if (SDOobjects[n].index == index) {
        if (static_idx) *static_idx = n;
        return &SDOobjects[n];
    }
    
    /* Если не найдено в статическом, ищем в динамическом */
    if (static_idx) *static_idx = -1;
    return device_obj_areas_get_dynamic_entry(index);
}
