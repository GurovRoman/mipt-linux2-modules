#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include "phonebook.h"

MODULE_LICENSE("GPL");

static int major_num;
static size_t device_open_count;

static struct class* pClass;
static dev_t dev;


static char msg_buffer[MAX_BUFFER_SIZE];
static char* msg_ptr;

static LIST_HEAD(phonebook_list);


static ssize_t device_read(struct file* flip, char* buffer, size_t len, loff_t* offset) {
	int bytes_read = 0;

	while (len && *msg_ptr) {
		put_user(*(msg_ptr++), buffer++);
		--len;
		++bytes_read;
	}
	return bytes_read;
}


struct user_data_node* find_user_by_last_name(char* last_name, struct list_head* list) {
	struct user_data_node* cur;
	list_for_each_entry(cur, list, list) {
		if (!strcmp(last_name, cur->last_name)) {
			break;
		}
	}
	if (&cur->list == list) {
		return NULL;
	}
	return cur;
}


void user_data_to_string(char* str, struct user_data_node* udata) {
	strcpy(str, udata->last_name);
	strcat(str, " ");
	strcat(str, udata->first_name);
	strcat(str, " ");
	strcat(str, udata->age);
	strcat(str, " ");
	strcat(str, udata->phone_number);
	strcat(str, " ");
	strcat(str, udata->email);
}


void read_token_from_user(const char** user_str, size_t* str_len, char* buf, size_t buf_size) {
	if (!buf_size) {
		return;
	}
	--buf_size;    // leave a byte for null

	char chr;
	while (*str_len && buf_size && get_user(chr, (*user_str)++) != -EFAULT) {
		--*str_len;
		--buf_size;
		if (chr == ' ') {
			break;
		}
		*(buf++) = chr;
	}
	*buf = '\0';
}


static ssize_t device_write(struct file* flip, const char* buffer, size_t len, loff_t* offset) {
	char opcode;
	size_t exit_code = len;

	if (!len || get_user(opcode, buffer++) == -EFAULT) {
		return -EINVAL;
	}
	len -= 2;
	++buffer;      // skip the space after opcode

	switch (opcode) {
		case 'd': {
			char last_name[MAX_FIELD_SIZE];
			read_token_from_user(&buffer, &len, last_name, MAX_FIELD_SIZE);

			struct user_data_node* udata = find_user_by_last_name(last_name, &phonebook_list);

			if (!udata) {
				return -ENOENT;
			}

			list_del(&udata->list);
			kfree(udata);
			break;
		}
		case 'g': {
			char last_name[MAX_FIELD_SIZE];
			read_token_from_user(&buffer, &len, last_name, MAX_FIELD_SIZE);

			struct user_data_node* udata = find_user_by_last_name(last_name, &phonebook_list);
			
			if (!udata) {
				return -ENOENT;
			}

			user_data_to_string(msg_buffer, udata);
			msg_ptr = msg_buffer;
			break;
		}
		case 'a': {
			struct user_data_node* udata = kmalloc(sizeof(*udata), GFP_KERNEL);
			read_token_from_user(&buffer, &len, udata->last_name, MAX_FIELD_SIZE);
			read_token_from_user(&buffer, &len, udata->first_name, MAX_FIELD_SIZE);
			read_token_from_user(&buffer, &len, udata->age, MAX_FIELD_SIZE);
			read_token_from_user(&buffer, &len, udata->phone_number, MAX_FIELD_SIZE);
			read_token_from_user(&buffer, &len, udata->email, MAX_FIELD_SIZE);
			list_add(&udata->list, &phonebook_list);
			break;
		}
		default:
			return -EINVAL;
	}

	return exit_code;
}


static int device_open(struct inode* inode, struct file* file) {
	if (device_open_count) {
		return -EBUSY;
	}
	device_open_count++;
	return 0;
}


static int device_release(struct inode* inode, struct file* file) {
	device_open_count--;
	return 0;
}


struct file_operations file_ops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,
	.owner = THIS_MODULE
};


static char *class_devnode(struct device *dev, umode_t *mode) {
	if (!mode) {
		return NULL;
	}
	*mode = 0666;
	return NULL;
}


static int __init phonebook_init(void) {
	printk(KERN_INFO "phonebook: init\n");

	msg_ptr = msg_buffer;

	major_num = register_chrdev(0, "phonebook", &file_ops);
	if (major_num < 0) {
		printk(KERN_ALERT "phonebook: could not register device: %d\n", major_num);
		return major_num;
	}

	dev = MKDEV(major_num, 0);

	pClass = class_create(THIS_MODULE, "phonebook");
	if (IS_ERR(pClass)) {
		printk(KERN_ALERT "phonebook: unable to create device class\n");
		unregister_chrdev(major_num, "phonebook");
		return PTR_ERR(pClass);
	}

	pClass->devnode = class_devnode;

	struct device* dev_err = device_create(pClass, NULL, dev, NULL, "phonebook");

	if (IS_ERR(dev_err)) {
		printk(KERN_ALERT "phonebook: unable to create character device file\n");
		class_destroy(pClass);
		unregister_chrdev(major_num, "phonebook");
		return PTR_ERR(dev_err);
	}

	printk(KERN_INFO "phonebook: registered phonebook device with major number %d\n", major_num);

	return 0;
}


static void __exit phonebook_exit(void) {
	device_destroy(pClass, dev);
	class_destroy(pClass);
	unregister_chrdev(major_num, "phonebook");
	
	struct user_data_node* cur;
	list_for_each_entry(cur, &phonebook_list, list) {
		kfree(cur);
	}

	printk(KERN_INFO "phonebook: exited\n");
}


module_init(phonebook_init);
module_exit(phonebook_exit);