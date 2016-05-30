/*Writen by dany rev 1.0 - 2016A
 * rfbb.c   Parallel port version 
 *
 * rfbb -   Device driver that sends data to gpio pins
 * 
 * Sample Calls 
 * CLI (command Line)
 * echo echo 255 > /dev/rfbb 	All gpio pins on
 * echo echo 255 > /dev/rfbb 	All gpio pins on
 * echo 1 > /dev/rfbb           turn On D0
 * echo 128 > /dev/rfbb 		turn On D7
 * 
 *  In C
 * 
 *  char msg[]="05";
 *  fp=fopen("/dev/rfbb", "r+");
 * 
 *
 *	i=0;
 *
 *	while(msg[i])
 *	fputc((int)msg[i++],fp);
 *
 *	fputc(0,fp);
 *	
 *	fclose(fp);
 *
 * 
 *  Python: 
 *  f = open('/dev/rfbb', 'w')
 *  f.write("2\n")
 *  f.close()
 *
 * 
 * 
 * For pin order (D7..D0) please read leds_gpios array definition below 
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

//*****************Select Hardware Type: Serial or parallel*********************************
#define PARA		0
#define SERIAL		1

#define HARDWARE 	PARA
//*****************************************************************************************




#define RFBB_DRIVER_VERSION "0.05"
#define RFBB_DRIVER_NAME "rfbb"
#define MAX_RFBB_DEVS 1 /* One device only */
/*
 * We export one rfbb device.
 */
static struct cdev rfbbDevs[MAX_RFBB_DEVS];
static int rfbb_major = 0; /* use dynamic major number assignment */
static int debug = 1;
static int device_open = 0;

static DEFINE_MUTEX(read_lock);

