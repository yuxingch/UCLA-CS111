#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "SortedList.h"

/* use circular doubly linked list  */

/**
 * SortedList_insert ... insert an element into a sorted list
 *
 *	The specified element will be inserted in to
 *	the specified list, which will be kept sorted
 *	in ascending order based on associated keys
 *
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
    //  Check if the list, elements are nullptrs, also as the head of the list, list->key shoule be NULL
    if (list == NULL || element == NULL || list->key != NULL)
        return;
    SortedList_t* old = list;
    SortedList_t* curr = list->next;
    
    
    while (curr != list && strcmp(curr->key, element->key) <= 0) {
        if (opt_yield & INSERT_YIELD)
            pthread_yield();
        old = curr;
        curr = curr->next;
    }
    if (opt_yield & INSERT_YIELD)
        pthread_yield();
    element->next = old->next;
    element->prev = old;
    old->next = element;
    element->next->prev = element;
    
    return;
}

//void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
//    if (list == NULL || element == NULL || list->key != NULL)
//        return;
//    
//    SortedList_t* old = list;
//    SortedList_t* curr = list->next;
//    
//    //  Compare the keys and insert
//    //  the prev of element should be linked to the old
//    //  the next of element should be linked to the next of the old
//    //  We should keep checking until we reach the end of the doubly-linked list <=> old->next is the head of our list
//    if (curr == list) {
//        if (opt_yield & INSERT_YIELD)
//            pthread_yield();
//        list->next = element;
//        list->prev = element;
//        element->next = list;
//        element->prev = list;
//        return;
//    }
//    
//    while (curr != list) {
//        if (strcmp(curr->key, element->key) > 0)
//        {
//            if (opt_yield & INSERT_YIELD)
//                pthread_yield();
//            element->prev = curr->prev;
//            element->next = curr;
//            curr->prev->next = element;
//            curr->prev = element;
//            return;
//        }
//        old = curr;
//        curr = curr->next;
//    }
//    
//    if (opt_yield & INSERT_YIELD)
//        pthread_yield();
//    element->prev = old;
//    element->next = list;
//    list->prev = element;
//    old->next = element;
//    return;
//}

/**
 * SortedList_delete ... remove an element from a sorted list
 *
 *	The specified element will be removed from whatever
 *	list it is currently in.
 *
 *	Before doing the deletion, we check to make sure that
 *	next->prev and prev->next both point to this node
 *
 * @param SortedListElement_t *element ... element to be removed
 *
 * @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 *
 */
int SortedList_delete( SortedListElement_t *element) {
    if (element->next->prev != element || element->prev->next != element || element == NULL || element->key == NULL)
        return 1;
    
    if (opt_yield & DELETE_YIELD)
        pthread_yield();
    
    element->prev->next = element->next;
    element->next->prev = element->prev;
    return 0;
}

/**
 * SortedList_lookup ... search sorted list for a key
 *
 *	The specified list will be searched for an
 *	element with the specified key.
 *
 * @param SortedList_t *list ... header for the list
 * @param const char * key ... the desired key
 *
 * @return pointer to matching element, or NULL if none is found
 */

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
    //  return pointer to matching element, or NULL if none is found
    if (list == NULL || list->key != NULL)
        return NULL;
    SortedListElement_t* curr = list->next;
    
    while (curr != list) {
        if (strcmp(curr->key, key) == 0)
            return curr;
        if (opt_yield & LOOKUP_YIELD) //?
            pthread_yield();
        curr = curr->next;
    }
    return NULL;
}

/**
 * SortedList_length ... count elements in a sorted list
 *	While enumeratign list, it checks all prev/next pointers
 *
 * @param SortedList_t *list ... header for the list
 *
 * @return int number of elements in list (excluding head)
 *	   -1 if the list is corrupted
 */
int SortedList_length(SortedList_t *list) {
    if (list == NULL || list->key != NULL)
        return -1;  //?
    SortedList_t* curr = list->next;
    int counter = 0;
    while(curr != list) {
        if (curr->next->prev != curr || curr->prev->next != curr)
            return -1;
        counter++;
        if (opt_yield & LOOKUP_YIELD) //?
            pthread_yield();
        curr = curr->next;
    }
    return counter;
}

