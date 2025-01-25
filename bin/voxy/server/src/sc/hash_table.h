// Before including this header, the following should be defined:
//  - SC_HASH_TABLE_PREFIX
//  - SC_HASH_TABLE_NODE_TYPE
//  - SC_HASH_TABLE_KEY_TYPE
//
// SC_HASH_TABLE_NODE_TYPE:
//   must at minimal have the same fields as the following struct:
//    - struct SC_HASH_TABLE_NODE_TYPE *next
//    - size_t                          hash

#ifndef SC_HASH_TABLE_H
#define SC_HASH_TABLE_H

#define SC_HASH_TABLE_CONCAT2_INNER(a, b) a ## b
#define SC_HASH_TABLE_CONCAT2(a, b) SC_HASH_TABLE_CONCAT2_INNER(a, b)

#define SC_HASH_TABLE_CONCAT3_INNER(a, b, c) a ## b ## c
#define SC_HASH_TABLE_CONCAT3(a, b, c) SC_HASH_TABLE_CONCAT3_INNER(a, b, c)

#define SC_HASH_TABLE_MAKE_NAME(name)          SC_HASH_TABLE_CONCAT3(SC_HASH_TABLE_PREFIX, _, name)
#define SC_HASH_TABLE_MAKE_NAME_INTERNAL(name) SC_HASH_TABLE_CONCAT2(_, SC_HASH_TABLE_MAKE_NAME(name))

#endif // SC_HASH_TABLE_H

#define SC_HASH_TABLE_BUCKET_COUNT_FROM_ORDER(order) \
  ({ \
   size_t _order = (order); \
   _order != 0 ? (size_t)1 << _order : (size_t)0; \
  })

#define SC_HASH_TABLE_BUCKET_COUNT_MASK(order) (SC_HASH_TABLE_BUCKET_COUNT_FROM_ORDER(order) - (size_t)1)

#define SC_HASH_TABLE_FOREACH(hash_table, name) \
  for(size_t i=0; i<SC_HASH_TABLE_BUCKET_COUNT_FROM_ORDER((hash_table).bucket_order); ++i) \
    for(name = (hash_table).buckets[i].head; name; name = name->next)

#ifdef SC_HASH_TABLE_INTERFACE
#include <stddef.h>

struct SC_HASH_TABLE_MAKE_NAME(hash_table_bucket)
{
  SC_HASH_TABLE_NODE_TYPE *head;
};

struct SC_HASH_TABLE_MAKE_NAME(hash_table)
{
  struct SC_HASH_TABLE_MAKE_NAME(hash_table_bucket) *buckets;
  size_t                                             bucket_order;
  size_t                                             load;
};

void SC_HASH_TABLE_MAKE_NAME(hash_table_init)(struct SC_HASH_TABLE_MAKE_NAME(hash_table) *hash_table);
void SC_HASH_TABLE_MAKE_NAME(hash_table_rehash)(struct SC_HASH_TABLE_MAKE_NAME(hash_table) *hash_table, size_t bucket_order);
int SC_HASH_TABLE_MAKE_NAME(hash_table_insert)(struct SC_HASH_TABLE_MAKE_NAME(hash_table) *hash_table, SC_HASH_TABLE_NODE_TYPE *node);
int SC_HASH_TABLE_MAKE_NAME(hash_table_insert_unchecked)(struct SC_HASH_TABLE_MAKE_NAME(hash_table) *hash_table, SC_HASH_TABLE_NODE_TYPE *node);
SC_HASH_TABLE_NODE_TYPE *SC_HASH_TABLE_MAKE_NAME(hash_table_lookup)(struct SC_HASH_TABLE_MAKE_NAME(hash_table) *hash_table, SC_HASH_TABLE_KEY_TYPE key);
SC_HASH_TABLE_NODE_TYPE *SC_HASH_TABLE_MAKE_NAME(hash_table_remove)(struct SC_HASH_TABLE_MAKE_NAME(hash_table) *hash_table, SC_HASH_TABLE_KEY_TYPE key);
void SC_HASH_TABLE_MAKE_NAME(hash_table_dispose)(struct SC_HASH_TABLE_MAKE_NAME(hash_table) *hash_table);

#endif // SC_HASH_TABLE_INTERFACE

#ifdef SC_HASH_TABLE_IMPLEMENTATION
#include <assert.h>
#include <stdlib.h>

