/*
*****************************************************************************************************
*	Semi-automatic malloc-free library:																*
*	Implements new versions of malloc, calloc, realloc and free with the "sa_" prefix				*
*	These functions do almost the same as their pairs in stdlib.h, with a little difference:		*
*	The library builds a list of pointers allocated by its functions								*
*	When sa_freeall is called, every pointer created by the library will be freed.					*
*	It will purge all pointers in the list upon exit.												*
*	Rules of use:																					*
*		1. 	DO NOT MIX THE POINTERS RETURNED BY THE LIBRARY AND THE POINTERS OF 					*
*			STANDARD MEMORY ALLOCATION FUNCTIONS (e.g.: do not call free for a sa_malloc pointer)	*
*			USE "sa_" FUNCTIONS FOR SA POINTERS. HOWEVER, YOU CAN TURN A STANDARD DYNAMIC POINTER	*
			INTO A "sa_" POINTER WITH "sa_add"														*
*																									*
*		2. 	THE LIBRARY DOES NOT PREVENT LONG-TERM MEMORY LEAKS, FREEING ALLOCATED MEMORIES 		*
*			PERIODICALLY IS NECESSARY																*
*																									*
*		3. 	IT MIGHT NOT BE EFFICIENT TO CALL "sa_realloc"											*
*																									*
*		4. 	IT IS CERTAINLY NOT EFIICIENT TO CALL "sa_free" TOO MUCH - IT IS BETTER TO ACCUMULATE	*
*			SOME POINTERS AND FREE THEM ALL (e.g in event controlled apps 							*
*			at the end of the event control loop)													*
*																									*
*		5.	IT'S NOT ALWAYS THE BEST IDEA TO MASS-FREE MEMORY, FOR VARIABLES						*
*			WITH A MORE FLEXIBLE LIFETIME USE THE STANDARD ALLOCATION FUNCTIONS						*
*																									*
*		6.	THE BEST WAY TO FAMILIARIZE YOURSELF WITH THE LIBRARY IS LOOKING AT THE CODE - IT HAS	*
*			QUITE A SIMPLE CONCEPT, THAT YOU CAN EASILY UNDERSTAND									*
*****************************************************************************************************
*/
#ifndef SA_MALLOC_H_INCLUDED
#define SA_MALLOC_H_INCLUDED
#include <stdlib.h>
void *sa_malloc(size_t size);
void *sa_calloc(size_t element, size_t unit);
void *sa_realloc(void *ptr, size_t size);
void *sa_calloc(size_t element, size_t unit);
void sa_add(void *ptr);
void sa_free(void *ptr);
void sa_freeall(void);
typedef enum SA_ListAction {
    Add, Remove, Purge
} SA_ListAction;
typedef struct SA_SemiAutoPointers {
    void *dynpointer;
    struct SA_SemiAutoPointers *next;
} SA_SemiAutoPointers;

static SA_SemiAutoPointers *addNew(SA_SemiAutoPointers *list, void *pointer)
{
    static int calls = 0; //counts function calls

    if(calls == 0) //adds atexit hook, when the first item is added
        atexit(sa_freeall);

    ++calls;
    SA_SemiAutoPointers *new_p = NULL;
    while(new_p == NULL) //For the library to work correctly, the allocation must happen eventually
        new_p = (SA_SemiAutoPointers*)malloc(sizeof(SA_SemiAutoPointers));
    new_p->dynpointer = pointer;
    new_p->next = list;
    return new_p;
}
static SA_SemiAutoPointers *removePtr(SA_SemiAutoPointers *list, void *pointer)
{
    SA_SemiAutoPointers *prev = NULL;
    SA_SemiAutoPointers *cur = list;
    while(cur != NULL)
    {
        if(pointer == cur->dynpointer) //match
        {
            if(prev == NULL)
            {
                SA_SemiAutoPointers *tofree = cur;
                cur = cur->next;
                list = cur;
                free(tofree);
            }
            else
            {
                prev->next = cur->next;
                free(cur);
                cur = prev->next;
            }
        }
        else
        {
            prev = cur;
            cur = cur->next;
        }
    }
    return list;
}
static void purgeAllocs(SA_SemiAutoPointers *list)
{
    while(list != NULL)
    {
        free(list->dynpointer);
        SA_SemiAutoPointers *tofree = list;
        list = list->next;
        free(tofree);
    }
}
static void listmanager(SA_ListAction action, void *ptr)
{
    static SA_SemiAutoPointers *ptrlist = NULL; //only when called first
    switch(action)
    {
        case Add:
            ptrlist = addNew(ptrlist, ptr);
            break;
        case Remove:
            ptrlist = removePtr(ptrlist, ptr);
            break;
        case Purge:
            purgeAllocs(ptrlist);
            ptrlist = NULL;
            break;
        default:
            break;

    }
}
//Mallocs a pointers then adds it to the linked list
void *sa_malloc(size_t size)
{
    void *ptr = malloc(size);
    listmanager(Add, ptr);
    return ptr;
}
//Calls calloc, adds pointer to list
void *sa_calloc(size_t element, size_t unit)
{
    void *ptr = calloc(element, unit);
    listmanager(Add, ptr);
    return ptr;
}
//Calls realloc, modifies list if needed
void *sa_realloc(void *ptr, size_t size)
{
    void *newptr = realloc(ptr, size);
    if(ptr != newptr)
    {
        listmanager(Remove, ptr);
        listmanager(Add, newptr);
    }
    return newptr;
}
//Adds a given pointer to the linked list - The given pointer must point to dynamically allocated memory space
void sa_add(void *ptr)
{
	listmanager(Add, ptr);
}
//Frees an individual pointer, and deletes it from the list - It's not efficient to overuse it
void sa_free(void *ptr)
{
    listmanager(Remove, ptr);
    free(ptr);
}
//Frees every pointer in the list, and purges the list at the same time
void sa_freeall(void)
{
    listmanager(Purge, NULL);
}
#endif // SA_MALLOC_H_INCLUDED
