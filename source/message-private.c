#include "message-private.h"
#include "data-private.h"
#include "data.h"
#include "entry.h"
#include "logger-private.h"
#include "sdmessage.pb-c.h"

#include <stdlib.h>
#include <string.h>

DataMessage* data_to_msg(struct data_t* dataStruct) {
  if (dataStruct == NULL) {
    return NULL;
  }
  DataMessage* msg = malloc(SIZE_OF_DATA_MESSAGE);
  if (msg == NULL) {
    logger_error_malloc_failed("data_to_msg");
    return NULL;
  }
  data_message__init(msg);
  msg->data.len = dataStruct->datasize;
  msg->data.data = copy(dataStruct->data, dataStruct->datasize);
  return msg;
}

struct data_t* msg_to_data(DataMessage* msg) {
  if (msg == NULL) {
    return NULL;
  }
  int datasize = msg->data.len;
  void* data = copy(msg->data.data, datasize);
  return data_create2(datasize, data);
}

NullableString* string_to_msg(char* string) {
  if (string == NULL) {
    return NULL;
  }
  NullableString* msg = malloc(SIZE_OF_NULLABLE_STRING);
  if (msg == NULL) {
    logger_error_malloc_failed("string_to_msg");
    return NULL;
  }
  nullable_string__init(msg);
  msg->str = strdup(string);
  return msg;
}

char* msg_to_string(NullableString* msg) {
  if (msg == NULL) {
    return NULL;
  }
  return strdup(msg->str);
}

EntryMessage* entry_to_msg(struct entry_t* entry) {
  if (entry == NULL) {
    return NULL;
  }
  EntryMessage* msg = malloc(SIZE_OF_ENTRY_MESSAGE);
  if (msg == NULL) {
    logger_error_malloc_failed("entry_to_msg");
    return NULL;
  }
  entry_message__init(msg);
  msg->key = string_to_msg(entry->key);
  msg->value = data_to_msg(entry->value);
  return msg;
}

struct entry_t* msg_to_entry(EntryMessage* msg) {
  if (msg == NULL) {
    return NULL;
  }
  char* key = msg_to_string(msg->key);
  struct data_t* value = msg_to_data(msg->value);
  return entry_create(key, value);
}

int _keys_count(char** keys) {
  int index = 0;
  while (keys[index] != NULL) {
    index += 1;
  }
  return index;
}

KeysMessage* keys_to_msg(char** keys) {
  if (keys == NULL) {
    return NULL;
  }
  KeysMessage* msg = malloc(SIZE_OF_KEYS_MESSAGE);
  if (msg == NULL) {
    logger_error_malloc_failed("keys_to_msg");
    return NULL;
  }
  keys_message__init(msg);
  int keys_count = _keys_count(keys);
  msg->n_keys = keys_count;
  if (keys_count == 0) {
    return msg;
  }

  msg->keys = malloc(keys_count * sizeof(char*));
  for (int i = 0; i < keys_count; ++i) {
    msg->keys[i] = strdup(keys[i]);
  }
  return msg;
}

char** msg_to_keys(KeysMessage* msg) {
  if (msg == NULL) {
    return NULL;
  }
  int n_keys = msg->n_keys;
  char** keys = malloc((n_keys + 1) * sizeof(char*));
  for (int i = 0; i < n_keys; ++i) {
    keys[i] = strdup(msg->keys[i]);
  }
  keys[n_keys] = NULL;
  return keys;
}

Message* Message_create() {
  Message* msg = malloc(SIZE_OF_MESSAGE);
  if (msg == NULL) {
    logger_error_malloc_failed("Message_create");
    return NULL;
  }
  message__init(msg);
  return msg;
}

struct message_t* message_create() {
  Message* msg = Message_create();
  if (msg == NULL) {
    return NULL;
  }

  struct message_t* message = malloc(SIZE_OF_MESSAGE_T);
  if (message == NULL) {
    logger_error_malloc_failed("message_create 2");
    free(msg);
    return NULL;
  }
  message->msg = msg;

  return message;
}

void message_destroy(struct message_t* message) {
  if (message != NULL) {
    if (message->msg != NULL) {
      message__free_unpacked(message->msg, NULL);
    }
    free(message);
  }
}