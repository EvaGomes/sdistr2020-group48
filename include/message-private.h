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

/* Aliases to entries of Message__OperationCode* */
#define OP_BAD MESSAGE__OPERATION_CODE__OP_BAD
#define OP_SIZE MESSAGE__OPERATION_CODE__OP_SIZE
#define OP_DEL MESSAGE__OPERATION_CODE__OP_DEL
#define OP_GET MESSAGE__OPERATION_CODE__OP_GET
#define OP_PUT MESSAGE__OPERATION_CODE__OP_PUT
#define OP_GETKEYS MESSAGE__OPERATION_CODE__OP_GETKEYS
#define OP_HEIGHT MESSAGE__OPERATION_CODE__OP_HEIGHT
#define OP_VERIFY MESSAGE__OPERATION_CODE__OP_VERIFY
#define OP_ERROR MESSAGE__OPERATION_CODE__OP_ERROR

/* Aliases to entries of Message__ContentCase* */
#define CT_NONE MESSAGE__CONTENT__NOT_SET
#define CT_KEY MESSAGE__CONTENT_KEY
#define CT_VALUE MESSAGE__CONTENT_VALUE
#define CT_ENTRY MESSAGE__CONTENT_ENTRY
#define CT_KEYS MESSAGE__CONTENT_KEYS
#define CT_INT_RESULT MESSAGE__CONTENT_INT_RESULT
#define CT_OP_ID MESSAGE__CONTENT_OP_ID

/* sizes */
#define SIZE_OF_DATA_MESSAGE sizeof(DataMessage)
#define SIZE_OF_NULLABLE_STRING sizeof(NullableString)
#define SIZE_OF_ENTRY_MESSAGE sizeof(EntryMessage)
#define SIZE_OF_KEYS_MESSAGE sizeof(KeysMessage)
#define SIZE_OF_MESSAGE sizeof(Message)
#define SIZE_OF_MESSAGE_T sizeof(struct message_t)

DataMessage* data_to_msg(struct data_t* data);
struct data_t* msg_to_data(DataMessage* sg);

NullableString* string_to_msg(char* string);
char* msg_to_string(NullableString* msg);

EntryMessage* entry_to_msg(struct entry_t* entry);
struct entry_t* msg_to_entry(EntryMessage* msg);

KeysMessage* keys_to_msg(char** keys);
char** msg_to_keys (KeysMessage* msg);

Message* Message_create();
struct message_t* message_create();
void message_destroy(struct message_t* message);

#endif
