#include <stdio.h>
#include <string.h>
#include "ecat_slv.h"
#include "utypes.h"
#include "esc_mdp.h"
#include "device_obj_areas.h"
#include <bcm2835.h>

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

#ifdef USE_MDP
/* MDP Module callbacks and initialization */

/* Module 0: Digital I/O Module (8 inputs, 8 outputs) */
void cb_module0_get_inputs(void)
{
    /* Read GPIO inputs and update module data */
    Obj.Mdp.module0_input[0] = bcm2835_gpio_lev(GPIO26);
    Obj.Mdp.module0_input[1] = bcm2835_gpio_lev(GPIO19);
    Obj.Mdp.module0_input[2] = bcm2835_gpio_lev(GPIO13);
    Obj.Mdp.module0_input[3] = bcm2835_gpio_lev(GPIO06);
    Obj.Mdp.module0_input[4] = bcm2835_gpio_lev(GPIO05);
    Obj.Mdp.module0_input[5] = bcm2835_gpio_lev(GPIO22);
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
#endif /* USE_MDP */

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
#ifdef USE_MDP
    /* Update module inputs */
    cb_module0_get_inputs();
    cb_module1_get_inputs();
    
    /* Update module outputs */
    cb_module0_set_outputs();
    cb_module2_set_outputs();
#endif
}

int main_run(void *arg)
{
    static esc_cfg_t config =
    {
        .user_arg = "rpi5,cs0",
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
    
#ifdef USE_MDP
    MDP_init_modules();
    printf("MDP modules initialized\n");
#endif
    
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
