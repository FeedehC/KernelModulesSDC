#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>

// En el kernel, el tipo dev_t (definido en <linux/types.h>) se usa para contener 
// números de dispositivos, tanto MAJOR como MINOR. A partir del kernel v2.6.0
// dev_t es una cantidad de 32 bits con 12 bits para MAJOR y 20 bits para MINOR.
// -----------------------------------------------------------------------------
// alloc_chrdev_region() registra un par <major, minor> para un CDD.
// Devuelve en el primer parámetro en # MAJOR disponible partiendo de 0 
// y en este caso solicitando 3 MINOR para ese MAJOR. 
// MAJOR(fist) y MINOR(first) devuelven los respectivos valores de first.
// -----------------------------------------------------------------------------
// ¿ Dónde ver MAJOR(first) ? 

// $cat /proc/devices | grep SdeC
// -----------------------------------------------------------------------------
// ¿ Dónde ver los tres MINOR(first) asociados a MAJOR(first) ?
// Se deben crear con :
// $sudo mknode /dev/SdeC_drv2_0 c 244 0
// $sudo mknode /dev/SdeC_drv2_1 c 244 1
// $sudo mknode /dev/SdeC_drv2_2 c 244 2

// El valor de 244 puede cambar..!!

static dev_t first; 

static int __init drv2_init(void) /* Constructor */
{
    int ret;

    printk(KERN_INFO "SdeC: drv2 Registrado exitosamente..!!");

    // alloc_chrdev_region calcula (¿ obtiene ?) dinámicamente un MAJOR libre 
    // (lo devuelve en first) y registra 3 números de device MINOR a partir de
    // <el mayor libre, primer menor>, con el nombre SdeC_drv2. 
    // Si devuelve 0, está todo bien..!!

    if ((ret = alloc_chrdev_region(&first, 0, 3, "SdeC_Driver2")) < 0)
    {
        return ret;
    }
    printk(KERN_INFO "SdeC_drv2: <Major, Minor>: <%d, %d>\n", MAJOR(first), MINOR(first));
    return 0;
}

static void __exit drv2_exit(void) /* Destructor */
{
    unregister_chrdev_region(first, 3);
    printk(KERN_INFO "SdeC_drv2: dice Adios mundo kernel..!!\n");
}

module_init(drv2_init);
module_exit(drv2_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Miguel Solinas <miguel.solinas@unc.edu.ar>");
MODULE_DESCRIPTION("Segundo driver de SdeC");