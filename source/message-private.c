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

DataMessage* _DataMessage_dup(DataMessage* msg) {
  if (msg == NULL) {
    return NULL;
  }
  DataMessage* copied_msg = malloc(SIZE_OF_DATA_MESSAGE);
  if (copied_msg == NULL) {
    logger_error_malloc_failed("_DataMessage_dup");
    return NULL;
  }
  data_message__init(copied_msg);
  copied_msg->data.len = msg->data.len;
  copied_msg->data.data = copy(msg->data.data, msg->data.len);
  return copied_msg;
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

EntryMessage* _EntryMessage_dup(EntryMessage* msg) {
  if (msg == NULL) {
    return NULL;
  }
  EntryMessage* copied_msg = malloc(SIZE_OF_ENTRY_MESSAGE);
  if (copied_msg == NULL) {
    logger_error_malloc_failed("_EntryMessage_dup");
    return NULL;
  }
  entry_message__init(copied_msg);
  copied_msg->key = (msg->key == NULL) ? NULL : string_to_msg(msg->key->str);
  copied_msg->value = _DataMessage_dup(msg->value);
  return copied_msg;
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

char** _str_arr_copy(char** str_arr, int arr_len) {
  char** copied_arr = malloc(arr_len * SIZE_OF_CHAR_POINTER);
  if (copied_arr == NULL) {
    logger_error_malloc_failed("_str_arr_copy");
    return NULL;
  }
  for (int i = 0; i < arr_len; ++i) {
    copied_arr[i] = strdup(str_arr[i]);
  }
  return copied_arr;
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

  msg->n_keys = _keys_count(keys);
  if (msg->n_keys != 0) {
    msg->keys = _str_arr_copy(keys, msg->n_keys);
  }

  return msg;
}

KeysMessage* _KeysMessage_dup(KeysMessage* msg) {
  if (msg == NULL) {
    return NULL;
  }
  KeysMessage* copied_msg = malloc(SIZE_OF_KEYS_MESSAGE);
  if (copied_msg == NULL) {
    logger_error_malloc_failed("keys_to_msg");
    return NULL;
  }
  keys_message__init(copied_msg);

  copied_msg->n_keys = msg->n_keys;
  if (msg->n_keys != 0) {
    copied_msg->keys = _str_arr_copy(msg->keys, msg->n_keys);
  }

  return copied_msg;
}

char** msg_to_keys(KeysMessage* msg) {
  if (msg == NULL) {
    return NULL;
  }
  int n_keys = msg->n_keys;
  char** keys = malloc((n_keys + 1) * SIZE_OF_CHAR_POINTER);
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

Message* Message_dup(Message* msg) {
  Message* copied_msg = Message_create();
  if (copied_msg == NULL) {
    return NULL;
  }
  copied_msg->op_code = msg->op_code;
  copied_msg->content_case = msg->content_case;
  switch (msg->content_case) {
    case MESSAGE__CONTENT__NOT_SET:
      break;

    case MESSAGE__CONTENT_KEY:
      copied_msg->key = strdup(msg->key);
      break;

    case MESSAGE__CONTENT_VALUE:
      copied_msg->value = _DataMessage_dup(msg->value);
      break;

    case MESSAGE__CONTENT_ENTRY:
      copied_msg->entry = _EntryMessage_dup(msg->entry);
      break;

    case MESSAGE__CONTENT_KEYS:
      copied_msg->keys = _KeysMessage_dup(msg->keys);
      break;

    case MESSAGE__CONTENT_INT_RESULT:
      copied_msg->int_result = msg->int_result;
      break;

    case MESSAGE__CONTENT_OP_ID:
      copied_msg->op_id = msg->op_id;
      break;

    default:
      logger_error("Message_dup", "Unknown content_case");
      return NULL;
  }
  return copied_msg;
}

void Message_destroy(Message* msg) {
  if (msg != NULL) {
    message__free_unpacked(msg, NULL);
  }
}

void message_destroy(struct message_t* message) {
  if (message != NULL) {
    Message_destroy(message->msg);
    free(message);
  }
}