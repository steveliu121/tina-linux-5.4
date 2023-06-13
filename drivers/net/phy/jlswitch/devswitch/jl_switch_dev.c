#include <linux/phy.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/netdevice.h>

#include "driver/jl_reg_io.h"

#include "jl_base.h"
#include "port.h"

#define BUFSIZE	100
#define VERSION "v1.0"

#define PROC_DIR	"jlmdio"
#define PROC_FILE	"debug"
#define PROC_PATH	"> /proc/"PROC_DIR"/"PROC_FILE

static struct proc_dir_entry *proc_dir = NULL;
static struct proc_dir_entry *proc_file = NULL;

static void proc_print_usage(void)
{
	printk("Usage: read/write registers\n");
	printk("Example:\n");
	printk("\twrite:echo -w -a <register address> -v <value0,vlaue1,...> -s <size> %s\n", PROC_PATH);
	printk("\twrite:echo -w -a 0x200000 -v 0x10222345,0x2087 -s 2 %s\n", PROC_PATH);
	printk("\tread:echo -r -a <register address> -s <size> %s\n", PROC_PATH);
	printk("\tread:echo -r -a 0x200000 -s 2 %s\n", PROC_PATH);
}

static ssize_t proc_debug_read(struct file* filp, char *ubuf, size_t count, loff_t* pos)
{
	proc_print_usage();
	return 0;
}

static int proc_parse_cmd(char *str)
{
	char *delim = " ";
	char *token = NULL;
	char value_str[256] = {0};
	char *str0 = NULL;
	char *str1 = NULL;
	char value[16] = {0};
	uint32_t buf[8] = {0};
	uint32_t register_addr = 0;
	uint8_t size = 1;
	/* 0:read, 1:write */
	uint8_t rw = 0;
	int i = 0;
	jl_api_ret_t ret;
	unsigned long val = 0;

	while(token = strsep(&str, delim)) {
		if(token[0] != '-' || token[1] == 0) {
			printk("invalid param\n");
			proc_print_usage();
			return -1;
		}
		switch(token[1]) {
		case 'a':
			token = strsep(&str, delim);
			kstrtoul(token, 16, &val);
			register_addr = val;
			break;
		case 'v':
			token = strsep(&str, delim);
			strcpy(value_str, token);
			break;
		case 's':
			token = strsep(&str, delim);
			kstrtoul(token, 10, &val);
			size = val;
			break;
		case 'r':
			rw = 0;
			break;
		case 'w':
			rw = 1;
			break;
		case 'h':
		default:
			proc_print_usage();
			return 0;
		}
	}

	ret = jl_reg_io_init();
	if (ret) {
		printk("io init fail\n");
		return -1;
	}

	if (rw) {
		/* write */
		str0 = value_str;
		str1 = strstr(str0, ",");
		i = 0;
	
		if (str1 == NULL) {
			kstrtoul(str0, 16, &val);
			buf[i] = val;
		}

		while (str1 != NULL) {
			memset(value, 0, 16);
			memcpy(value, str0, str1 - str0);
			kstrtoul(value, 16, &val);
			buf[i] = val;
			printk("0x%08x\t", buf[i]);

			str0 = str1 + 1;
			str1 = strstr(str0, ",");
			i++;

			if (str1 == NULL) {
				kstrtoul(str0, 16, &val);
				buf[i] = val;
				printk("0x%08x\t", buf[i]);
			}
		}

		ret = jl_apb_reg_burst_write(register_addr, &buf[0], size);
		if (ret) {
			printk("####error[%d]int func[%s] line[%d]\n", ret, __func__, __LINE__);
			goto exit;
		}
	} else {
		/* read */
		ret = jl_apb_reg_burst_read(register_addr, &buf[0], size);
		if (ret) {
			printk("####error[%d]int func[%s] line[%d]\n", ret, __func__, __LINE__);
			goto exit;
		}

		printk("read register[0x%08x]\n", register_addr);
		for (i = 0; i < size; i++)
			printk("\t0x%08x", buf[i]);
	}

exit:
	printk("\n");
	jl_reg_io_deinit();

	return 0;
}

