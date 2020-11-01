/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

#include "data.h"
#include "entry.h"
#include "sdmessage.pb-c.h"

/* A wrapper for a Message. */
struct message_t {
  Message* msg;
};

DataMessage* data_to_msg(struct data_t* data);
struct data_t* msg_to_data(DataMessage* sg);

NullableString* string_to_msg(char* string);
char* msg_to_string(NullableString* msg);

EntryMessage* entry_to_msg(struct entry_t* entry);
struct entry_t* msg_to_entry(EntryMessage* msg);

KeysMessage* keys_to_msg(char** keys);
char** msg_to_keys (KeysMessage* msg);

#endif
