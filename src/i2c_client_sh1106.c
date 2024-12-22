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

#define SH1106_CMD                      1U
#define SH1106_DATA                     0U

#define AVAILABLE_RPI_I2C_BUS           1U
#define SSH1306_OLED_ADR                0x3C
#define CLIENT_NAME                     "i2c_client_sh1106"

/* Refer Protocol Section on PgNo. 13 of the datasheet  */
#define OLED_CNT_BYTE_CMD_SINGLE 0x80  //  Co = 1, D/C = 0, (1000 0000)b
#define OLED_CNT_BYTE_CMD_STREAM 0x00  //  Co = 0, D/C = 0, (0000 0000)b
#define OLED_CNT_BYTE_DATA_SINGLE 0xC0 //  Co = 1, D/C = 1, (1100 0000)b
#define OLED_CNT_BYTE_DATA_STREAM 0x40 //  Co = 0, D/C = 1, (0100 0000)b

#define OLED_SET_DISPLAY_OFF 0xAE     // Alawys initialize with display off
#define OLED_SET_DISPLAY_CLK_DIV 0xD5 // DB CMD: Follow with 0x80
#define OLED_SET_MULTIPLEX_RATIO 0xA8 // follow with 0x3F for 128x64 bit display
#define OLED_SET_DISPLAY_OFFSET 0xD3  // DB CMD: Follow with 0x00
#define OLED_SET_DISPLAY_START_L 0x40 // Set the RAM Display line addr for COM0

#define OLED_SET_SEG_REMAP_L 0xA0 // SEG0 = COL0 == 00H
#define OLED_SET_SEG_REMAP_H 0xA1 // SEG0 = COL0 == 83H

#define OLED_SET_COM_OP_SCAN 0xC0   // Do not flip vertically, refer pg. 24
#define OLED_SET_COM_OP_SCAN_V 0xC8 // Flip vertically.

#define OLED_SET_COM_PAD_HW 0xDA // DB CMD: Follow with 0x12

#define OLED_SET_CONTRAST 0x81 // Follow with 0x80 (default), for more info refer pg 20.

#define OLED_SET_PRECHARGE 0xD9 // DB CMD: Follow with 0x22

#define OLED_SET_VCOM_LEVEL 0xDB // DB CMD: Follow with 0x35

#define OLED_SET_ENT_DISPLAY_RAM 0xA4 // Normal display status
#define OLED_SET_DISPLAY_NORMAL 0xA6

#define OLED_SET_PAGE_ADR 0xB0 // B0 -> Page 1..... B7 -> Page 2

#define OLED_SET_DISPLAY_ON 0xAF // Initialize DC-DC then use this command to turn on the display

/*  display initialization commands */
uint8_t display_off_cmd[] = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_DISPLAY_OFF};
uint8_t clk_div_cmd[] = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_DISPLAY_CLK_DIV, 0x80};
uint8_t multiplex_ratio[] = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_MULTIPLEX_RATIO, 0x3F};
uint8_t display_offset[] = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_DISPLAY_OFFSET, 0x00};
uint8_t display_start_line[] = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_DISPLAY_START_L};
uint8_t unknown_cmd_1[] = {OLED_CNT_BYTE_CMD_STREAM, 0x8D, 0x14};
uint8_t unknown_cmd_2[] = {OLED_CNT_BYTE_CMD_STREAM, 0x20, 0x00};
uint8_t segment_remap[] = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_SEG_REMAP_H};
uint8_t scan_dir[] = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_COM_OP_SCAN_V};
uint8_t com_hw_pad[] = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_COM_PAD_HW, 0x12};
uint8_t contrast[] = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_CONTRAST, 0xCF};
uint8_t precharge[] = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_PRECHARGE, 0xF1};
uint8_t vcom[] = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_VCOM_LEVEL, 0x40};
uint8_t unknown_cmd_3[] = {OLED_CNT_BYTE_CMD_STREAM, 0x2E};
uint8_t display_style[] = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_ENT_DISPLAY_RAM};
uint8_t display_style1[] = {OLED_CNT_BYTE_CMD_STREAM, OLED_SET_DISPLAY_NORMAL};
uint8_t setpage[] = {OLED_CNT_BYTE_CMD_STREAM, 0x10, 0x2, OLED_SET_PAGE_ADR};

/*  declare and define a adapter structure to which the device will be connected on */
static struct i2c_adapter *oled_i2c_adap = NULL;
static struct i2c_client *oled_i2c_client = NULL;

/*  */
static int sh1106_write(const char *buf){
    int xRet = -1;

    if((xRet = i2c_master_send(oled_i2c_client, buf, sizeof(buf)))){
        return xRet;
    }
    return 0;
}

/**
 *  SSH1106 Display initialization commands and routines, also compatible with SSD1306
 *  Refer the SSH1106 and use the initialization codes to initialize the display.
 *  
 *  STATUS = Pending
 */
static int SSH1106_display_init(void){
    int xRet = -1;

    if((xRet = sh1106_write(display_off_cmd)))
        goto err_dump;
    if ((xRet = sh1106_write(clk_div_cmd)))
        goto err_dump;
    if ((xRet = sh1106_write(multiplex_ratio)))
        goto err_dump;
    if ((xRet = sh1106_write(display_offset)))
        goto err_dump;
    if ((xRet = sh1106_write(display_start_line)))
        goto err_dump;
    if ((xRet = sh1106_write(unknown_cmd_1)))
        goto err_dump;
    if ((xRet = sh1106_write(unknown_cmd_2)))
        goto err_dump;
    if ((xRet = sh1106_write(segment_remap)))
        goto err_dump;
    if ((xRet = sh1106_write(scan_dir)))
        goto err_dump;
    if ((xRet = sh1106_write(com_hw_pad)))
        goto err_dump;
    if ((xRet = sh1106_write(contrast)))
        goto err_dump;
    if ((xRet = sh1106_write(precharge)))
        goto err_dump;
    if ((xRet = sh1106_write(vcom)))
        goto err_dump;
    if ((xRet = sh1106_write(unknown_cmd_3)))
        goto err_dump;
    if ((xRet = sh1106_write(display_style)))
        goto err_dump;
    if ((xRet = sh1106_write(display_style1)))
        goto err_dump;
    if ((xRet = sh1106_write(setpage)))
        goto err_dump;

    return 0;

    err_dump:
        return -1;
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