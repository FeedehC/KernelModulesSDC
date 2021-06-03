// Bibliografía a consultar:
// https://www.kernel.org/doc/htmldocs/kernel-api/index.html

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

static dev_t first; 		// Vble global para device number "first"
static struct cdev c_dev; 	// Global variable for the character device structure
static struct class *cl; 	// Global variable for the device class

static int my_open(struct inode *i, struct file *f)
{
    printk(KERN_INFO "SdeC_drv3: open()\n");
    return 0;
}
static int my_close(struct inode *i, struct file *f)
{
    printk(KERN_INFO "SdeC_drv3: close()\n");
    return 0;
}
static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "SdeC_drv3: read()\n");
    return 0;
}
static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "SdeC_drv3: write()\n");
    return len;
}

static struct file_operations pugs_fops =
{
    // Dentro de file_operations defino las funciones que voy a implementar..!!

    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
    .read = my_read,
    .write = my_write
};

static int __init drv3_init(void) /* Constructor */
{
    int ret;
    struct device *dev_ret;

    printk(KERN_INFO "SdeC: drv3 Registrado exitosamente..!!\n");
    
    if ((ret = alloc_chrdev_region(&first, 0, 1, "SdeC_drv3")) < 0)
    {
        return ret;
    }

    // En versiones del kernel 2.4 y anteriores, la creación automática de 
    // "device files" las realizaba el propio kernel llamando a las API
    // apropiadas de devfs.

    // A medida que el kernel evolucionó, los desarrolladores del kernel 
    // se dieron cuenta de que los "device files" son más una cuestión del 
    // espacio de usuario y, por lo tanto, como política, solo ellos debían 
    // tratarlo, y no el kernel.

    // Con esta idea, ahora el kernel ofrece información sobre los dispositivos, 
    // organizado por clases en la carpeta /sys/class.
    // En particular para ESTE CDD, se creará /sys/class/SdeC_cdrv.

    // Con class_create se crea un puntero a un objeto de la clase device 
    // particular (chardrv).


    if (IS_ERR(cl = class_create(THIS_MODULE, "SdeC_cdrv")))
    {
        unregister_chrdev_region(first, 1);
        return PTR_ERR(cl);
    }

    // Ese puntero "cl" será utilizado en la llamada a device_create, junto a 
    // los valores <MAJOR, MINOR> contenidos en "first" y el nombre que se le 
    // dará a esta nueva entrada.

    if (IS_ERR(dev_ret = device_create(cl, NULL, first, NULL, "SdeC_drv3")))
    {
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return PTR_ERR(dev_ret);
    }

    // Lo que queda es inicializar cdev, recordando la estructura file_operations, 
    // preparándola para agregar al sistema con cdev_add.

    cdev_init(&c_dev, &pugs_fops);
    if ((ret = cdev_add(&c_dev, first, 1)) < 0)
    {
        device_destroy(cl, first);
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return ret;
    }
    return 0;
}

static void __exit drv3_exit(void) /* Destructor */
{
    cdev_del(&c_dev);
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    printk(KERN_INFO "SdeC_drv3: dice Adios mundo kernel..!!\n");
}

module_init(drv3_init);
module_exit(drv3_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Miguel Solinas <miguel.solinas@unc.edu.ar>");
MODULE_DESCRIPTION("Tercer driver de SdeC");