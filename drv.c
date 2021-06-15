#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/hrtimer.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define LED1	15
#define buf_size 128

/* Define GPIOs for LEDs */
static struct gpio leds[] = {
		{  15, GPIOF_OUT_INIT_LOW, "LED 1" },
};

//Estructura para usar el timer
static struct hrtimer hr_timer;
static int led1_value = 0; //Valor del LED1
static int timer_count = 0; //Contador que va de 0 a 2
static int timer_interval = 3; //segundos de parpadeo

//Buffer para la transmision de datos
uint8_t *kernel_buffer;
//Buffer para lectura de datos
uint8_t *kernelRead;

/* Define GPIOs for BUTTONS */
static struct gpio buttons[] = {
		{ 22, GPIOF_IN, "BUTTON 1" },	// turns LED on
		{ 23, GPIOF_IN, "BUTTON 2" },	// turns LED off
};

//Variables usadas la configuracion de Char Dev Driver
dev_t first = 0;
static struct cdev c_dev; 	        // Global variable for the character device structure
static struct class *class_dev; 	// Global variable for the device class

/* Later on, the assigned IRQ numbers for the buttons are stored here */
static int button_irqs[] = { -1, -1 };

/////////////////////////////////////
static int __init mod_init(void);
static void __exit mod_exit(void);

static int my_open(struct inode *i, struct file *f);
static int my_close(struct inode *i, struct file *f);
static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off);
static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off);
static irqreturn_t button_isr(int irq, void *data);
enum hrtimer_restart timer_callback(struct hrtimer *timer_for_restart);
/////////////////////////////////////

static int my_open(struct inode *i, struct file *f)
{
    printk(KERN_INFO "alarma: open()\n");
    return 0;
}
static int my_close(struct inode *i, struct file *f)
{
    printk(KERN_INFO "alarma: close()\n");
    return 0;
}

/*
** Esta funcion sera llamada cuando se lee el fichero de dispositivo
*/
static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "alarma: read()\n");
	if (copy_to_user(buf, kernel_buffer, buf_size) > 0)
    {
        pr_info("Error while reading data\n");
        return -1;
    }
    pr_info("Data is being read\n");
    return 0;
}

/*
** Esta funcion sera llamada cuando se escribe en el fichero de dispositivo
*/
static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "alarma: write()\n");
	memset(kernelRead, '\0', buf_size);
    if (copy_from_user(kernelRead, buf, len) > 0)
    {
        pr_info("Error while writing data\n");
        return len;
    }

	kstrtol(kernelRead, 10, led1_value);

	/*
    if ( kernelRead[0] == 'p' ){
         polaridad *= -1;
         pr_info("Data is being written, %d\n", polaridad);
    }
    else if( kernelRead[0] == 'u' ){ //020
        kernelRead[0] = '0';
        pr_info("%s\n", kernelRead);
        if(kstrtol(kernelRead, 10, &escalaboton1) != 0)
        {
            pr_info("Failed converting Btn1 scale, exiting.\n");
            goto fail;
        }
        pr_info("Escala boton 1 cambiada a %ld\n", escalaboton1);
    }
    else if( kernelRead[0] == 'd' ){
        kernelRead[0] = '0';
        pr_info("%s\n", kernelRead);
        if(kstrtol(kernelRead, 10, &escalaboton2) != 0)
        {
            pr_info("Failed converting Btn2 scale, exiting.\n");
            goto fail;
        }
        pr_info("Escala boton 2 cambiada a %ld\n", escalaboton2);
    }*/

    return len;
fail:
    memset(kernelRead, '\0', buf_size);
    return len;
}

static struct file_operations fops =
{
    //Se definen las funciones de entrada/salida
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
    .read = my_read,
    .write = my_write
};

/*
 * The interrupt service routine called on button presses
 */
static irqreturn_t button_isr(int irq, void *data)
{
    switch (timer_count)
    {
    case 0:
        timer_interval = 3;
        timer_count++;
        break;    
    case 1:
        timer_interval = 2;
        timer_count++;
        break;
    case 2:
        timer_interval = 1;
        timer_count = 0;
        break;
    
    default:
        //error
        break;
    }

    /*
	if(irq == button_irqs[0]){// && !gpio_get_value(leds[0].gpio)) {
		if(gpio_get_value(leds[0].gpio) == 1){			
			gpio_set_value(leds[0].gpio, 0);
		}
		else{
			gpio_set_value(leds[0].gpio, 1);
		}
	}
	else if(irq == button_irqs[1] && gpio_get_value(leds[0].gpio)) {
			gpio_set_value(leds[0].gpio, 0);
	}*/

	return IRQ_HANDLED;
}

/*
 * Timer function called periodically
 */
enum hrtimer_restart timer_callback(struct hrtimer *timer_for_restart)
{
  	ktime_t currtime;
	ktime_t interval;

  	currtime  = ktime_get();
  	interval = ktime_set(timer_interval, 0);

  	hrtimer_forward(timer_for_restart, currtime, interval);

	gpio_set_value(LED1, led1_value);

	memset(kernel_buffer, '\0', buf_size);
    sprintf(kernel_buffer, "%d", led1_value);
    //pr_info("%s\n", kernel_buffer);

