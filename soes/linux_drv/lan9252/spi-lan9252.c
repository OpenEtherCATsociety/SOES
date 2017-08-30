//#ifndef DEBUG
//#define DEBUG
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/spi/spi.h>
#include <linux/uaccess.h>


/*
        Datasheet : http://ww1.microchip.com/downloads/en/DeviceDoc/
                00001909A.pdf
*/

#define LAN9252_MAGIC 'l' //8bit=0~0xff 'l'an9252
#define TEST _IO(LAN9252_MAGIC, 0)

#define LAN9252_LOCK mutex_lock(&lan9252_mutex);
#define LAN9252_UNLOCK mutex_unlock(&lan9252_mutex);

#define DEVICE_NAME	"lan9252"
#define LAN9252_MAJOR	153

#define FAST_READ	0x0B
#define FAST_READ_DUMMY	1

struct mutex lock;
static DEFINE_MUTEX(lan9252_mutex);

dev_t id;
static struct device *dev;
static struct class *lan9252_class;
static struct spi_device *lan9252_devices;

static int lan9252_ethercat_probe(struct spi_device *spi);
static int lan9252_ethercat_remove(struct spi_device *spi);


static int lan9252_open(struct inode *inode, struct file *filp)
{
	dev_dbg(dev, "%s() called\n", __FUNCTION__);
	return 0;
}

static int lan9252_release(struct inode *inode, struct file *filp)
{
	dev_dbg(dev, "%s() called\n", __FUNCTION__);
	return 0;
}

static loff_t lan9252_llseek(struct file *file, loff_t address,
                         int whence)
{
	int status;

	LAN9252_LOCK;

        file->f_pos = address;

	status = file->f_pos;

	LAN9252_UNLOCK;
	
	return status;
}

static int * lan9252_reg_addr_make(int address)
{

	int temp;
	static int lan9252_reg_addr[2];
#ifdef DEBUG
	int i;
#endif

	dev_dbg(dev, "%s() called\n", __FUNCTION__);

	temp = address & 0xff00;
	lan9252_reg_addr[0] = temp >> 8;
	temp = address & 0x00ff;
	lan9252_reg_addr[1] = temp;

#ifdef DEBUG
	for (i=0; i<2; i++)
	modp
		dev_dbg(dev, "%s() lan9252_reg_addr[%d] = 0x%x\n",
		        __FUNCTION__, i, lan9252_reg_addr[i]);
#endif
   
	return lan9252_reg_addr;
}

/*
        int lan9252_test(void);
        Description :   Check the connection status of LAN9252    
        Retrun Value
        0       :   success
        other   :   fail   

        Register Type           :   SYSTEM CONTROL AND STATUS REGISTERS
        Register Name(SYMBOL)   :   BYTE ORDER TEST REGISTER(BYTE_TEST)
        Offset                  :   0x0064
        Size                    :   32bit
        Default                 :   0x87654321
        Datasheet description   :   Chip-level reset/configuration
                                completion can be determined by first
                                polling the 
                                Byte Order Test Register(BYTE_TEST).
*/
static int lan9252_test(void)
{
        u8 command[4];
        u8 test_buffer[4];
        int status=0;
#ifdef DEBUG
	int i;
#endif
        command[0]=FAST_READ;
        command[1]=0x00;
        command[2]=0x64;
        command[3]=FAST_READ_DUMMY;


        status = spi_write_then_read(lan9252_devices, &command,
                sizeof(command), test_buffer, sizeof(test_buffer));
        if (status < 0 ) {
                dev_err(dev, "%s() spi_write_then_read failed "
                        "status=%d\n", __FUNCTION__, status);
                return -1;
        }

        status = spi_write_then_read(lan9252_devices, &command,
                sizeof(command), test_buffer, sizeof(test_buffer));
        if (status < 0 ) {
                dev_err(dev, "%s() spi_write_then_read failed "
                        "status=%d\n", __FUNCTION__, status);
                return -1;
        }
    
#ifdef DEBUG
    for (i=0; i<sizeof(test_buffer); i++)
        dev_dbg(dev, "%s() test_buffer[%d] = 0x%x\n", __FUNCTION__, i,
	        test_buffer[i]);
#endif

    if (test_buffer[0]==0x21 && test_buffer[1]==0x43 &&
        test_buffer[2]==0x65 && test_buffer[3]==0x87)
        return 0;
    else 
        return -1;                     
}

