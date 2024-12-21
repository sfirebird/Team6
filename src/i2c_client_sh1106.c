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
#define SSH1306_OLED_ADR                0x3C
#define CLIENT_NAME                     "i2c_client_sh1106"

/*  declare and define a adapter structure to which the device will be connected on */
static struct i2c_adapter *oled_i2c_adap = NULL;
static struct i2c_client *oled_i2c_client = NULL;

/**
 *  SSH1106 Display initialization commands and routines, also compatible with SSD1306
 *  Refer the SSH1106 and use the initialization codes to initialize the display.
 *  
 *  STATUS = Pending
 */
static int SSH1106_display_init(void){
    return 0;
}

/**
 *  Will be invoked if the client device is found on the desired i2c bus
 *  Will only be called once
 */
static int oled_probe(struct i2c_client *client){
    int xRet = -1;

    /*  Initialize the display  */
    if((xRet = SSH1106_display_init())){
        pr_err("%s: error in initializing the sh1106 display!\n", CLIENT_NAME);
        goto dump_error;
    }
    pr_info("%s: probed!\n", CLIENT_NAME);
    return 0;

    dump_error:
        return xRet;
}

static void oled_remove(struct i2c_client *client){
    /*  Apply a mechanism to control something on the oled display before getting removed   */

    pr_info("%s: removed!\n", CLIENT_NAME);
}

/*  Structure containing client id, used for registration   */
static const struct i2c_device_id oled_id[] = {
    { .name = CLIENT_NAME},
    { }
};
MODULE_DEVICE_TABLE(i2c, oled_id);

/*  Instantiate the <struct i2c_driver> for the device  */
static struct i2c_driver oled_driver = {
    .driver = {
        .name = CLIENT_NAME,
        .owner = THIS_MODULE,
    },
    .probe = oled_probe,
    .remove = oled_remove,
    .id_table = oled_id,
};

/*  Specify board info to get the i2c_client    */
static struct i2c_board_info oled_i2c_board_info = {
        I2C_BOARD_INFO(CLIENT_NAME, SSH1306_OLED_ADR)
};

/*  OLED Module init function   */
static int __init oled_driver_init(void){
    int xRet = -1;
    
    /*  Get the i2c_adapter structure for the specified i2c bus */
    oled_i2c_adap = i2c_get_adapter(AVAILABLE_RPI_I2C_BUS);
    if(!oled_i2c_adap){
        pr_err("%s: Failed to get the i2c_adapter for the i2c%d bus!\n", CLIENT_NAME, AVAILABLE_RPI_I2C_BUS);
        goto err_dump;
    }

    
    oled_i2c_client = i2c_new_client_device(oled_i2c_adap, &oled_i2c_board_info);
    if(!oled_i2c_client){
        pr_err("%s: Failed to register the i2c_client!\n", CLIENT_NAME);
        goto err_dump;
    }

    i2c_put_adapter(oled_i2c_adap);

    i2c_add_driver(&oled_driver);

    pr_info("%s: Driver added!\n", CLIENT_NAME);
    return 0;

    err_dump:
        return xRet;
}

/*  OLED module Exit Function    */
static void __exit oled_driver_exit(void){
    //i2c_unregister_device(oled_i2c_client);
    i2c_del_driver(&oled_driver);
    pr_info("Driver Removed!\n");
}

module_init(oled_driver_init);
module_exit(oled_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Team6");
MODULE_DESCRIPTION("Basic Implmentation of I2C Linux Device Driver");
MODULE_VERSION("1.0");