/*Modyfied by dany rev 001
 * rfbb.c
 *
 * rfbb -   Device driver
 *
 */


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 32)
#error "**********************************************************"
#error " Sorry, this driver needs kernel version 2.6.32 or higher "
#error "**********************************************************"
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#endif

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/serial_reg.h>
#include <linux/time.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/platform_device.h>
#include <asm/system.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/fcntl.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <linux/cdev.h> 
#include <linux/kfifo.h>
#include "query_ioctl.h"



MODULE_DESCRIPTION("Custom Tiny Device Driver");
MODULE_AUTHOR("Dany Jacoby");
MODULE_LICENSE("GPL");




#define RFBB_DRIVER_VERSION "0.05"
#define RFBB_DRIVER_NAME "rfbb"

#define MAX_RFBB_DEVS 1 /* One device only */






///typedef int32_t lirc_t;

/*
 * We export one rfbb device.
 */
static struct cdev rfbbDevs[MAX_RFBB_DEVS];
 


static int rfbb_major = 0; /* use dynamic major number assignment */


static int debug = 1;
static int is_device_open = 0;
static unsigned int pulse_dly_ms=500;


static DEFINE_MUTEX(read_lock);

#define dprintk(fmt, args...)					\
	do {							\
		if (debug)					\
			printk(KERN_DEBUG RFBB_DRIVER_NAME ": "	\
			       fmt, ## args);			\
	} while (0)

/* forward declarations */

static void on(void); /* TX signal on */
static void off(void); /* TX signal off */
static void send_pulse(unsigned long length);
static void send_space(unsigned long length);
static void device_exit_module(void);
static int device_init(void);
static int init_port(void);
static void rfbb_setup_cdev(struct cdev *dev, int minor,struct file_operations *fops);
static int hardware_init(void);



static int device_open(struct inode *, struct file *);
static ssize_t device_read(struct file *,  char *, size_t , loff_t * );
static ssize_t device_write(struct file *, const char *,size_t , loff_t *);
static int device_release(struct inode *, struct file *);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
static int device_ioctl(struct inode *node, struct file *filep, unsigned int cmd,unsigned long arg);
#else
static long device_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);
#endif





// Hardware PINS
static int led_pin = 17;
static int optical_switch_pin = 22;



#define RBUF_LEN 4096
#define WBUF_LEN 4096


static int wbuf[WBUF_LEN];


/**********************/#include "query_ioctl.h"
/* callback functions */
/**********************/

static struct file_operations file_ops = {

	.owner 		= THIS_MODULE,

	.open 		= device_open,

	.release 	= device_release,

	.write		= device_write,

	.read 		= device_read,
	
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
	.ioctl		= device_ioctl,
#else
	.unlocked_ioctl	= device_ioctl,
#endif

};


/*************************************************************/
/* called when 'open' system call is done on the device file */
/*************************************************************/

static int device_open(struct inode *ino, struct file *filep)
{
	

	if(is_device_open)
	{
		printk(KERN_ERR RFBB_DRIVER_NAME ": Already opened\n"); 
		return -EBUSY;
	}

	
	try_module_get(THIS_MODULE);
	
	is_device_open++;      
	return 0;
}

/**************************************************************/
/* called when 'close' system call is done on the device file */
/**************************************************************/

static int device_release(struct inode *node, struct file *file)
{	
	off();

  
	is_device_open--;          /* We're now ready for our next caller */ 
	module_put(THIS_MODULE);
	return 0;
}

/**************************************************************/
/* called when 'write' system call is done on the device file */
/**************************************************************/
static ssize_t device_write(struct file *file, const char *buf,size_t n, loff_t *ppos)
{
	int i;
	unsigned char count=0; 
	int result = 0;
       

	dprintk("device_write %d bytes\n", n);    


	if (n > WBUF_LEN)
	{
		dprintk("Too many elements (%d) in TX buffer\n", count);
		return -EINVAL;
	}
	
	result = copy_from_user(wbuf, buf, n);	
	if (result)
	{
		dprintk("Copy_from_user returns %d\n", result);    
		return -EFAULT;
	}

	for (i = 0; i < n; i++)
	printk("%d-Recibi (%.2X) \n",i, wbuf[i]);

	// count Number of times led blinks

	count=wbuf[0]-'0';			// remove ascii bias (0x30)


	for (i = 0; i < count; i++) {
		
		
			send_pulse(1000*pulse_dly_ms);   // pulse_dly_ms ms HI
		
			send_space(1000*pulse_dly_ms);   // pulse_dly_ms ms LO
	}
	off();  // turn pin off 
	
	return n;
}


/*************************************************************/
/* called when 'read' system call is done on the device file */
/*************************************************************/


static ssize_t device_read(struct file *filp,  char *msgusr, size_t length, loff_t *offset )
{

 	char geda_buffer[]="Light:  \n";  // 9 bytes
	
	ssize_t ret = 0;
static	ssize_t bytes_copied = 0;
static	ssize_t bytes_left= 20;	//something 
	
	   geda_buffer[7]='0'+(char)gpio_get_value(optical_switch_pin);
	   
	dprintk("1- ret %d , bytes_copied %d , bytes_left %d\n", ret, bytes_copied,bytes_left);
	
	if(bytes_left==0)
	{
		bytes_left=20; // next time 
	   	return 0;
	}		
	
	
	bytes_left=strlen(geda_buffer);

  	/* We transfer data to user space */
  	ret=copy_to_user(msgusr,geda_buffer,bytes_left);
	bytes_copied = bytes_left;
	
	bytes_left=0;
	
	
	dprintk("2- ret %d , bytes_copied %d , bytes_left %d\n", ret, bytes_copied,bytes_left);
	
	
	
	return bytes_copied;
}

