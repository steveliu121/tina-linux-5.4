#include <linux/phy.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/string.h>

#include "driver/jl_reg_io.h"

#define BUFSIZE	100
#define VERSION "v1.0"

static int proc_debug_open(struct inode *inode, struct file *file)
{
    return 0;
}

static void print_usage(void)
{
	char head[] = "echo";
	char tail[] = "> proc/test/debug";
	printk("Usage: read/write registers\n");
	printk("Example:\n");
	printk("\twrite:%s -w -a <register address> -v <value0,vlaue1,...> -s <size> %s\n", head, tail);
	printk("\twrite:%s -w -a 0x200000 -v 0x10222345,0x2087 -s 2 %s\n", head, tail);
	printk("\tread:%s -r -a <register address> -s <size> %s\n", head, tail);
	printk("\tread:%s -r -a 0x200000 -s 2 %s\n", head, tail);
}

static ssize_t proc_debug_read(struct file* filp, char *ubuf, size_t count, loff_t* pos)
{
	print_usage();
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
			print_usage();
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
			print_usage();
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

static struct proc_dir_entry *test_dir = NULL;
static struct proc_dir_entry *debug_dir = NULL;

static struct file_operations proc_debug_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_debug_open,
    .write      = proc_debug_write,
    .read       = proc_debug_read,
};

int jl_proc_init(void)
{
	pr_info("[%s]: version %s\n", __func__, VERSION);
	test_dir = proc_mkdir("test", NULL);
    if(!test_dir) {
        return -ENOMEM;
    }

	debug_dir = proc_create("debug", 0666, test_dir, &proc_debug_fops);
	if(!debug_dir) {
		return -ENOMEM;
	}

	return 0;
}

void jl_proc_exit(void)
{
	remove_proc_entry("debug", test_dir);
	remove_proc_entry("test", NULL);
}

