#include <stdio.h>
#include <string.h>
#include "ecat_slv.h"
#include "utypes.h"
#include "esc_mdp.h"
#include "device_obj_areas.h"

/* bcm2835 GPIO access:
 * - If HAVE_BCM2835 is defined, use the real bcm2835 headers.
 * - Otherwise (macOS/host build or when the library is not installed),
 *   provide minimal stubs so the demo still compiles.
 */
#if !defined(HAVE_BCM2835)
#define LOW 0
#define HIGH 1
#define BCM2835_GPIO_FSEL_OUTP 1
#define BCM2835_GPIO_FSEL_INPT 0
#define BCM2835_GPIO_PUD_DOWN 0

/* Dummy pin numbers for host build */
#define RPI_BPLUS_GPIO_J8_40 40
#define RPI_BPLUS_GPIO_J8_38 38
#define RPI_BPLUS_GPIO_J8_36 36
#define RPI_BPLUS_GPIO_J8_32 32
#define RPI_BPLUS_GPIO_J8_18 18
#define RPI_BPLUS_GPIO_J8_16 16
#define RPI_BPLUS_GPIO_J8_37 37
#define RPI_BPLUS_GPIO_J8_35 35
#define RPI_BPLUS_GPIO_J8_33 33
#define RPI_BPLUS_GPIO_J8_31 31
#define RPI_BPLUS_GPIO_J8_29 29
#define RPI_BPLUS_GPIO_J8_15 15

static inline int bcm2835_init(void) { return 1; }
static inline void bcm2835_close(void) {}
static inline void bcm2835_gpio_fsel(int pin, int mode) { (void)pin; (void)mode; }
static inline void bcm2835_gpio_set_pud(int pin, int pud) { (void)pin; (void)pud; }
static inline void bcm2835_gpio_write(int pin, int val) { (void)pin; (void)val; }
static inline int bcm2835_gpio_lev(int pin) { (void)pin; return 0; }

/* Stubs for HAL symbols when building host-side (no real ESC). */
void ESC_read(uint16_t address, void *buf, uint16_t len)
{
    (void)address;
    memset(buf, 0, len);
}

void ESC_write(uint16_t address, void *buf, uint16_t len)
{
    (void)address; (void)buf; (void)len;
}

void ESC_init(const esc_cfg_t * cfg)
{
    (void)cfg;
}

void ESC_reset(void) {}

/* Stubs for generic digital demo callbacks (not used in this MDP demo). */
void cb_get_inputs(void) {}
void cb_set_outputs(void) {}

#else
#include <bcm2835.h>
#endif

/* Application variables */
_Objects    Obj;

/* GPIO definitions for Raspberry Pi 5 */
#define GPIO21 RPI_BPLUS_GPIO_J8_40
#define GPIO20 RPI_BPLUS_GPIO_J8_38
#define GPIO16 RPI_BPLUS_GPIO_J8_36
#define GPIO12 RPI_BPLUS_GPIO_J8_32
#define GPIO24 RPI_BPLUS_GPIO_J8_18
#define GPIO23 RPI_BPLUS_GPIO_J8_16
#define GPIO26 RPI_BPLUS_GPIO_J8_37
#define GPIO19 RPI_BPLUS_GPIO_J8_35
#define GPIO13 RPI_BPLUS_GPIO_J8_33
#define GPIO06 RPI_BPLUS_GPIO_J8_31
#define GPIO05 RPI_BPLUS_GPIO_J8_29
#define GPIO22 RPI_BPLUS_GPIO_J8_15

/* MDP Module callbacks and initialization */

/* Module 0: Digital I/O Module (8 inputs, 8 outputs) */
void cb_module0_get_inputs(void)
{
    /* Read GPIO inputs and update module data */
    Obj.Mdp.module0_input[0] = (uint8_t)bcm2835_gpio_lev(GPIO26);
    Obj.Mdp.module0_input[1] = (uint8_t)bcm2835_gpio_lev(GPIO19);
    Obj.Mdp.module0_input[2] = (uint8_t)bcm2835_gpio_lev(GPIO13);
    Obj.Mdp.module0_input[3] = (uint8_t)bcm2835_gpio_lev(GPIO06);
    Obj.Mdp.module0_input[4] = (uint8_t)bcm2835_gpio_lev(GPIO05);
    Obj.Mdp.module0_input[5] = (uint8_t)bcm2835_gpio_lev(GPIO22);
    Obj.Mdp.module0_input[6] = 0; /* Reserved */
    Obj.Mdp.module0_input[7] = 0; /* Reserved */
}