static ssize_t proc_debug_write(struct file* filp, const char *ubuf, size_t count, loff_t* pos)
{
	char buf[BUFSIZE] = {0};

	if(*pos > 0 || count > BUFSIZE)
		return -EFAULT;
	if(copy_from_user(buf, ubuf, count))
		return -EFAULT;
	proc_parse_cmd(buf);

	return count;
}

static struct file_operations proc_debug_fops = {
    .owner      = THIS_MODULE,
    .write      = proc_debug_write,
    .read       = proc_debug_read,
};

static int jl_proc_init(void)
{
	pr_info("[%s]: version %s\n", __func__, VERSION);
	proc_dir = proc_mkdir(PROC_DIR, NULL);
    if(!proc_dir) {
		pr_err("[%s]: proc_mkdir %s failed!\n", __func__, PROC_DIR);
        return -ENOMEM;
    }

	proc_file = proc_create(PROC_FILE, 0666, proc_dir, &proc_debug_fops);
	if(!proc_file) {
		pr_err("[%s]: proc_create %s failed!\n", __func__, PROC_FILE);
		return -ENOMEM;
	}

	return JL_ERR_OK;
}

static void jl_proc_exit(void)
{
	remove_proc_entry(PROC_FILE, proc_dir);
	remove_proc_entry(PROC_DIR, NULL);
}

/**
 * jl_switch_open - initialize jlsemi switch mac.
 * @ndev: net device.
 * @duplex: duplex mode, 0: half-duplex, 1: full-duplex.
 * @speed: mac working speed, eg: 10, 100, 1000.
 *
 * return 0 if success, otherwise failed.
 */
int jl_switch_open(struct net_device *ndev, int duplex, int speed)
{
	jl_port_ext_mac_ability_t ability;
	jl_port_mac_ability_t status;
	jl_api_ret_t ret = 0;

	ret = jl_proc_init();
	if (ret) {
		pr_err("[%s]: jlsemi proc init failed!\n", __func__);
		return ret;
	}

	ret = jl_switch_init();
	if (ret) {
		pr_err("[%s]: jlsemi switch init failed!\n", __func__);
		return ret;
	}

	/* Force the MAC of EXT_PORT0 working with 100F */
	/* and Symmetric PAUSE flow control abilities */
	memset(&ability, 0x00, sizeof(jl_port_ext_mac_ability_t));
	//ability.force_mode = 1;
	if (speed == 100)
		ability.speed = PORT_SPEED_100M;
//	else if (speed == 1000)
//		ability.speed = PORT_SPEED_1000M;
	else
		ability.speed = PORT_SPEED_10M;
	ability.duplex = duplex;
	ability.link = PORT_LINKUP;
	ability.tx_pause = 1;
	ability.rx_pause = 1;
	ret = jl_port_mac_force_link_ext_set(EXT_PORT0, &ability);
	if (ret) {
		pr_err("[%s]: jl_port_mac_force_link_ext_set EXT_PORT0 error[%d]\n", __func__, ret);
		return ret;
	}

	/* Get MAC link status of EXT_PORT0 */
	memset(&status, 0x00, sizeof(jl_port_mac_ability_t));
	ret = jl_port_mac_status_get(EXT_PORT0, &status);
	if (ret) {
		pr_err("[%s]: jl_port_mac_status_get EXT_PORT0 error[%d]\n", __func__, ret);
		netif_carrier_off(ndev);
	} else if (status.link == 1) {
		pr_info("[%s]: link Up\n", __func__);
		netif_carrier_on(ndev);
	} else {
		pr_info("[%s]: link Down\n", __func__);
		netif_carrier_off(ndev);
	}

	return JL_ERR_OK;
}

int jl_switch_stop(void)
{
	jl_api_ret_t ret = 0;

	jl_proc_exit();

	ret = jl_switch_deinit();
	if (ret) {
		pr_err("[%s]: jlsemi switch deinit failed!\n", __func__);
		return ret;
	}

	return JL_ERR_OK;
}