#define dprintk(fmt, args...)					\
	do {							\
		if (debug)					\
			printk(KERN_DEBUG RFBB_DRIVER_NAME ": "	\
			       fmt, ## args);			\
	} while (0)

/* forward declarations */

static void rfbb_exit_module(void);
static int rfbb_init(void);
static int init_port(void);
static void rfbb_setup_cdev(struct cdev *dev, int minor,struct file_operations *fops);
static int hardware_init(void);

static void on(void); /* TX signal on */
static void off(void); /* TX signal off */
static void send_pulse(unsigned long length);
static void send_space(unsigned long length);
void Set_Pin(int pin);
void Clr_Pin(int pin);
unsigned int spiWrite(unsigned int spidata);


static unsigned int data_step=0;			// data_step is added to incomming data

// Hardware PIN used for pulse generation
static int tx_pin = 17;


#define RBUF_LEN 4096
#define WBUF_LEN 4096

#if HARDWARE == SERIAL


static struct gpio Spi_gpios[] = {
		
		{ 17, GPIOF_OUT_INIT_HIGH, "LOAD" }, 	/* default to ON */		
		{ 22, GPIOF_OUT_INIT_LOW,  "SCK" }, 	/* default to OFF */		
		{ 23, GPIOF_OUT_INIT_LOW,  "DATA_IN" }, /* default to OFF */
		
	};

#define PIN_LOAD 17
#define	PIN_SCK  22
#define	PIN_DATA_IN 23



#elif HARDWARE == PARA

static struct gpio leds_gpios[] = {
		{ 4,  GPIOF_OUT_INIT_LOW,  "D0" }, /* default to OFF */
		{ 17, GPIOF_OUT_INIT_HIGH, "D1" }, /* default to ON */
		{ 27, GPIOF_OUT_INIT_LOW,  "D2" }, /* default to OFF */
		{ 22, GPIOF_OUT_INIT_LOW,  "D3" }, /* default to OFF */
		{ 18, GPIOF_OUT_INIT_HIGH, "D4" }, /* default to ON */
		{ 23, GPIOF_OUT_INIT_LOW,  "D5" }, /* default to OFF */
		{ 24, GPIOF_OUT_INIT_LOW,  "D6" }, /* default to OFF */
		{ 25, GPIOF_OUT_INIT_LOW,  "D7" }, /* default to OFF */

	};

#endif


static char wbuf[WBUF_LEN];

// Pulse HI / LO

static void on(void)
{
	gpio_set_value(tx_pin, 1);
}

static void off(void)
{
	gpio_set_value(tx_pin, 0); 
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

/*	Call examples
		
//			send_pulse(100);   // 100 us HI
		
//			send_space(100);   // 100 us LO

//			send_pulse(10000);   // 10 ms HI
		
//			send_space(10000);   // 10 ms LO

//			send_pulse(1000000);   // 1 seg HI
		
//			send_space(1000000);   // 1 seg LO


*/



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


void Set_Pin(int pin)
{
	gpio_set_value(pin, 1);
}
void Clr_Pin(int pin)
{
	gpio_set_value(pin, 0);
}

static ssize_t rfbb_read(struct file *filp,  char *msgusr, size_t length, loff_t *offset )
{

 	char geda_buffer[]="123AB\n";
	
		ssize_t ret = 0;
static	ssize_t bytes_copied = 0;
static	ssize_t bytes_left= 10;	//something 
	
	
	if(bytes_left==0)
	{
		bytes_left=10; // next time 
	   	return 0;
	}		
	
	
	bytes_left=strlen(geda_buffer);

  	/* We transfer data to user space */
  	ret=copy_to_user(msgusr,geda_buffer,bytes_left);
	bytes_copied = bytes_left;
	
	bytes_left=0;
	
	
	dprintk("ret %d , bytes_copied %d , bytes_left %d\n", ret, bytes_copied,bytes_left);
	
	
	
	return bytes_copied;
}


static ssize_t rfbb_write(struct file *file, const char *buf,
			 size_t n, loff_t *ppos)
{
	int i;
	unsigned int spidata=0; 
	int result = 0;
       

	dprintk("rfbb_write %d bytes\n", n);    


	if (n > WBUF_LEN)
	{
		dprintk("Too many elements (%d) in TX buffer\n", spidata);
		return -EINVAL;
	}
	
	result = copy_from_user(wbuf, buf, n);	
	if (result)
	{
		dprintk("Copy_from_user returns %d\n", result);    
		return -EFAULT;
	}

	for (i = 0; i < n; i++)					// Show what we recieved from User Space 
	printk("%d-Recibi (%X) \n",i, wbuf[i]);
	
	for (i = 0; i < n; i++)					// Find the first non ASCII number 
	{
		if (wbuf[i] < '0' || wbuf[i] > '9')
			wbuf[i]=0x00;  					//and replace it with EOT (0x00) 
	}

	    
    result=kstrtoint(wbuf, 10, &spidata);		// Convert received string to int 
	
	if (result)								// check for conversion error
	{
		dprintk("kstrtoint error %d\n", result);    
		return -EFAULT;
	}

	spidata = data_step + spidata;   // data_step is added to incomming data
	
	spiWrite(spidata);						// Send Data to hardware

	return n;
}


#if HARDWARE == PARA						

unsigned int spiWrite(unsigned int spidata) // Send Data to hardware
{
	int i;	
		
		dprintk("entering on spiwrite %d\n", spidata);  
	// Check if spidata is inside of 8 bit unsigned range
	
	if(spidata >= 0 && spidata <= 255)   // write to leds only when in range 
	{
		// Set / Clr individual pins
		for (i=0; i< sizeof(leds_gpios)/sizeof(leds_gpios[0]); i++)
		{
				
				if(spidata&0x01)
				{
					Set_Pin(leds_gpios[i].gpio);
					printk("-Spidata (%X) SET pin %d\n",spidata,leds_gpios[i].gpio);
				
				}
				else
				{
				
					Clr_Pin(leds_gpios[i].gpio);
					printk("-Spidata (%X) CLR pin %d\n",spidata,leds_gpios[i].gpio);
				}
				
			spidata>>=1;   // Next Bit 
		}
	}	
	
	return 0;
}


#elif HARDWARE == SERIAL

unsigned int spiWrite(unsigned int spidata)	// Send Data to hardware
{
	unsigned int i;	
	
	dprintk("entering on spiwrite %d\n", spidata);
	
	Clr_Pin(PIN_SCK);     // Load and CK low (steady state)
	Clr_Pin(PIN_LOAD); 
		
		
	// Check if spidata is inside of 8 bit unsigned range
	
	if(spidata >= 0 && spidata <= 255)   // write to leds only when in range 
	{
		// Set / Clr individual pins
		for (i=0; i<8; i++)
		{
				
				if(spidata & 0x80)
				{
					Set_Pin(PIN_DATA_IN);	
					printk("-Spidata (%X) SET pin \n",spidata);
				
				}
				else
				{
				
					Clr_Pin(PIN_DATA_IN);	
					printk("-Spidata (%X) CLR pin \n",spidata);
				}
				
				Clr_Pin(PIN_SCK); 									// Shift out each bit
				Set_Pin(PIN_SCK);
								
				
			spidata<<=1;   // Next Bit 
		}
		
		
		Set_Pin(PIN_LOAD);							// transfer shift register to output latch 
		
	}	
	
	return 0;
}


#endif

// ************************IOCTL**************************************

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
static int device_ioctl(struct inode *node, struct file *filep, unsigned int cmd,unsigned long arg)
#else
static long device_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
#endif
{
query_arg_t q;	

	dprintk("IOCTL function \n");

 
    switch (cmd)
    {
        case QUERY_GET_VARIABLES:
            q.data_step = data_step ;
            q.param1 = 0;
            q.param2 = 1;
		
			dprintk("IOCTL: QUERY_GET_VARIABLES  data_step:%d  param1:%d param2:%d \n"
			,q.data_step,q.param1,q.param2 ); 

            if (copy_to_user((query_arg_t *)arg, &q, sizeof(query_arg_t)))
            {
                return -EACCES;
            }
            break;
        case QUERY_CLR_VARIABLES:
            
            q.param1 = 0;
            dprintk("IOCTL: QUERY_CLR_VARIABLES  data_step:%d  param1:%d param2:%d \n"
			,q.data_step,q.param1,q.param2 ); 
            
            
                      
            break;
        case QUERY_SET_VARIABLES:
            if (copy_from_user(&q, (query_arg_t *)arg, sizeof(query_arg_t)))
            {
                return -EACCES;
            }
            data_step = q.data_step;
            
            dprintk("IOCTL: QUERY_SET_VARIABLES  data_step:%d  param1:%d param2:%d \n"
			,q.data_step,q.param1,q.param2 ); 
            
            
            break;
        default:
            return -EINVAL;
    }
 
    return 0;
}


// **********************END**IOCTL**********************************

static int rfbb_open(struct inode *ino, struct file *filep)
{
	

	if(device_open)
	{
		printk(KERN_ERR RFBB_DRIVER_NAME ": Already opened\n"); 
		return -EBUSY;
	}

	try_module_get(THIS_MODULE);
	
	device_open++;      
	return 0;
}

static int rfbb_release(struct inode *node, struct file *file) // close file ojo
{	
	/// off();


	/* lirc_buffer_free(&rbuf); */ 
  
	device_open--;          /* We're now ready for our next caller */ 
	module_put(THIS_MODULE);
	return 0;
}

static struct file_operations rfbb_fops = {
	.owner = THIS_MODULE,
	.open = rfbb_open,
	.release = rfbb_release,
	.write	= rfbb_write,
	.read = rfbb_read,
	

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
	.ioctl		= device_ioctl,
#else
	.unlocked_ioctl	= device_ioctl,
#endif




};


// **********************************
// Everything starts here!!!
// **********************************

static int rfbb_init_module(void)
{
	int result;

	result = rfbb_init();
	if (result)
		goto exit_rfbb_exit; 

	result = init_port();
	if (result < 0)
		goto exit_rfbb_exit;

	printk(KERN_INFO
	       RFBB_DRIVER_NAME " " RFBB_DRIVER_VERSION " registered\n");
	dprintk("dev major = %d\n", rfbb_major);
	dprintk("Dany %d\n", 2);
	

	return 0;

exit_rfbb_exit:
	rfbb_exit_module();
	return result;
}

static int rfbb_init(void)
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

	rfbb_setup_cdev(rfbbDevs, 0, &rfbb_fops);

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

static void rfbb_exit_module(void)
{
	cdev_del(rfbbDevs);
	unregister_chrdev_region(MKDEV(rfbb_major, 0), 1);
	
#if HARDWARE == SERIAL

		gpio_unexport(PIN_LOAD);
		gpio_unexport(PIN_SCK);
		gpio_unexport(PIN_DATA_IN);
		
		/* Free Requested Pins  (don't forget this othewise you MUST reboot the board)*/
		
		//gpio_free(tx_pin);   // one pin
		
		gpio_free_array(Spi_gpios, ARRAY_SIZE(Spi_gpios));  // many pins at once
		

#elif HARDWARE == PARA

		gpio_unexport(tx_pin);
		gpio_unexport(4);
		
		/* Free Requested Pins  (don't forget this othewise you MUST reboot the board)*/
		
		//gpio_free(tx_pin);
		
		gpio_free_array(leds_gpios, ARRAY_SIZE(leds_gpios));  // many pins at once
	
#endif
	
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


	printk(RFBB_DRIVER_NAME   " Empezando.....ya 2016 SPI / PP \n");


#if HARDWARE == SERIAL

	 printk(RFBB_DRIVER_NAME   " Empezando.....ya 2016  Serial Mode \n");
		
		
	//err = gpio_request_one(tx_pin, GPIOF_OUT_INIT_LOW, "RFBB_TX");  // request one pin
	err = gpio_request_array(Spi_gpios, ARRAY_SIZE(Spi_gpios));       // request many pin at once

	if (err) 
	{
		printk(KERN_ERR  RFBB_DRIVER_NAME
		         "Could not request RFBB TX pin, error: %d\n", err);
		return -EIO;
	}
                      

	/* Export pins and make them able to change from sysfs for troubleshooting */

	gpio_export(PIN_LOAD, 0);
	gpio_export(PIN_SCK, 0);
	gpio_export(PIN_DATA_IN, 0);
		



#elif HARDWARE == PARA

		printk(RFBB_DRIVER_NAME   " Empezando.....ya 2016  Parallel Mode \n");

		//err = gpio_request_one(tx_pin, GPIOF_OUT_INIT_LOW, "RFBB_TX");  // request one pin
		err = gpio_request_array(leds_gpios, ARRAY_SIZE(leds_gpios));     // request many pin at once
		if (err) {
						printk(KERN_ERR  RFBB_DRIVER_NAME
				       "Could not request RFBB TX pin, error: %d\n", err);
						return -EIO;
				 }
                        

		/* Export pins and make them able to change from sysfs for troubleshooting */
		gpio_export(tx_pin, 0);
		gpio_export(4, 0);
		
		
#endif
		
	return 0;
}




module_init(rfbb_init_module);
module_exit(rfbb_exit_module);

MODULE_DESCRIPTION("RF transmitter and receiver driver for embedded CPU:s with GPIO. Based on lirc_serial");
MODULE_AUTHOR("Tord Andersson");
MODULE_LICENSE("GPL");

module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Enable debugging messages");




/*

sudo su

./start.sh
 
sudo make KERNELDIR=/home/pi/rfdany/raspberrypi-linux-6f2064c

insmod rfbb.ko

rmmod rfbb

scp rfbb.c pi@10.0.100.219:/home/pi/rfdany/rf-bitbanger/myrfbb/

*/



