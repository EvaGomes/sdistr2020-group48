# sdistr2020-group48-proj3


## Known limitations:

- Tree does not balance itself when inserting new nodes.

- (solved) ~~Serialization of entries does not handle entries with null keys or null values.~~

- (solved) ~~Serialization of trees does not handle entries with null keys or null values.~~

- Does not use standard error handling ("errno" and "perror").

- `tree_put` refuses NULL keys. By specification, function `tree_get_keys` returns NULL in the last
 entry to flag the end of the array; if we accepted NULL keys in the tree it would be impossible to
 distinguish if a NULL array-entry represented a NULL key or the array termination.
 
- `tree_put` refuses NULL values. By specification, function `tree_get` returns NULL if errors
 occur; if we accepted NULL values in the tree, it would be impossible to distinguish whether
 `tree_get` was returning a stored value or indicating that an error occurred.
 
- Currently stores an unlimited number of task-results. Should consider limiting it (to 100, for
  example). We could drop the results of the oldest operations (which are unlikely to be verified
  again).
