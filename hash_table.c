#include "hash_table.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(UINT32_MAX)
  typedef uint32_t hash_code_t;
  typedef uint32_t bucket_size_t;
#elif defined(_WIN32) || defined(_WIN64)
  typedef unsigned __int32 hash_code_t;
  typedef unsigned __int32 bucket_size_t;
#else
  typedef uint32_t hash_code_t;
  typedef uint32_t bucket_size_t;
#endif

#define MAXIMUM_BUCKET_SIZE ((bucket_size_t) (1 << 31))

typedef struct bucket_struct* bucket_t;

struct bucket_struct {
  const char* key;
  hash_code_t hash;
  void* value;
  bucket_t next;
};

struct hash_table_struct {
  bucket_size_t size;
  bucket_size_t bucket_size;
  bucket_t buckets;
};

struct hash_table_iterator_struct {
  hash_table_t table;
  bucket_t bucket;
  bucket_size_t index;
};

#define HASH_TABLE_SIZE          sizeof(struct hash_table_struct)
#define BUCKET_SIZE              sizeof(struct bucket_struct)
#define HASH_TABLE_ITERATOR_SIZE sizeof(struct hash_table_iterator_struct)
#define HASH_PRIME               47
#define MINIMUM_BUCKET_SIZE      (1 << 4)
#define LOAD_FACTOR              0.75f

#define TABLE_SIZE(_TABLE_) ((_TABLE_)->size)
#define TABLE_LOAD_FACTOR(_TABLE_, _LOAD_FACTOR_) (((float) BUCKET_COUNT(_TABLE_)) * ((float) (_LOAD_FACTOR_)))

#define BUCKET_INDEX(_BUCKET_SIZE_, _HASH_) (((_BUCKET_SIZE_) - 1) & (_HASH_))
#define BUCKET_COUNT(_TABLE_) ((_TABLE_)->bucket_size)
#define GET_BUCKET(_TABLE_, _HASH_) (&((_TABLE_)->buckets[BUCKET_INDEX(BUCKET_COUNT(_TABLE_), _HASH_)]))
#define INIT_BUCKET(_BUCKET_, _KEY_, _HASH_, _VALUE_) \
  ((_BUCKET_)->key = (_KEY_)); \
  ((_BUCKET_)->hash = (_HASH_)); \
  ((_BUCKET_)->value = (_VALUE_)); \
  ((_BUCKET_)->next = 0);

static hash_code_t calculate_hash(const char* cstring) {
  hash_code_t hash = 0;

  char ch;
  while((ch = *cstring)) {
    hash = (HASH_PRIME * hash) + ch;
    cstring += sizeof(char);
  }

  return hash;
}

hash_table_t hash_table(void) {
  hash_table_t table = malloc(HASH_TABLE_SIZE);

  table->size = 0;
  table->bucket_size = 0;
  table->buckets = 0;

  return table;
}

static bucket_size_t calculate_table_size(
    bucket_size_t size) {

  size |= size >> 1;
  size |= size >> 2;
  size |= size >> 4;
  size |= size >> 8;
  size |= size >> 16;

  if (size < MAXIMUM_BUCKET_SIZE) {
    return (size + 1);
  } else {
    return MAXIMUM_BUCKET_SIZE;
  }
}

static int initialize_buckets(
    const hash_table_t table) {

  const bucket_size_t current_size = BUCKET_COUNT(table);
  const bucket_size_t new_size = calculate_table_size(current_size + 4);

  if (new_size != current_size) {
    const bucket_t buckets = malloc(new_size * BUCKET_SIZE);

    for(bucket_size_t index = 0; index < new_size; ++index) {
      buckets[index].key = 0;
    }

    table->bucket_size = new_size;
    table->buckets = buckets;

    return 1;
  } else {
    return 0;
  }
}

static void* hash_table_bucket_put(
    bucket_t bucket,
    const char* key,
    const hash_code_t hash,
    void* value,
    bucket_size_t* size) {

  const char* bucket_key = bucket->key;
  if (bucket_key == 0) {
    INIT_BUCKET(bucket, key, hash, value);
    *size += 1;
    return 0;
  }

  do {
    if ((hash == bucket->hash) &&
        (strcmp(bucket_key, key) == 0)) {

      void* old_value = bucket->value;
      bucket->value = value;
      return old_value;
    }

    const bucket_t next = bucket->next;
    if (next == 0) {
      break;
    }

    bucket = next;
    bucket_key = bucket->key;
  } while(1);

  const bucket_t link = malloc(BUCKET_SIZE);
  bucket->next = link;

  INIT_BUCKET(link, key, hash, value);
  *size += 1;
  return 0;
}