static ssize_t lan9252_read(
        struct file *file, char *buffer,
	size_t read_buffer_size, loff_t *f_pos)
{
        u8 command[4];
        u8 *read_buffer;
        int status=0;
        int *lan9252_reg_read_addr;
#ifdef DEBUG
        int i;
#endif

	dev_dbg(dev, "%s() called\n", __FUNCTION__);

	LAN9252_LOCK;

	if (read_buffer_size <= 0) {
		dev_err(dev, "read_buffer_size(%d) <= 0 \n",
			read_buffer_size);
		goto out;
	} else {
		dev_dbg(dev, "read_buffer_size = %d\n",
		        read_buffer_size);
	}

	read_buffer= (u8 *)kmalloc((read_buffer_size) *
			sizeof(u32), GFP_KERNEL | GFP_DMA);
	if (!read_buffer) {
		dev_err(dev, "write read_buffer kmalloc error\n");
		status = -ENOMEM;
		goto out;
	 }

        memset(read_buffer, 0, (read_buffer_size) * sizeof(u8));

	lan9252_reg_read_addr = lan9252_reg_addr_make(file->f_pos);
	command[0] = FAST_READ;
	command[1] = lan9252_reg_read_addr[0];
	command[2] = lan9252_reg_read_addr[1];
	command[3] = FAST_READ_DUMMY;

#ifdef DEBUG
	for (i=0; i<4; i++)
		dev_dbg(dev, "%s() command[%d] = 0x%x\n", __FUNCTION__,
			i, command[i]);
#endif

	status = spi_write_then_read(lan9252_devices, &command,
                        sizeof(command), read_buffer,read_buffer_size);
	if (status < 0 ) {
		dev_err(dev, "%s() spi_write_then_read failed "
		        "status=%d\n", __FUNCTION__, status);
		goto kfree_read_buffer;
	}

#ifdef DEBUG
	for (i=0; i<read_buffer_size; i++)
		dev_dbg(dev, "%s() read_buffer[%d] = 0x%x\n",
		        __FUNCTION__, i, read_buffer[i]);
#endif

	if (copy_to_user(buffer, read_buffer, read_buffer_size)) {
		dev_err(dev, "%s() copy_to_user failed\n",
		        __FUNCTION__);
		status = -EFAULT;
		goto kfree_read_buffer;
	}

	status = read_buffer_size;

kfree_read_buffer:
	kfree(read_buffer);

out:
	LAN9252_UNLOCK;
	return status;
}


static ssize_t lan9252_write(
        struct file *file, const char *command_buff,
        size_t command_buff_size, loff_t *f_pos)
{
	u8 *command;
	int status=0;
#ifdef DEBUG
	int i;
#endif

	dev_dbg(dev, "%s() called\n", __FUNCTION__);

	LAN9252_LOCK;

	if (command_buff_size <= 0) {
		dev_err(dev, "command_buff_size(%d) <= 0 \n",
			command_buff_size);
		goto out;
	} else {
		dev_dbg(dev, "command_buff_size = %d\n",
		        command_buff_size);
	}

	command = (u8 *)kmalloc((command_buff_size) *
			sizeof(u8), GFP_KERNEL | GFP_DMA);
	if (!command) {
		dev_err(dev, "write command kmalloc error\n");
		status = -ENOMEM;
		goto out;
	}

	memset(command, 0, (command_buff_size) * sizeof(u8));

	if (copy_from_user(command, command_buff, command_buff_size)) {
		dev_err(dev, "%s() copy_from_user failed\n",
	                __FUNCTION__);
		status = -EFAULT;
		goto kfree_command;
	}

#ifdef DEBUG
	for(i=0;i<command_buff_size;i++)
		dev_dbg(dev, "command[%d]=0x%x\n",i,command[i]);
#endif

	status = spi_write_then_read(lan9252_devices, command,
	                command_buff_size, NULL, 0);
	if (status < 0 ) {
		dev_err(dev, "%s() spi_write_then_read failed "
		 "status=%d\n", __FUNCTION__, status);
		goto kfree_command;
	}

kfree_command:
	kfree(command);

out:
	LAN9252_UNLOCK;
	return status;
}

