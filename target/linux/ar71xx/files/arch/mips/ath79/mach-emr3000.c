/*
 *  EnGenius EMR3000 board support
 *
 *  Copyright (c) 2014 Jon Suphammer <jon@suphammer.net>
 *  Copyright (c) 2017 Christian Beier <cb@shoutrlabs.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "pci.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-eth.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "nvram.h"

#define EMR3000_GPIO_LED_RED		11
#define EMR3000_GPIO_LED_ORANGE 	16
#define EMR3000_GPIO_LED_WHITE	        18
#define EMR3000_GPIO_LED_BLUE	        19

#define EMR3000_GPIO_BTN_WPS		17
#define EMR3000_GPIO_BTN_RESET		20

#define EMR3000_KEYS_POLL_INTERVAL	20	/* msecs */
#define EMR3000_KEYS_DEBOUNCE_INTERVAL	(3 * EMR3000_KEYS_POLL_INTERVAL)

#define EMR3000_ART_ADDR                0x1f050000
#define EMR3000_MAC0_OFFSET             0x0
#define EMR3000_MAC1_OFFSET             0x6
#define EMR3000_WMAC_CALDATA_OFFSET     0x1000

//FIXME
#define EMR3000_NVRAM_ADDR	0x1f030000
#define EMR3000_NVRAM_SIZE	0x10000

static struct gpio_led emr3000_leds_gpio[] __initdata = {
	{
		.name		= "emr3000:red:front",
		.gpio		= EMR3000_GPIO_LED_RED,
		.active_low	= 0,
	},
	{
		.name		= "emr3000:orange:front",
		.gpio		= EMR3000_GPIO_LED_ORANGE,
		.active_low	= 0,
	},
	{
		.name		= "emr3000:white:front",
		.gpio		= EMR3000_GPIO_LED_WHITE,
		.active_low	= 0,
	},
	{
		.name		= "emr3000:blue:front",
		.gpio		= EMR3000_GPIO_LED_BLUE,
		.active_low	= 0,
	}
};

static struct gpio_keys_button emr3000_gpio_keys[] __initdata = {
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = EMR3000_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= EMR3000_GPIO_BTN_WPS,
		.active_low	= 1,
	},
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = EMR3000_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= EMR3000_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

//FIXME
/*
static struct ar8327_pad_cfg emr3000_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL2,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_platform_data emr3000_ar8327_data = {
	.pad0_cfg = &emr3000_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info emr3000_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &emr3000_ar8327_data,
	},
};
*/

/*
static int emr3000_get_mac(const char *name, char *mac)
{
	u8 *nvram = (u8 *) KSEG1ADDR(EMR3000_NVRAM_ADDR);
	int err;

	err = ath79_nvram_parse_mac_addr(nvram, EMR3000_NVRAM_SIZE,
					 name, mac);
	if (err) {
		pr_err("no MAC address found for %s\n", name);
		return false;
	}

	return true;
}
*/

static void __init emr3000_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(EMR3000_ART_ADDR);
	//u8 mac1[ETH_ALEN];

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(emr3000_leds_gpio),
					emr3000_leds_gpio);
	ath79_register_gpio_keys_polled(-1, EMR3000_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(emr3000_gpio_keys),
					emr3000_gpio_keys);

	ath79_register_usb();

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	ath79_register_mdio(0, 0x0);

	//FIXME
	//mdiobus_register_board_info(emr3000_mdio0_info,
	//				ARRAY_SIZE(emr3000_mdio0_info));


	
	/* GMAC0 is connected to an external PHY: AR8035 */
	ath79_init_mac(ath79_eth0_data.mac_addr, art + EMR3000_MAC0_OFFSET, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(1);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x9a000000;
	ath79_eth0_pll_data.pll_100  = 0x80000101;
	ath79_eth0_pll_data.pll_10   = 0x80001313;

	ath79_register_eth(0);

	/* GMAC1 is connected to an external PHY: AR8033 */
	ath79_init_mac(ath79_eth1_data.mac_addr, art + EMR3000_MAC1_OFFSET, 0);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.phy_mask = BIT(2);
	ath79_eth1_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth1_pll_data.pll_1000 = 0x9b000000;
	ath79_eth1_pll_data.pll_100  = 0x80000101;
	ath79_eth1_pll_data.pll_10   = 0x80001313;

	ath79_register_eth(1);

	//FIXME
	/* GMAC0 is connected to an QCA8327N switch */
	/*
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;

	if (emr3000_get_mac("ethaddr=", mac1))
		ath79_init_mac(ath79_eth0_data.mac_addr, mac1, 0);

	ath79_eth0_pll_data.pll_1000 = 0xa6000000;
	ath79_register_eth(0);
	*/
	

	ath79_register_wmac(art + EMR3000_WMAC_CALDATA_OFFSET, NULL);

	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_EMR3000, "EMR3000",
	     "EnGenius EMR3000",
	     emr3000_setup);