SC_HASH_TABLE_KEY_TYPE SC_HASH_TABLE_MAKE_NAME(key)(SC_HASH_TABLE_NODE_TYPE *node);
size_t SC_HASH_TABLE_MAKE_NAME(hash)(SC_HASH_TABLE_KEY_TYPE key);
int SC_HASH_TABLE_MAKE_NAME(compare)(SC_HASH_TABLE_KEY_TYPE key1, SC_HASH_TABLE_KEY_TYPE key2);
void SC_HASH_TABLE_MAKE_NAME(dispose)(SC_HASH_TABLE_NODE_TYPE *node);

void SC_HASH_TABLE_MAKE_NAME(hash_table_init)(struct SC_HASH_TABLE_MAKE_NAME(hash_table) *hash_table)
{
  hash_table->buckets      = NULL;
  hash_table->bucket_order = 0;
  hash_table->load         = 0;
}

void SC_HASH_TABLE_MAKE_NAME(hash_table_rehash)(struct SC_HASH_TABLE_MAKE_NAME(hash_table) *hash_table, size_t bucket_order)
{
  if(hash_table->bucket_order >= bucket_order)
    return;

  hash_table->buckets = reallocarray(hash_table->buckets, SC_HASH_TABLE_BUCKET_COUNT_FROM_ORDER(bucket_order), sizeof hash_table->buckets[0]);
  for(size_t i=SC_HASH_TABLE_BUCKET_COUNT_FROM_ORDER(hash_table->bucket_order); i<SC_HASH_TABLE_BUCKET_COUNT_FROM_ORDER(bucket_order); ++i)
    hash_table->buckets[i].head = NULL;
  hash_table->bucket_order = bucket_order;

  SC_HASH_TABLE_NODE_TYPE *orphans = NULL;
  for(size_t i=0; i<SC_HASH_TABLE_BUCKET_COUNT_FROM_ORDER(hash_table->bucket_order); ++i)
  {
    struct SC_HASH_TABLE_MAKE_NAME(hash_table_bucket) *bucket = &hash_table->buckets[i];

    SC_HASH_TABLE_NODE_TYPE **it = &bucket->head;
    while(*it)
      if(((*it)->hash & SC_HASH_TABLE_BUCKET_COUNT_MASK(hash_table->bucket_order)) != i)
      {
        SC_HASH_TABLE_NODE_TYPE *orphan = *it;
        *it = (*it)->next;

        orphan->next = orphans;
        orphans      = orphan;
      }
      else
        it = &((*it)->next);
  }

  while(orphans)
  {
    SC_HASH_TABLE_NODE_TYPE *orphan = orphans;
    orphans = orphans->next;

    struct SC_HASH_TABLE_MAKE_NAME(hash_table_bucket) *bucket = &hash_table->buckets[orphan->hash & SC_HASH_TABLE_BUCKET_COUNT_MASK(hash_table->bucket_order)];
    orphan->next = bucket->head;
    bucket->head = orphan;
  }
}

int SC_HASH_TABLE_MAKE_NAME(hash_table_insert)(struct SC_HASH_TABLE_MAKE_NAME(hash_table) *hash_table, SC_HASH_TABLE_NODE_TYPE *new_node)
{
  if(hash_table->bucket_order == 0)
    SC_HASH_TABLE_MAKE_NAME(hash_table_rehash)(hash_table, 1);
  else if(2 * hash_table->load > 3 * SC_HASH_TABLE_BUCKET_COUNT_FROM_ORDER(hash_table->bucket_order))
    SC_HASH_TABLE_MAKE_NAME(hash_table_rehash)(hash_table, hash_table->bucket_order+1);

  size_t hash = SC_HASH_TABLE_MAKE_NAME(hash)(SC_HASH_TABLE_MAKE_NAME(key)(new_node));

  struct SC_HASH_TABLE_MAKE_NAME(hash_table_bucket) *bucket = &hash_table->buckets[hash & SC_HASH_TABLE_BUCKET_COUNT_MASK(hash_table->bucket_order)];
  for(SC_HASH_TABLE_NODE_TYPE *node = bucket->head; node; node = node->next)
    if(node->hash == hash && SC_HASH_TABLE_MAKE_NAME(compare)(SC_HASH_TABLE_MAKE_NAME(key)(node), SC_HASH_TABLE_MAKE_NAME(key)(new_node)) == 0)
      return 0;

  ++hash_table->load;
  new_node->hash = hash;
  new_node->next = bucket->head;
  bucket->head = new_node;
  return 1;
}