static long lan9252_ioctl(struct file *file, unsigned int cmd,
                unsigned long argument)
{

        int status;

        LAN9252_LOCK;
	
        switch (cmd) {
        case TEST:
                status=lan9252_test();   
                break;

        default:   
                status=0;
                break;
        }

        LAN9252_UNLOCK;

        return status;	
}


static const struct spi_device_id lan9252_id_table[] = {
	{ "lan9252", 0 },
	{ }
};
MODULE_DEVICE_TABLE(spi, lan9252_id_table);

static const struct file_operations lan9252_fops = {
        .owner =	THIS_MODULE,
        .open =		lan9252_open,
        .release =	lan9252_release,
        .llseek	=	lan9252_llseek,
        .write =	lan9252_write,
        .read =		lan9252_read,
        .unlocked_ioctl = lan9252_ioctl,
};

static struct spi_driver lan9252_ethercat_driver = {
	.driver = {
		.name = DEVICE_NAME,
	},
	.id_table = lan9252_id_table,
	.probe = lan9252_ethercat_probe,
	.remove = lan9252_ethercat_remove,
};

static int lan9252_ethercat_probe(struct spi_device *spi)
{
	int status;

	dev_dbg(&spi->dev, "%s() called\n", __FUNCTION__);

	mutex_init(&lock);

	spi->mode=0x03;

	lan9252_devices = spi;

        if(lan9252_test())
                dev_warn(&spi->dev, "lan9252 not connected\n");

	if (register_chrdev(LAN9252_MAJOR,
	        lan9252_ethercat_driver.driver.name, &lan9252_fops)) {
		dev_err(&spi->dev, "unable to get major %d for "            
		, LAN9252_MAJOR);
		status = -ENODEV;
		goto register_chrdev_fail;
	}

	lan9252_class = class_create(THIS_MODULE,
        		lan9252_ethercat_driver.driver.name);
	if (IS_ERR(lan9252_class)) {
		status = PTR_ERR(lan9252_class);
		goto class_create_fail;
	}
	
	id = MKDEV(LAN9252_MAJOR, 0);
	dev = device_create(lan9252_class, NULL, id, NULL,
		 lan9252_ethercat_driver.driver.name);  
	if (IS_ERR(dev)) {
		status = PTR_ERR(dev);  
		dev_err(dev, "device_create error %d\n", status);
		goto device_create_fail;

	}  

	dev_dbg(dev, "spi->max_speed_hz=%d spi->bits_per_word=%d "
                "spi->mode=%d\n", spi->max_speed_hz,
                        spi->bits_per_word, spi->mode);
	dev_dbg(dev, "spi->chip_select=%d spi->cs_gpio=%d \n",
                spi->chip_select, spi->cs_gpio);

	dev_info(dev, "Probed\n");

	return 0;


device_create_fail:
	class_destroy(lan9252_class);

class_create_fail:
	unregister_chrdev(LAN9252_MAJOR, 
	        lan9252_ethercat_driver.driver.name);

register_chrdev_fail:
	dev_err(&spi->dev, "Probed failed : %d\n", status);  
	return status;
}

static int lan9252_ethercat_remove(struct spi_device *spi)
{
	dev_dbg(dev, "%s() called\n", __FUNCTION__);

	device_destroy(lan9252_class, id); 
	class_destroy(lan9252_class);
	unregister_chrdev(LAN9252_MAJOR,
	        lan9252_ethercat_driver.driver.name);

	dev_info(dev, "Removed\n");
	return 0;
}
module_spi_driver(lan9252_ethercat_driver);

MODULE_AUTHOR("tykwon@m2i.co.kr");
MODULE_DESCRIPTION("Microchip LAN9252 Ethercat Driver");
MODULE_LICENSE("GPL");
//#endif
