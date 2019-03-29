#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "hash_table.h"

static const char KEY_STRING[] = "key";
static const char VALUE_STRING[] = "value";
static const size_t KEY_STRING_SIZE = sizeof(KEY_STRING);
static const size_t VALUE_STRING_SIZE = sizeof(VALUE_STRING);

#define NUMBER_OF_ITERATIONS 1024*1024

int main() {

  hash_table_t table = hash_table();

  printf("Set with ever increasing keys\n");
  for(size_t count = 0; count < NUMBER_OF_ITERATIONS; ++count) {
    char number[16];
    sprintf(number, "%zu", count);
    const size_t number_length = strlen(number);

    char* key = malloc((number_length + KEY_STRING_SIZE + 1) * sizeof(char));
    sprintf(key, "%s%s", KEY_STRING, number);

    char* value = malloc((number_length + VALUE_STRING_SIZE + 1) * sizeof(char));
    sprintf(value, "%s%s", VALUE_STRING, number);

    if ((count % 1024) == 0) {
      printf("%s\n", key);
    }

    hash_table_put(table, key, value);
  }

  printf("Verify\n");

  for(size_t count = 0; count < NUMBER_OF_ITERATIONS; ++count) {
    char number[16];
    sprintf(number, "%zu", count);
    const size_t number_length = strlen(number);

    char* key = malloc((number_length + KEY_STRING_SIZE + 1) * sizeof(char));
    sprintf(key, "%s%s", KEY_STRING, number);

    char* value = malloc((number_length + VALUE_STRING_SIZE + 1) * sizeof(char));
    sprintf(value, "%s%s", VALUE_STRING, number);

    const char* validate = hash_table_get(table, key);
    if (0 != strcmp(value, validate)) {
      printf("Mismatch - %s (%s != %s)", key, value, validate);
      return 1;
    }

    free(key);
    free(value);
  }

  printf("Verify succeeded\n");
  printf("Free\n");

  hash_table_iterator_t iter = hash_table_iterator(table);
  while(hash_table_next(iter)) {
    free((char*) hash_table_key(iter));
    free(hash_table_value(iter));
  }

  hash_table_free_iterator(iter);
  hash_table_free(table);

  printf("Free complete\n");

  return 0;
}
