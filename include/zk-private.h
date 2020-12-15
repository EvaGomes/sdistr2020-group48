/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */
#ifndef _ZK_PRIVATE_H
#define _ZK_PRIVATE_H

/* Connects to the zookeeper server at the given IP address and port and, in case it isn't created
 * yet, creates the root ZNode (with content "/kvstore").
 */
int zk_connect(char* zookeeper_address_and_port);

/* Closes the connection to Zookeeper and frees-up its resources. */
int zk_close();

#endif
