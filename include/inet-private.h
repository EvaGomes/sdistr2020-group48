/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */

#include "message-private.h"

/* Returns the Internet address of the host machine, or NULL if an error occurred. */
struct in_addr* get_host();

/* Parses the address_port string, sets pointer_to_address to the parsed IP address and sets
 * pointer_to_port to the parsed and converted and port.
 *  Expects address_port to have format "<ip-address>:<port>", where <ip-address> is a valid IP
 *  address (X.X.X.X) and <port> is a positive number.
 *  Returns 0 if the string was parsed correctly, or -1 if some error occurred.
 */
int parse_address_port(char* address_port, char** pointer_to_address, int* pointer_to_port);

/* Creates a TCP socket and connects it to the server at the given IP address and port.
 *  Returns the descriptor of the created socket if the connection was established successfully, or
 * -1 if an error occurred.
 */
int server_connect(char* server_ip_address, int server_port);

/* Reads bytes from the socket with the given descriptor and de-serializes them into a message_t.
 *  Returns the de-serialized message_t, or NULL if an error occurred.
 */
struct message_t* network_receive_message(int sockfd);

/* Serializes the given message_t and writes it to the socket with the given descriptor.
 *  Returns 0 if the message was successfully sent, or -1 if an error occurred.
 */
int network_send_message(int sockfd, struct message_t* msg);