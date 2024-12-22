#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/err.h>

#include "../inc/sh1106.h"

#define AVAILABLE_RPI_I2C_BUS           1U
#define SSH1306_OLED_ADR                0x3C
#define CLIENT_NAME                     "i2c_client_sh1106"

#define OK                              0U

/*  display initialization commands */
unsigned char display_off_cmd[]     = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_DISPLAY_OFF};
unsigned char clk_div_cmd[]         = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_DISPLAY_CLK_DIV, 0x80};
unsigned char multiplex_ratio[]     = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_MULTIPLEX_RATIO, 0x3F};
unsigned char display_offset[]      = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_DISPLAY_OFFSET, 0x00};
unsigned char display_start_line[]  = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_DISPLAY_START_L};
unsigned char unknown_cmd_1[]       = {OLED_CNT_BYTE_CMD_STREAM, 0x8D, 0x14};
unsigned char unknown_cmd_2[]       = {OLED_CNT_BYTE_CMD_STREAM, 0x20, 0x00};
unsigned char segment_remap[]       = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_SEG_REMAP_H};
unsigned char scan_dir[]            = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_COM_OP_SCAN_V};
unsigned char com_hw_pad[]          = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_COM_PAD_HW, 0x12};
unsigned char contrast[]            = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_CONTRAST, 0xCF};
unsigned char precharge[]           = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_PRECHARGE, 0xF1};
unsigned char vcom[]                = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_VCOM_LEVEL, 0x40};
unsigned char unknown_cmd_3[]       = {OLED_CNT_BYTE_CMD_STREAM, 0x2E};
unsigned char display_style[]       = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_ENT_DISPLAY_RAM};
unsigned char display_style1[]      = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_DISPLAY_NORMAL};
unsigned char setpage[]             = {OLED_CNT_BYTE_CMD_STREAM, 0x10, 0x2, OLED_SET_PAGE_ADR};

/*  declare and define a adapter structure to which the device will be connected on */
static struct i2c_adapter *oled_i2c_adap = NULL;
static struct i2c_client *oled_i2c_client = NULL;

/**
 *  Write message to the i2c_bus, takes a pointer to the char data type.
 */
static int sh1106_write(const unsigned char *buf, int size)
{
    int xRet = OK;

    if(!oled_i2c_client){
        pr_err("%s: oled_i2c_client!\n", CLIENT_NAME);  
        return -ENODATA;
    }
    xRet = i2c_master_send(oled_i2c_client, buf, size);
    if(xRet < 0){
        pr_err("%s: Error with i2c_master_send: %d", CLIENT_NAME, xRet);
        return xRet;
    }
    return 0;
}
/**
 *  Send the display initialization commands to the oled display
 * 
 */
static int SSH1106_display_init(void)
{
    int xRet = OK;

    if ((xRet = sh1106_write(display_off_cmd, sizeof(display_off_cmd))))        goto err_dump;
    if ((xRet = sh1106_write(clk_div_cmd, sizeof(clk_div_cmd))))                goto err_dump;
    if ((xRet = sh1106_write(multiplex_ratio, sizeof(multiplex_ratio))))        goto err_dump;
    if ((xRet = sh1106_write(display_offset, sizeof(display_off_cmd))))         goto err_dump;
    if ((xRet = sh1106_write(display_start_line, sizeof(display_start_line))))  goto err_dump;
    if ((xRet = sh1106_write(unknown_cmd_1, sizeof(unknown_cmd_1))))            goto err_dump;
    if ((xRet = sh1106_write(unknown_cmd_2, sizeof(unknown_cmd_2))))            goto err_dump;
    if ((xRet = sh1106_write(segment_remap, sizeof(segment_remap))))            goto err_dump;
    if ((xRet = sh1106_write(scan_dir, sizeof(scan_dir))))                      goto err_dump;
    if ((xRet = sh1106_write(com_hw_pad, sizeof(com_hw_pad))))                  goto err_dump;
    if ((xRet = sh1106_write(contrast, sizeof(contrast))))                      goto err_dump;
    if ((xRet = sh1106_write(precharge, sizeof(precharge))))                    goto err_dump;
    if ((xRet = sh1106_write(vcom, sizeof(vcom))))                              goto err_dump;
    if ((xRet = sh1106_write(unknown_cmd_3, sizeof(unknown_cmd_3))))            goto err_dump;
    if ((xRet = sh1106_write(display_style, sizeof(display_style))))            goto err_dump;
    if ((xRet = sh1106_write(display_style1, sizeof(display_style1))))          goto err_dump;
    if ((xRet = sh1106_write(setpage, sizeof(setpage))))                        goto err_dump;
    return 0;

err_dump:
    return xRet;
}

/*********************************************************************************************/
/**
*  Will be invoked if the client device is found on the desired i2c bus
*  Will only be called once
*/
static int oled_probe(struct i2c_client * client){
    int xRet = OK;

    /*  Initialize the display  */
    if ((xRet = SSH1106_display_init())){
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
/*********************************************************************************************/

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

/*********************************************************************************************/
/*  OLED Module init function   */
static int __init oled_driver_init(void){
    int xRet = OK;

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
    i2c_unregister_device(oled_i2c_client);
    i2c_del_driver(&oled_driver);
    pr_info("Driver Removed!\n");
}
/*********************************************************************************************/

module_init(oled_driver_init);
module_exit(oled_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Team6");
MODULE_DESCRIPTION("Basic Implmentation of I2C Linux Device Driver");
MODULE_VERSION("1.0");