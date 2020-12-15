/* Grupo 48
 *   Eva Gomes (37806)
 *   João Santos (40335)
 *   João Vieira (45677)
 */
#ifndef _ZK_PRIVATE_H
#define _ZK_PRIVATE_H

#include "server_role-private.h"

/* Connects to the zookeeper server at the given IP address and port and, in case it isn't created
 * yet, creates the root ZNode (with content "/kvstore").
 */
int zk_connect(const char* zookeeper_address_and_port);

/* Registers the tree_server that is listening in the given socket in the Zookeeper server. If no
 * primary server is registered yet, sets the tree_server as the primary server and returns
 * ServerRole PRIMARY. Otherwise, if no backup tree_server is registered yet, sets the tree_server
 * as backup and returns ServerRole BACKUP. If both a primary and a backup tree_server are already
 * registered, the tree_server is not registered and returns ServerRole NONE.
 *  Returns the ServerRole given to the tree_server, or NONE if an error occurred.
 */
enum ServerRole zk_register_tree_server(int server_listening_socket_fd);

/* Returns the "<ip-address>:<port>" of the primary tree_server, or NULL if it isn't registered. */
char* zk_get_primary_tree_server();
/* Returns the "<ip-address>:<port>" of the backup tree_server, or NULL if it isn't registered. */
char* zk_get_backup_tree_server();

typedef void (*servers_listener_fn)(void* context);
/* Registers a function to be notified if the registered servers change. */
void zk_register_servers_listener(servers_listener_fn listener, void* context);

/* Closes the connection to Zookeeper and frees-up its resources. */
void zk_close();

#endif