	led1_value = !led1_value;

	return HRTIMER_RESTART;
}

/*
 * Module init function
 */
static int __init mod_init(void)
{
    ktime_t interval;
    int ret;

    
    /*Alocando el numero mayor*/
    if ((alloc_chrdev_region(&first, 0, 1, "alarma")) < 0)
    {
        pr_err("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d \n", MAJOR(first), MINOR(first));

    /*Creando estructura cdev*/
    cdev_init(&c_dev, &fops);

    /*Agregando el CDD al sistema*/
    if ((cdev_add(&c_dev, first, 1)) < 0)
    {
        pr_err("Cannot add the device to the system\n");
        goto r_class;
    }

    /*Creando estrucutra de clase*/
    if ((class_dev = class_create(THIS_MODULE, "class_alarma")) == NULL)
    {
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }

    /*Creando dispositivo*/
    if ((device_create(class_dev, NULL, first, NULL, "alarma")) == NULL)
    {
        pr_err("Cannot create the Device 1\n");
        goto r_device;
    }
    
    //////////////////////////////////////////////////

	//Aloco memoria para el String kernel_buffer
    if ((kernel_buffer = kmalloc(buf_size, GFP_KERNEL)) == 0)
    {
        pr_info(KERN_INFO "Cannot allocate the memory to the kernel\n");
        return -1;
    }
    if ((kernelRead = kmalloc(buf_size, GFP_KERNEL)) == 0)
    {
        pr_info(KERN_INFO "Cannot allocate the memory to the kernel\n");
        return -1;
    }

	/////////////////////////////////////////////////
    
	printk(KERN_INFO "%s\n", __func__);

    	// register LED gpios
	ret = gpio_request_array(leds, ARRAY_SIZE(leds));

	if (ret) {
		printk(KERN_ERR "Unable to request GPIOs for LEDs: %d\n", ret);
		return ret;
	}

	// register BUTTON gpios
	ret = gpio_request_array(buttons, ARRAY_SIZE(buttons));

	if (ret) {
		printk(KERN_ERR "Unable to request GPIOs for BUTTONs: %d\n", ret);
		goto fail1;
	}

	printk(KERN_INFO "Current button1 value: %d\n", gpio_get_value(buttons[0].gpio));

	ret = gpio_to_irq(buttons[0].gpio);

	if(ret < 0) {
		printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
		goto fail2;
	}

	button_irqs[0] = ret;

	printk(KERN_INFO "Successfully requested BUTTON1 IRQ # %d\n", button_irqs[0]);

	ret = request_irq(button_irqs[0], button_isr, IRQF_TRIGGER_RISING /* | IRQF_DISABLED */, "gpiomod#button1", NULL);

	if(ret) {
		printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
		goto fail2;
	}


	ret = gpio_to_irq(buttons[1].gpio);

	if(ret < 0) {
		printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
		goto fail2;
	}

	button_irqs[1] = ret;

	printk(KERN_INFO "Successfully requested BUTTON2 IRQ # %d\n", button_irqs[1]);

	ret = request_irq(button_irqs[1], button_isr, IRQF_TRIGGER_RISING /* | IRQF_DISABLED */, "gpiomod#button2", NULL);

	if(ret) {
		printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
		goto fail3;
	}

    /* init timer, add timer function */
	interval = ktime_set(timer_interval, 0); //1 second, 0 nanoseconds
	hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hr_timer.function = &timer_callback;
	hrtimer_start(&hr_timer, interval, HRTIMER_MODE_REL);

	return 0;

// cleanup what has been setup so far
fail3:
	free_irq(button_irqs[0], NULL);

fail2:
	gpio_free_array(buttons, ARRAY_SIZE(leds));

fail1:
	gpio_free_array(leds, ARRAY_SIZE(leds));
r_device:
    class_destroy(class_dev);
r_class:
    unregister_chrdev_region(first, 1);

	return ret;
}

/**
 * Module exit function
 */
static void __exit mod_exit(void)
{
	int i;
    int ret = 0;

	printk(KERN_INFO "%s\n", __func__);  

	//Se libera la memoria del Kernel
    kfree(kernel_buffer); 
    kfree(kernelRead); 

	// free irqs
	free_irq(button_irqs[0], NULL);
	free_irq(button_irqs[1], NULL);

    // check errors in hrtimer
    ret = hrtimer_cancel(&hr_timer);
	if(ret) {
		printk("Failed to cancel timer.\n");
	}

	// turn all LEDs off
	for(i = 0; i < ARRAY_SIZE(leds); i++) {
		gpio_set_value(leds[i].gpio, 0);
	}    

	// unregister
	gpio_free_array(leds, ARRAY_SIZE(leds));
	gpio_free_array(buttons, ARRAY_SIZE(buttons));

    //Se elimina la configuracion de /dev
    device_destroy(class_dev, first);
    class_destroy(class_dev);
    cdev_del(&c_dev);
    unregister_chrdev_region(first, 1);
    printk("Eliminando device driver de alarma.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Maca y Fede");
MODULE_DESCRIPTION("Modulo de Kernel para parpadear un LED y otras cositas");

module_init(mod_init);
module_exit(mod_exit);