/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#ifndef _SERIALIZATION_PRIVATE_H
#define _SERIALIZATION_PRIVATE_H

#include "message-private.h"

int message_to_buffer(struct message_t* message, char** message_buf);
struct message_t* buffer_to_message(char* buffer, int buffer_size);

#endif
