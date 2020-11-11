/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "message-private.h"

// tamanho máximo da mensagem enviada pelo cliente
#define MAX_MSG 2048

/* Registers an handler to ignore SIGPIPE signals.
 *  Returns 0 if registration went fine or -1 if it failed.
 */
int ignore_SIGPIPE_signals();

/* Reads bytes from the socket with the given descriptor and de-serializes them into a message_t.
 *  Returns the de-serialized message_t, or NULL if an error occurred.
 */
struct message_t* network_receive_message(int sockfd);

/* Serializes the given message_t and writes it to the socket with the given descriptor.
 *  Returns 0 if the message was successfully sent, or -1 if an error occurred.
 */
int network_send_message(int sockfd, struct message_t* msg);