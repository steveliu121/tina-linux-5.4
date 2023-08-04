/*
 * Copyright (c) 2014-2023 JLSemi Limited
 * All Rights Reserved
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE of JLSemi Limited
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code.
 *
 * No part of this code may be reproduced, stored in a retrieval system,
 * or transmitted, in any form or by any means, electronic, mechanical,
 * photocopying, recording, or otherwise, without the prior written
 * permission of JLSemi Limited
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/acpi.h>
#include <linux/spi/spi.h>

#include "driver/jl_reg_io.h"
#include "jl_base.h"
#ifdef CONFIG_JL51XX
//#include "port.h"
#else
#include "jl_switch.h"
//#include "jl_port.h"
#endif

#define DRIVERNAME "spi-jlswitch"
#define DEVICENAME "jlspi"
#define DEVICECLASS "jlspidev"
#define MAJORNO 154
#define MINORNO 0

static struct class *jldev_class;
struct spi_device	*spi_dev = NULL;

enum jl_chip_variant {
	jl51xx,
	jl61xx,
	max_variant
};

struct jl_spi_switch {
	dev_t		devt;
	struct spi_device	*spi;
	spinlock_t	lock;
	struct bin_attribute	regs_attr;
	jl_dev_t	device;
	int			revision_id;
};

static const struct spi_device_id jl_switch_id[] = {
	{"jl51xx", jl51xx},
	{"jl61xx", jl61xx},
	{ }
};
MODULE_DEVICE_TABLE(spi, jl_switch_id);

static const struct of_device_id jl_spi_of_match[] = {
        { .compatible = "jlsemi,jl51xx" },
        { .compatible = "jlsemi,jl61xx" },
        { },
 };
MODULE_DEVICE_TABLE(of, jl_spi_of_match);


static int jl_spi_probe(struct spi_device *spi)
{
	struct jl_spi_switch *jldev;
	struct device *dev;
	int err;
	int variant = spi_get_device_id(spi)->driver_data;

	if (variant >= max_variant) {
		dev_err(&spi->dev, "bad chip variant %d\n", variant);
		return -ENODEV;
	}

	jldev = devm_kzalloc(&spi->dev, sizeof(*jldev), GFP_KERNEL);
	if (!jldev)
		return -ENOMEM;

	spin_lock_init(&jldev->lock);
	jldev->spi = spi;
	spi_dev = spi;
	
	jldev->devt = MKDEV(MAJORNO, MINORNO);
	dev = device_create(jldev_class, &spi->dev, jldev->devt,
				jldev, DEVICENAME);
	if(dev == NULL)	{
		dev_err(&spi->dev, "device creation failed\n");
		kfree(jldev);
		return -1;
	}

	spi_set_drvdata(spi, jldev);

	spi->mode = SPI_MODE_0;
	spi->bits_per_word = 8;
	err = spi_setup(spi);
	if (err) {
		dev_err(&spi->dev, "spi_setup failed, err=%d\n", err);
		kfree(jldev);
		return err;
	}

	dev_info(&spi->dev, "%d device found\n", jldev->device.compat_id);

	return 0;
}

static int jl_spi_remove(struct spi_device *spi)
{
	struct jl_spi_switch *jldev = spi_get_drvdata(spi);

	spin_lock_irq(&jldev->lock);
	jldev->spi = NULL;
	spin_unlock_irq(&jldev->lock);

	return 0;
}

static struct spi_driver jl_spi_driver = {
	.driver = {
		.name	    = DRIVERNAME,
		.of_match_table = of_match_ptr(jl_spi_of_match),
	},
	.probe	  = jl_spi_probe,
	.remove	  = jl_spi_remove,
	.id_table = jl_switch_id,
};


/* ------------------------------------------------------------------------ */

int jl_spi_dev_write(const void * buf, size_t len)
{
	return spi_write(spi_dev, buf, len);
}

int jl_spi_dev_read(const void * txbuf, unsigned n_tx, void * rxbuf, unsigned n_rx)
{
	return spi_write_then_read(spi_dev, txbuf, n_tx, rxbuf, n_rx);
}

static jl_chip_name_t compat_id[] = {JL_CHIP_6108, JL_CHIP_6110, JL_CHIP_6105,
									 JL_CHIP_6107, JL_CHIP_6107S,JL_CHIP_6107SC};
static int jl_dev_open(struct inode *inode, struct file *filp)
{
	int i = 0;
	jl_api_ret_t ret = 0;
	jl_uint32 chip_id = 0;
	jl_dev_t dev_61xx = {
		.compat_id = JL_CHIP_6110,
		.name = "device-jl61xx",
		.id = chip_id, /* must be less than JL_MAX_CHIP_NUM */
		.io = {
			.type = JL_IO_SMI,
			.smi.mdio.bus_id = 0
		}
	};

	pr_info("[%s][%d]\n", __func__, __LINE__);

	ret = jl_switch_init();
	if (ret)
		goto exit;

	for (i = 0; i < (int)(sizeof(compat_id)/sizeof(jl_chip_name_t)); i++) {
		dev_61xx.compat_id = compat_id[i];
		ret = jl_switch_device_create(&dev_61xx);
		if (ret == JL_ERR_OK) {
			pr_info("[%s]: jlsemi device compat_id = 0x%x!\n", __func__, compat_id[i]);
			break;
		}
	}

exit:
	if (ret == JL_ERR_OK) {
		pr_info("[%s]: jlsemi device create success\n", __func__);
	} else {
		jl_switch_deinit();
		pr_err("[%s]: jlsemi device create failed\n", __func__);
	}
	
	return ret;
}

static int jl_dev_release(struct inode *inode, struct file *filp)
{
	jl_api_ret_t ret = 0;

	pr_info("[%s][%d]\n", __func__, __LINE__);
	ret = jl_switch_deinit();
	if (ret) {
		pr_err("[%s]: jlsemi switch deinit failed!\n", __func__);
	}

	return ret;
}

static const struct file_operations jl_dev_fops = {
	.owner =	THIS_MODULE,
	/* REVISIT switch to aio primitives, so that userspace
	 * gets more complete API coverage.  It'll simplify things
	 * too, except for the locking.
	 */
//	.write =	spidev_write,
//	.read =		spidev_read,
//	.unlocked_ioctl = spidev_ioctl,
	.open =		jl_dev_open,
	.release =	jl_dev_release,
};

static int __init jl_spi_init(void)
{
	int status;

	status = register_chrdev(MAJORNO, DEVICENAME, &jl_dev_fops);
	if (status < 0)
		return status;

	jldev_class = class_create(THIS_MODULE, DEVICECLASS);
	if (IS_ERR(jldev_class)) {
		unregister_chrdev(MAJORNO, jl_spi_driver.driver.name);
		return PTR_ERR(jldev_class);
	}

	status = spi_register_driver(&jl_spi_driver);
	if (status < 0) {
		class_destroy(jldev_class);
		unregister_chrdev(MAJORNO, jl_spi_driver.driver.name);
	}
	return status;
}
module_init(jl_spi_init);

static void __exit jl_spi_exit(void)
{
	spi_unregister_driver(&jl_spi_driver);
	class_destroy(jldev_class);
	unregister_chrdev(MAJORNO, jl_spi_driver.driver.name);
}
module_exit(jl_spi_exit);


MODULE_LICENSE("GPL v2");