//******************* IOCTL *******************************

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
static int device_ioctl(struct inode *node, struct file *filep, unsigned int cmd,unsigned long arg)
#else
static long device_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
#endif
{
query_arg_t q;	

	dprintk("IOCLT function \n");

 
    switch (cmd)
    {
        case QUERY_GET_VARIABLES:
            q.pulse_width =pulse_dly_ms ;
            q.param1 = 0;
            q.param2 = 1;

            if (copy_to_user((query_arg_t *)arg, &q, sizeof(query_arg_t)))
            {
                return -EACCES;
            }
            break;
        case QUERY_CLR_VARIABLES:
            
            q.param1 = 0;
                      
            break;
        case QUERY_SET_VARIABLES:
            if (copy_from_user(&q, (query_arg_t *)arg, sizeof(query_arg_t)))
            {
                return -EACCES;
            }
            pulse_dly_ms = q.pulse_width;
            
            break;
        default:
            return -EINVAL;
    }
 
    return 0;
}






// Pulse HI / LO

static void on(void)
{
	gpio_set_value(led_pin, 1);
}

static void off(void)
{
	gpio_set_value(led_pin, 0); 
}

#ifndef MAX_UDELAY_MS
#define MAX_UDELAY_US 5000
#else
#define MAX_UDELAY_US (MAX_UDELAY_MS*1000)
#endif

static void safe_udelay(unsigned long usecs)
{
	while (usecs > MAX_UDELAY_US) {
		udelay(MAX_UDELAY_US);
		usecs -= MAX_UDELAY_US;
	}
	udelay(usecs);
}

static void send_pulse(unsigned long length)
{   
	on();
	/* dprintk("send_pulse_gpio %ld us\n", length); */ 
	safe_udelay(length);
}

static void send_space(unsigned long length)
{
	off();
	/* dprintk("send_space_gpio %ld us\n", length); */
	safe_udelay(length);
}









// **********************************
// Everything starts here!!!
// **********************************

static int device_init_module(void)
{
	int result;

	result = device_init();
	if (result)
		goto exit_rfbb_exit; 

	result = init_port();
	if (result < 0)
		goto exit_rfbb_exit;

	printk(KERN_INFO
	       RFBB_DRIVER_NAME " " RFBB_DRIVER_VERSION " registered\n");
	dprintk("dev major = %d\n", rfbb_major);
	dprintk("Dany --  %d\n", 2);
	

	return 0;

exit_rfbb_exit:
	device_exit_module();
	return result;
}

static int device_init(void)
{

	int result;
	dev_t dev = 0;

	/*
	 * Dynamic major if not set otherwise.
	 */
      if (rfbb_major) {
              dev = MKDEV(rfbb_major, 0);
              result = register_chrdev_region(dev, 1, "rfbb");
      } else {
              result = alloc_chrdev_region(&dev, 0, 1, "rfbb");  // Dynamic major
              rfbb_major = MAJOR(dev);
      }
      if (result < 0) {
              printk(KERN_WARNING "rfbb: can't get major %d\n", rfbb_major);
              return result;
      }   

	rfbb_setup_cdev(rfbbDevs, 0, &file_ops);

	return 0;
}

/*
 * Set up the cdev structure for a device.
 */
static void rfbb_setup_cdev(struct cdev *dev, int minor,
			      struct file_operations *fops)
{
	int err, devno = MKDEV(rfbb_major, minor);

	cdev_init(dev, fops);
	dev->owner = THIS_MODULE;
	dev->ops = fops;
	err = cdev_add(dev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk(KERN_NOTICE "Error %d adding rfbb %d", err, minor);
}





// **********************************
// Everything Ends here!!!
// **********************************

static void device_exit_module(void)
{
	cdev_del(rfbbDevs);
	unregister_chrdev_region(MKDEV(rfbb_major, 0), 1);
	
	
		gpio_unexport(led_pin);
		gpio_free(led_pin);
		gpio_unexport(optical_switch_pin);
		gpio_free(optical_switch_pin);
	
	
	dprintk("cleaned up module\n");
}


static int init_port(void)
{
	int err = 0;

	err = hardware_init();
    	if (err)
		return err;
	else 
	    return 0;
}





static int hardware_init(void)
{
	
	int err = 0;


	printk(RFBB_DRIVER_NAME   " Empezando..678...ya \n");

/// https://www.kernel.org/doc/Documentation/gpio/gpio-legacy.txt

		/* Setup led pin */
		
			err = gpio_request_one(led_pin, GPIOF_OUT_INIT_LOW, "LED_CONTROL");
			if (err) {
				printk(KERN_ERR  RFBB_DRIVER_NAME
				       "Could not request LED pin, error: %d\n", err);
				return -EIO;
			}
			
		
			/* Setup led pin */
		
			err = gpio_request_one(optical_switch_pin, GPIOF_DIR_IN, "Optical_Switch_CONTROL");
			if (err) {
				printk(KERN_ERR  RFBB_DRIVER_NAME
				       "Could not request Optical_Switch_Pin, error: %d\n", err);
				return -EIO;
			}
                        
		

		

		/* Export pins and make them able to change from sysfs for troubleshooting */
		gpio_export(led_pin, 1);
	    gpio_export(optical_switch_pin, 1);
			
			
	return 0;
}




module_init(device_init_module);
module_exit(device_exit_module);



module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Enable debugging messages");




/*
sudo make KERNELDIR=/home/pi/rfdany/raspberrypi-linux-6f2064c

sudo insmod rfbb.kosnd_bcm2835

scp rfbb.c pi@10.0.100.219:/home/pi/rfdany/rf-bitbanger/myrfbb/

*/



