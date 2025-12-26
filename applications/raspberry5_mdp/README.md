# Raspberry Pi 5 MDP EtherCAT Slave Demo

## Описание

Этот пример демонстрирует использование Modular Device Profile (MDP) на Raspberry Pi 5 с LAN9252 EtherCAT контроллером.

## Особенности

- **MDP Support**: Полная поддержка ETG.5001.1 Modular Device Profile
- **3 модуля**:
  - **Module 0**: Digital I/O Module (8 inputs, 8 outputs)
  - **Module 1**: Analog Input Module (4 channels, 16-bit)
  - **Module 2**: Analog Output Module (4 channels, 16-bit)

## Требования

- Raspberry Pi 5
- LAN9252 EtherCAT контроллер (через SPI)
- Библиотека bcm2835 для работы с GPIO
- CMake 2.8.12 или выше

## Компиляция

```bash
# Установите переменную окружения для выбора демо
export RPI_VARIANT=1

# Создайте директорию сборки
mkdir build
cd build

# Настройте CMake
cmake ..

# Соберите проект
make

# Установите (опционально)
sudo make install
```

## Конфигурация

### SPI интерфейс

По умолчанию используется SPI интерфейс с CS0:
- SPI0_CS0 (GPIO 8)

Для изменения используйте параметр `user_arg` в `main.c`:
```c
.user_arg = "rpi5,cs0",  // или "rpi5,cs1" для CS1
```

### GPIO подключения

**Module 0 - Digital I/O:**

Inputs (GPIO):
- GPIO26 (Pin 37)
- GPIO19 (Pin 35)
- GPIO13 (Pin 33)
- GPIO06 (Pin 31)
- GPIO05 (Pin 29)
- GPIO22 (Pin 15)

Outputs (GPIO):
- GPIO21 (Pin 40)
- GPIO20 (Pin 38)
- GPIO16 (Pin 36)
- GPIO12 (Pin 32)
- GPIO24 (Pin 18)
- GPIO23 (Pin 16)

## Использование

После компиляции и запуска, устройство будет доступно как EtherCAT slave с MDP функционалом.

### Object Dictionary

- **Object 0x1000**: Device Type = `0x01901389` (MDP device, subtype 0x01)
- **Object 0xF000**: Modular Device Profile (Device Area)
- **Object 0xF030**: Configured Module Ident List
- **Object 0x6000-0x6FFF**: Module Input Areas
- **Object 0x7000-0x7FFF**: Module Output Areas
- **Object 0x9000-0x9FFF**: Module Information Areas

### Модули

**Module 0 (Slot 0)**: Digital I/O
- Module ID: `0x00010001`
- Input Area: `0x6000` (8 bits)
- Output Area: `0x7000` (8 bits)
- Information Area: `0x9000`

**Module 1 (Slot 1)**: Analog Input
- Module ID: `0x00010002`
- Input Area: `0x6010` (64 bits: 4 channels × 16 bits)
- Information Area: `0x9010`

**Module 2 (Slot 2)**: Analog Output
- Module ID: `0x00010003`
- Output Area: `0x7020` (64 bits: 4 channels × 16 bits)
- Information Area: `0x9020`

## Структура проекта

```
raspberry5_mdp/
├── CMakeLists.txt      # CMake конфигурация
├── main.c              # Основной файл с MDP инициализацией
├── slave_objectlist.c  # Object Dictionary определение
├── utypes.h            # Типы данных приложения
├── ecat_options.h      # Опции EtherCAT (USE_MDP=1)
└── README.md           # Этот файл
```

## Примечания

- Analog Input/Output модули используют симулированные данные (в реальном приложении подключите ADC/DAC)
- Все модули инициализируются при старте приложения
- Information Area доступна для всех модулей (device subtype = Modular Device)

## Лицензия

См. LICENSE файл в корне проекта.
