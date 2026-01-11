/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

/** \file
 * \brief
 * Header file for dynamic object dictionary management for modular slave devices
 *
 * This module provides functions for dynamically creating and managing
 * object dictionary entries for Modular Device Profile (MDP) slave devices.
 * Implementation according to ETG.5001.1 S (D) V0.9.0
 */

#ifndef SOES_DEVICE_OBJ_AREAS_H
#define SOES_DEVICE_OBJ_AREAS_H

#include <stdint.h>
#include "cc.h"
#include "esc_coe.h"

/**
 * @brief Регистрирует динамическую запись словаря для модуля
 * 
 * Создает запись в словаре объектов для указанной области модуля.
 * Области модуля согласно ETG.5001.1:
 * - 0: Input Area (0x6nn0) - TxPDO mappable
 * - 1: Output Area (0x7nn0) - RxPDO mappable
 * - 2: Configuration Area (0x8nn0)
 * - 3: Information Area (0x9nn0)
 * - 4: Diagnosis Area (0xAnnn)
 * - 5: Service Area (0xBnnn)
 * 
 * @param slot Номер слота модуля (0..MAX_MDP_SLOTS-1)
 * @param area_type Тип области (0-5)
 * @param data_ptr Указатель на данные модуля (может быть NULL для Information Area)
 * @param data_size Размер данных в битах
 * @param access_type Тип доступа (ATYPE_RO, ATYPE_RW, ATYPE_TXPDO, ATYPE_RXPDO, etc.)
 * @return 0 при успехе, -1 при ошибке
 */
int device_obj_areas_add_module_entry(uint8_t slot, uint8_t area_type,
                                      void *data_ptr, uint16_t data_size,
                                      uint16_t access_type);

/**
 * @brief Удаляет динамическую запись словаря для модуля
 * 
 * @param slot Номер слота модуля
 * @param area_type Тип области (0-5)
 * @return 0 при успехе, -1 при ошибке (запись не найдена)
 */
int device_obj_areas_remove_module_entry(uint8_t slot, uint8_t area_type);

/**
 * @brief Удаляет все динамические записи словаря для указанного модуля
 * 
 * Удаляет все 6 областей модуля (Input, Output, Config, Info, Diagnosis, Service).
 * 
 * @param slot Номер слота модуля
 * @return Количество удаленных записей
 */
int device_obj_areas_remove_module_all_entries(uint8_t slot);

/**
 * @brief Инициализирует все области модуля в словаре на основе данных MDP
 * 
 * Автоматически регистрирует все доступные области модуля на основе
 * данных из структуры mdp_slots[slot]. Используется для быстрой инициализации
 * словаря после добавления модуля через MDP_add_module* функции.
 * 
 * @param slot Номер слота модуля
 * @return 0 при успехе, -1 при ошибке
 */
int device_obj_areas_init_module_dict(uint8_t slot);

/**
 * @brief Получает динамическую запись словаря по индексу
 * 
 * Используется для поиска динамических записей в словаре объектов.
 * Может быть интегрирована с SDO_findobject для поддержки динамических объектов.
 * 
 * @param index Индекс объекта словаря (например, 0x6000, 0x7000, etc.)
 * @return Указатель на запись или NULL если не найдена
 */
const _objectlist* device_obj_areas_get_dynamic_entry(uint16_t index);

/**
 * @brief Очищает все динамические записи словаря
 * 
 * Освобождает всю выделенную память и сбрасывает счетчик записей.
 * Используется при деинициализации или полной перестройке словаря.
 */
void device_obj_areas_clear_all(void);

/**
 * @brief Получает количество динамических записей в словаре
 * 
 * @return Количество активных динамических записей
 */
uint16_t device_obj_areas_get_dynamic_count(void);

/**
 * @brief Расширенный поиск объекта в словаре (статический + динамический)
 * 
 * Ищет объект сначала в статическом словаре SDOobjects, затем в динамическом.
 * Может быть использована как расширение для SDO_findobject для поддержки
 * динамических объектов модулей.
 * 
 * @param index Индекс объекта словаря
 * @param static_idx Указатель для возврата индекса в статическом массиве (или -1)
 * @return Указатель на _objectlist запись или NULL если не найдена
 */
const _objectlist* device_obj_areas_find_object_extended(uint16_t index, int32_t *static_idx);

#endif /* SOES_DEVICE_OBJ_AREAS_H */