void cb_module0_set_outputs(void)
{
    /* Write GPIO outputs from module data */
    bcm2835_gpio_write(GPIO21, (Obj.Mdp.module0_output[0] ? LOW : HIGH));
    bcm2835_gpio_write(GPIO20, (Obj.Mdp.module0_output[1] ? LOW : HIGH));
    bcm2835_gpio_write(GPIO16, (Obj.Mdp.module0_output[2] ? LOW : HIGH));
    bcm2835_gpio_write(GPIO12, (Obj.Mdp.module0_output[3] ? LOW : HIGH));
    bcm2835_gpio_write(GPIO24, (Obj.Mdp.module0_output[4] ? LOW : HIGH));
    bcm2835_gpio_write(GPIO23, (Obj.Mdp.module0_output[5] ? LOW : HIGH));
    /* GPIO for outputs 6-7 not connected */
}

/* Module 1: Analog Input Module (4 channels) - simulated */
void cb_module1_get_inputs(void)
{
    /* Simulated analog inputs - in real application, read from ADC */
    Obj.Mdp.module1_input[0] = 0x1234;
    Obj.Mdp.module1_input[1] = 0x2345;
    Obj.Mdp.module1_input[2] = 0x3456;
    Obj.Mdp.module1_input[3] = 0x4567;
}

/* Module 2: Analog Output Module (4 channels) - simulated */
void cb_module2_set_outputs(void)
{
    /* Simulated analog outputs - in real application, write to DAC */
    /* Obj.Mdp.module2_output[0-3] contains output values */
}

void MDP_init_modules(void)
{
    /* Initialize MDP subsystem */
    MDP_init();
    
    /* Module 0: Digital I/O Module
     * Module ID: 0x00010001 (example: Vendor 0x0001, Product 0x0001)
     * 8 inputs (8 bits), 8 outputs (8 bits)
     */
    MDP_add_module_full(
        0,                          /* slot */
        0x00010001,                 /* module_id: Digital I/O Module */
        Obj.Mdp.module0_input,      /* input_data */
        8,                          /* input_size (bits) */
        Obj.Mdp.module0_output,    /* output_data */
        8,                          /* output_size (bits) */
        NULL, 0,                   /* config_data, config_size */
        NULL, 0,                   /* diagnosis_data, diagnosis_size */
        NULL, 0,                   /* service_transfer, service_size */
        0x00001389,                 /* device_type: MDP device */
        0x00000001,                 /* vendor_id */
        0x00010001,                 /* product_code */
        0x00000001,                 /* revision */
        0x00000001,                 /* serial_number */
        NULL, NULL,                /* config callbacks */
        NULL, NULL, NULL            /* diagnosis/service callbacks */
    );
    
    /* Initialize dictionary for module 0 */
    device_obj_areas_init_module_dict(0);
    
    /* Module 1: Analog Input Module
     * Module ID: 0x00010002 (example: Vendor 0x0001, Product 0x0002)
     * 4 analog inputs (16 bits each = 64 bits total)
     */
    MDP_add_module_full(
        1,                          /* slot */
        0x00010002,                 /* module_id: Analog Input Module */
        Obj.Mdp.module1_input,      /* input_data */
        64,                         /* input_size (bits): 4 channels * 16 bits */
        NULL, 0,                   /* output_data, output_size */
        NULL, 0,                   /* config_data, config_size */
        NULL, 0,                   /* diagnosis_data, diagnosis_size */
        NULL, 0,                   /* service_transfer, service_size */
        0x00001389,                 /* device_type: MDP device */
        0x00000001,                 /* vendor_id */
        0x00010002,                 /* product_code */
        0x00000001,                 /* revision */
        0x00000002,                 /* serial_number */
        NULL, NULL,                /* config callbacks */
        NULL, NULL, NULL            /* diagnosis/service callbacks */
    );
    
    /* Initialize dictionary for module 1 */
    device_obj_areas_init_module_dict(1);
    
    /* Module 2: Analog Output Module
     * Module ID: 0x00010003 (example: Vendor 0x0001, Product 0x0003)
     * 4 analog outputs (16 bits each = 64 bits total)
     */
    MDP_add_module_full(
        2,                          /* slot */
        0x00010003,                 /* module_id: Analog Output Module */
        NULL, 0,                   /* input_data, input_size */
        Obj.Mdp.module2_output,     /* output_data */
        64,                         /* output_size (bits): 4 channels * 16 bits */
        NULL, 0,                   /* config_data, config_size */
        NULL, 0,                   /* diagnosis_data, diagnosis_size */
        NULL, 0,                   /* service_transfer, service_size */
        0x00001389,                 /* device_type: MDP device */
        0x00000001,                 /* vendor_id */
        0x00010003,                 /* product_code */
        0x00000001,                 /* revision */
        0x00000003,                 /* serial_number */
        NULL, NULL,                /* config callbacks */
        NULL, NULL, NULL            /* diagnosis/service callbacks */
    );
    
    /* Initialize dictionary for module 2 */
    device_obj_areas_init_module_dict(2);
    
    printf("MDP: Initialized 3 modules\n");
    printf("  Module 0: Digital I/O (slot 0, ID: 0x%08X)\n", 0x00010001);
    printf("  Module 1: Analog Input (slot 1, ID: 0x%08X)\n", 0x00010002);
    printf("  Module 2: Analog Output (slot 2, ID: 0x%08X)\n", 0x00010003);
}