int SC_HASH_TABLE_MAKE_NAME(hash_table_insert_unchecked)(struct SC_HASH_TABLE_MAKE_NAME(hash_table) *hash_table, SC_HASH_TABLE_NODE_TYPE *new_node)
{
  if(hash_table->bucket_order == 0)
    SC_HASH_TABLE_MAKE_NAME(hash_table_rehash)(hash_table, 1);
  else if(2 * hash_table->load > 3 * SC_HASH_TABLE_BUCKET_COUNT_FROM_ORDER(hash_table->bucket_order))
    SC_HASH_TABLE_MAKE_NAME(hash_table_rehash)(hash_table, hash_table->bucket_order+1);

  size_t hash = SC_HASH_TABLE_MAKE_NAME(hash)(SC_HASH_TABLE_MAKE_NAME(key)(new_node));

  struct SC_HASH_TABLE_MAKE_NAME(hash_table_bucket) *bucket = &hash_table->buckets[hash & SC_HASH_TABLE_BUCKET_COUNT_MASK(hash_table->bucket_order)];
  ++hash_table->load;
  new_node->hash = hash;
  new_node->next = bucket->head;
  bucket->head = new_node;
  return 1;
}


SC_HASH_TABLE_NODE_TYPE *SC_HASH_TABLE_MAKE_NAME(hash_table_lookup)(struct SC_HASH_TABLE_MAKE_NAME(hash_table) *hash_table, SC_HASH_TABLE_KEY_TYPE key)
{
  if(hash_table->bucket_order == 0)
    return NULL;

  size_t hash = SC_HASH_TABLE_MAKE_NAME(hash)(key);

  struct SC_HASH_TABLE_MAKE_NAME(hash_table_bucket) *bucket = &hash_table->buckets[hash & SC_HASH_TABLE_BUCKET_COUNT_MASK(hash_table->bucket_order)];
  for(SC_HASH_TABLE_NODE_TYPE *node = bucket->head; node; node = node->next)
    if(node->hash == hash && SC_HASH_TABLE_MAKE_NAME(compare)(SC_HASH_TABLE_MAKE_NAME(key)(node), key) == 0)
      return node;

  return NULL;
}

SC_HASH_TABLE_NODE_TYPE *SC_HASH_TABLE_MAKE_NAME(hash_table_remove)(struct SC_HASH_TABLE_MAKE_NAME(hash_table) *hash_table, SC_HASH_TABLE_KEY_TYPE key)
{
  if(hash_table->bucket_order == 0)
    return NULL;

  size_t hash = SC_HASH_TABLE_MAKE_NAME(hash)(key);

  struct SC_HASH_TABLE_MAKE_NAME(hash_table_bucket) *bucket = &hash_table->buckets[hash & SC_HASH_TABLE_BUCKET_COUNT_MASK(hash_table->bucket_order)];

  SC_HASH_TABLE_NODE_TYPE **it = &bucket->head;
  while(*it)
    if((*it)->hash == hash && SC_HASH_TABLE_MAKE_NAME(compare)(SC_HASH_TABLE_MAKE_NAME(key)(*it), key) == 0)
    {
      SC_HASH_TABLE_NODE_TYPE *orphan = *it;
      *it = (*it)->next;

      --hash_table->load;
      return orphan;
    }
    else
      it = &((*it)->next);

  return NULL;
}

void SC_HASH_TABLE_MAKE_NAME(hash_table_dispose)(struct SC_HASH_TABLE_MAKE_NAME(hash_table) *hash_table)
{
  for(size_t i=0; i<SC_HASH_TABLE_BUCKET_COUNT_FROM_ORDER(hash_table->bucket_order); ++i)
  {
    struct SC_HASH_TABLE_MAKE_NAME(hash_table_bucket) *bucket = &hash_table->buckets[i];
    SC_HASH_TABLE_NODE_TYPE *node = bucket->head;
    while(node)
    {
      SC_HASH_TABLE_NODE_TYPE *next = node->next;
      SC_HASH_TABLE_MAKE_NAME(dispose)(node);
      node = next;
    }
  }
  free(hash_table->buckets);

  hash_table->buckets      = 0;
  hash_table->bucket_order = 0;
  hash_table->load         = 0;
}

#endif
