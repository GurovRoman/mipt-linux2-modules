#include <linux/phonebook.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>

static const char* device_file = "/dev/phonebook";


void user_data_to_string(char* str, struct user_data* udata) {
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


void read_token_from_str(const char** str, size_t* str_len, char* buf, size_t buf_size) {
	if (!buf_size) {
		return;
	}
	--buf_size;    // leave a byte for null

	char chr;
	while (*str_len && buf_size) {
        chr = **str;
        ++*str;
		--*str_len;
		--buf_size;
		if (chr == ' ') {
			break;
		}
		*(buf++) = chr;
	}
	*buf = '\0';
}


SYSCALL_DEFINE3(get_user,
                const char*, surname,
                unsigned int, len,
                struct user_data*, output_data)
{
    struct file* file = filp_open(device_file, O_RDWR, 0);
    if (IS_ERR(file)) {
        return -ENODEV;
    }

    char buf[MAX_BUFFER_SIZE + 2];
    strcpy(buf, "g ");
    if (len >= MAX_FIELD_SIZE || copy_from_user(buf + 2, surname, len)) {
        goto error;
    }
    
    loff_t offset = 0;

    kernel_write(file, buf, strlen(buf), &offset);

    offset = 0;
    struct user_data response = {};

    if (kernel_read(file, buf, MAX_BUFFER_SIZE, &offset) != 0) {
        size_t len;
        const char* buf_iter = buf;
        read_token_from_str(&buf_iter, &len, response.last_name, MAX_FIELD_SIZE);
        read_token_from_str(&buf_iter, &len, response.first_name, MAX_FIELD_SIZE);
        read_token_from_str(&buf_iter, &len, response.age, MAX_FIELD_SIZE);
        read_token_from_str(&buf_iter, &len, response.phone_number, MAX_FIELD_SIZE);
        read_token_from_str(&buf_iter, &len, response.email, MAX_FIELD_SIZE);
    }

    if (copy_to_user(output_data, &response, sizeof(struct user_data))) {
        goto error;
    }

    filp_close(file, NULL);
    return 0;

error:
    filp_close(file, NULL);
    return -EFAULT;
}


SYSCALL_DEFINE1(add_user,
                struct user_data*, input_data)
{
    struct file* file = filp_open(device_file, O_RDWR, 0);
    if (IS_ERR(file)) {
        return -ENODEV;
    }

    char buf[MAX_BUFFER_SIZE + 2];
    struct user_data udata;

    if (copy_from_user(&udata, input_data, sizeof(udata))) {
        filp_close(file, NULL);
        return -EFAULT;
    }

    strcpy(buf, "a ");

    user_data_to_string(buf + 2, &udata);

    loff_t offset = 0;

    kernel_write(file, buf, strlen(buf), &offset);


    filp_close(file, NULL);
    return 0;
}


SYSCALL_DEFINE2(del_user,
                const char*, surname,
                unsigned int, len)
{
    struct file* file = filp_open(device_file, O_RDWR, 0);
    if (IS_ERR(file)) {
        return -ENODEV;
    }

    char buf[MAX_FIELD_SIZE + 2];

    strcpy(buf, "d ");

    if (len >= MAX_FIELD_SIZE || copy_from_user(buf + 2, surname, len)) {
        filp_close(file, NULL);
        return -EFAULT;
    }
    
    loff_t offset = 0;

    kernel_write(file, buf, strlen(buf), &offset);
    
    filp_close(file, NULL);
    return 0;
}
