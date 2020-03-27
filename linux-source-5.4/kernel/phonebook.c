#include <linux/phonebook.h>
#include <linux/syscalls.h>

int phonebook_major_num;
EXPORT_SYMBOL(phonebook_major_num);


SYSCALL_DEFINE3(get_user,
                const char*, surname,
                unsigned int, len,
                struct user_data*, output_data)
{
    if (phonebook_major_num <= 0)
        return -ENODEV;
    return -EINVAL;
}


SYSCALL_DEFINE1(add_user,
                struct user_data*, input_data)
{
    if (phonebook_major_num <= 0)
        return -ENODEV;
    return -EINVAL;
}


SYSCALL_DEFINE2(del_user,
                const char*, surname,
                unsigned int, len)
{
    if (phonebook_major_num <= 0)
        return -ENODEV;
    return -EINVAL;
}
