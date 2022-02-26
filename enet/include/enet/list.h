/** 
 @file  list.h
 @brief ENet list management 
*/
#ifndef __ENET_LIST_H__
#define __ENET_LIST_H__

#include <stdlib.h>

typedef struct _ENetListNode
{
   struct _ENetListNode * next;
   struct _ENetListNode * previous;
} ENetListNode;

typedef ENetListNode * ENetListIterator;

typedef struct _ENetList
{
   ENetListNode sentinel;
} ENetList;

extern void renet_list_clear (ENetList *);

extern ENetListIterator renet_list_insert (ENetListIterator, void *);
extern void * renet_list_remove (ENetListIterator);
extern ENetListIterator renet_list_move (ENetListIterator, void *, void *);

extern size_t renet_list_size (ENetList *);

#define renet_list_begin(list) ((list) -> sentinel.next)
#define renet_list_end(list) (& (list) -> sentinel)

#define renet_list_empty(list) (renet_list_begin (list) == renet_list_end (list))

#define renet_list_next(iterator) ((iterator) -> next)
#define renet_list_previous(iterator) ((iterator) -> previous)

#define renet_list_front(list) ((void *) (list) -> sentinel.next)
#define renet_list_back(list) ((void *) (list) -> sentinel.previous)

#endif /* __ENET_LIST_H__ */

