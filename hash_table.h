#ifndef __HASH__TABLE__H__
#define __HASH__TABLE__H__

  #include <stddef.h>

  typedef struct hash_table_struct* hash_table_t;
  typedef struct hash_table_iterator_struct* hash_table_iterator_t;

  hash_table_t hash_table(void);
  hash_table_t hash_table_sized(const size_t size);

  size_t hash_table_size(const hash_table_t table);
  int hash_table_contains(const hash_table_t table, const char* key);

  void* hash_table_put(const hash_table_t table, const char* key, void* value);
  void* hash_table_get(const hash_table_t table, const char* key);
  const char* hash_table_get_key(const hash_table_t table, const char* key);

  void hash_table_free(const hash_table_t table);

  hash_table_iterator_t hash_table_iterator(const hash_table_t table);

  int hash_table_next(const hash_table_iterator_t iterator);
  const char* hash_table_key(const hash_table_iterator_t iterator);
  void* hash_table_value(const hash_table_iterator_t iterator);

  void hash_table_free_iterator(const hash_table_iterator_t iterator);

#endif