static int hash_table_resize(
    const hash_table_t table) {

  const bucket_size_t bucket_size = BUCKET_COUNT(table);
  const bucket_t buckets = table->buckets;

  if (buckets != 0) {
    if (initialize_buckets(table) == 0) {
      return 0;
    }

    bucket_size_t s = 0;
    for(bucket_size_t bucket_index = 0; bucket_index < bucket_size; ++bucket_index) {
      const bucket_t bucket = &buckets[bucket_index];

      const char* key = bucket->key;
      if (key == 0) continue;

      hash_code_t hash = bucket->hash;
      hash_table_bucket_put(
        GET_BUCKET(table, hash),
        key,
        hash,
        bucket->value,
        &s);

      bucket_t link = bucket->next;
      if (link == 0) continue;

      do {
        hash = link->hash;
        hash_table_bucket_put(
          GET_BUCKET(table, hash),
          link->key,
          hash,
          link->value,
          &s);

        const bucket_t freeable = link;
        link = link->next;
        free(freeable);
      } while(link);
    }

    free(buckets);
  } else {
    table->bucket_size = MINIMUM_BUCKET_SIZE - 2;
    initialize_buckets(table);
  }

  return 1;
}

hash_table_t hash_table_sized(
    const size_t size) {

  hash_table_t table = hash_table();

  if ((size > 0) && (table != 0)) {
    table->bucket_size = calculate_table_size(size);
    if (hash_table_resize(table) == 0) {
      hash_table_free(table);
      return 0;
    }
  }

  return table;
}

void* hash_table_get(
    const hash_table_t table,
    const char* key) {

  if ((TABLE_SIZE(table) == 0) || (key == 0)) {
    return 0;
  }

  const hash_code_t hash = calculate_hash(key);
  bucket_t bucket = GET_BUCKET(table, hash);
  if (bucket->key == 0) {
    return 0;
  }

  do {
    const char* bucket_key = bucket->key;
    if ((hash == bucket->hash) &&
        (strcmp(key, bucket_key) == 0)) {

      return bucket->value;
    }
  } while((bucket = bucket->next) != 0);

  return 0;
}

int hash_table_contains(
    const hash_table_t table,
    const char* key) {

  if (hash_table_get(table, key) != 0) {
    return 1;
  } else {
    return 0;
  }
}

void* hash_table_put(
    const hash_table_t table,
    const char* key,
    void* value) {

  if (key == 0) {
    return 0;
  }

  if (TABLE_SIZE(table) >=
      TABLE_LOAD_FACTOR(table, LOAD_FACTOR)) {

    hash_table_resize(table);
  }

  const hash_code_t hash = calculate_hash(key);
  return hash_table_bucket_put(
    GET_BUCKET(table, hash),
    key,
    hash,
    value,
    &TABLE_SIZE(table));
}

const char* hash_table_get_key(
    const hash_table_t table,
    const char* key) {

  if ((TABLE_SIZE(table) == 0) || (key == 0)) {
    return 0;
  }

  const hash_code_t hash = calculate_hash(key);
  bucket_t bucket = GET_BUCKET(table, hash);
  if (bucket->key == 0) {
    return 0;
  }

  do {
    const char* bucket_key = bucket->key;
    if ((hash == bucket->hash) &&
        (strcmp(key, bucket_key) == 0)) {

      return bucket_key;
    }
  } while((bucket = bucket->next) != 0);

  return 0;
}

size_t hash_table_size(
    const hash_table_t table) {

  return (size_t) TABLE_SIZE(table);
}

void hash_table_free(
    const hash_table_t table) {

  const bucket_t buckets = table->buckets;
  if (buckets != 0) {
    const bucket_size_t length = BUCKET_COUNT(table);
    for(bucket_size_t index = 0; index < length; ++index) {
      if (buckets[index].key == 0) continue;
      bucket_t ptr = buckets[index].next;
      while(ptr) {
        const bucket_t freeable = ptr;
        ptr = ptr->next;
        free(freeable);
      }
    }

    free(buckets);
  }

  free(table);
}

hash_table_iterator_t hash_table_iterator(
    const hash_table_t table) {

  hash_table_iterator_t iterator = malloc(HASH_TABLE_ITERATOR_SIZE);
  iterator->table = table;
  iterator->bucket = 0;
  iterator->index = 0;
  return iterator;
}

int hash_table_next(
    const hash_table_iterator_t iterator) {

  bucket_t bucket = iterator->bucket;
  if ((bucket != 0) && (bucket->next != 0)) {
    iterator->bucket = bucket->next;
    return 1;
  }

  const hash_table_t table = iterator->table;
  const bucket_size_t length = BUCKET_COUNT(table);
  for(bucket_size_t index = iterator->index; index < length; ++index) {
    bucket = &table->buckets[index];
    if (bucket->key != 0) {
      iterator->bucket = bucket;
      iterator->index = index + 1;
      return 1;
    }
  }

  return 0;
}

const char* hash_table_key(
    const hash_table_iterator_t iterator) {

  return iterator->bucket->key;
}

void* hash_table_value(
    const hash_table_iterator_t iterator) {

  return iterator->bucket->value;
}

void hash_table_free_iterator(
    const hash_table_iterator_t iterator) {

  free(iterator);
}