void GPIO_init(void)
{
    bcm2835_init();
    
    /* Configure GPIO outputs for Module 0 */
    bcm2835_gpio_fsel(GPIO21, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(GPIO20, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(GPIO16, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(GPIO12, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(GPIO24, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(GPIO23, BCM2835_GPIO_FSEL_OUTP);
    
    /* Configure GPIO inputs for Module 0 */
    bcm2835_gpio_fsel(GPIO26, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(GPIO19, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(GPIO13, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(GPIO06, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(GPIO05, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(GPIO22, BCM2835_GPIO_FSEL_INPT);
    
    /* Set pull-down resistors for inputs */
    bcm2835_gpio_set_pud(GPIO26, BCM2835_GPIO_PUD_DOWN);
    bcm2835_gpio_set_pud(GPIO19, BCM2835_GPIO_PUD_DOWN);
    bcm2835_gpio_set_pud(GPIO13, BCM2835_GPIO_PUD_DOWN);
    bcm2835_gpio_set_pud(GPIO06, BCM2835_GPIO_PUD_DOWN);
    bcm2835_gpio_set_pud(GPIO05, BCM2835_GPIO_PUD_DOWN);
    bcm2835_gpio_set_pud(GPIO22, BCM2835_GPIO_PUD_DOWN);
}

void application_hook(void)
{
    /* Update module inputs */
    cb_module0_get_inputs();
    cb_module1_get_inputs();
    
    /* Update module outputs */
    cb_module0_set_outputs();
    cb_module2_set_outputs();
}

int main_run(void *arg)
{
    static esc_cfg_t config =
    {
        /* Use Linux spidev on RP1: SPI10 CS0 is /dev/spidev10.0 */
        .user_arg = "/dev/spidev10.0",
        .use_interrupt = 0,
        .watchdog_cnt = 150,
        .set_defaults_hook = NULL,
        .pre_state_change_hook = NULL,
        .post_state_change_hook = NULL,
        .application_hook = application_hook,
        .safeoutput_override = NULL,
        .pre_object_download_hook = NULL,
        .post_object_download_hook = NULL,
        .rxpdo_override = NULL,
        .txpdo_override = NULL,
        .esc_hw_interrupt_enable = NULL,
        .esc_hw_interrupt_disable = NULL,
        .esc_hw_eep_handler = NULL,
        .esc_check_dc_handler = NULL,
    };

    printf("Raspberry Pi 5 MDP EtherCAT Slave Demo\n");
    printf("========================================\n");
    
    GPIO_init();

    MDP_init_modules();
    printf("MDP modules initialized\n");
    
    ecat_slv_init(&config);
    
    printf("EtherCAT slave initialized, entering main loop...\n");
    
    while (1)
    {
        ecat_slv();
    }

    return 0;
}

int main(void)
{
    main_run(NULL);
    return 0;
}
