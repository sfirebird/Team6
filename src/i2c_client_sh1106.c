/*  This is how you will be able to load your module into kernel
    Provides a set a macros and Kernel API to load your module.
*/
#include <linux/module.h>

/**
 *  For now we only use __init and __exit macro
 */
#include <linux/init.h>

/**
 *  Sanket: I still do not have a very good understanding of this header file
 *  Related to dynamic memory allocation?
 */
#include <linux/slab.h>

/*  Expose i2c related structures and api (core_i2c)    */
#include <linux/i2c.h>

#include <linux/kernel.h>

#define AVAILABLE_RPI_I2C_BUS           1U
#define SSH1306_OLED_ADR                0X3C
#define CLIENT_NAME                     "ssh1306_oled"

/*  declare and define a adapter structure to which the device will be connected on */
static struct i2c_adapter *oled_i2c_adap = NULL;
static struct i2c_client *oled_i2c_client = NULL;

/**
 *  SSH1106 Display initialization commands and routines, also compatible with SSD1306
 * 
 * 
 * 
 */
static int SSH1106_display_init(void){

}

/**
 *  Will be invoked if the client device is found on the desired i2c bus
 *  Will only be called once
 */
static int oled_probe(struct i2c_client *client){
    int xRet = -1;;

    /*  Initialize the display  */
    if((xRet = SSH1106_display_init())){
        pr_err("%s: error in initializing the sh1106 display!\n");
        goto dump_error;
    }
    pr_info("%s: probed!\n", CLIENT_NAME);
    return 0;

    dump_error:
        return xRet;
}

static void oled_remove(struct i2c_client *client){
    /*  Apply a mechanism to control something on the oled display before getting removed   */

    pr_info("%s: removed!\n");
    return 0;
}

/*  Structure containing client id, used for registration   */
static const struct i2c_device_id oled_id[] = {
    { .name = "SSH1106"},
}
MODULE_DEVICE_TABLE(i2c, oled_id);