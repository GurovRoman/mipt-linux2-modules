#ifndef PHONEBOOK_H
#define PHONEBOOK_H

#define MAX_FIELD_SIZE 20
#define FIELD_COUNT 5
#define MAX_BUFFER_SIZE ((MAX_FIELD_SIZE + 1) * FIELD_COUNT)

struct user_data {
	char last_name[MAX_FIELD_SIZE];
	char first_name[MAX_FIELD_SIZE];
	char age[MAX_FIELD_SIZE];
	char phone_number[MAX_FIELD_SIZE];
	char email[MAX_FIELD_SIZE];
};

#endif