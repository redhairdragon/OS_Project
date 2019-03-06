// NAME: SHEN TENG
// EMAIL: REDHAIRDRAGON@UCLA.EDU
// ID: 104758168
#include "SortedList.h"
#include <sched.h>
#include <string.h>
#include <stdlib.h>

int opt_yield;
void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
	if(element == NULL||list==NULL)
		return;

	SortedListElement_t * curr = list;

	if(opt_yield & INSERT_YIELD)
		sched_yield();
	
	while(curr->next!=list){
		if(strcmp(element->key,curr->next->key)>=0)
			curr=curr->next;
		else
			break;
	}
	element->next=curr->next;
	element->prev=curr;
	curr->next->prev=element;
	curr->next=element;	
}

int SortedList_delete( SortedListElement_t *element){
	if(element->next->prev!=element||element->prev->next!=element)
		return 1;
	
	if(opt_yield & DELETE_YIELD)
		sched_yield();
	
	SortedListElement_t* p=element->prev;
	SortedListElement_t* n=element->next;
	n->prev=p;
	p->next=n;

	element->next=NULL;
	element->prev=NULL;
	return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
	if(list==NULL)
		return NULL;
	if(key==NULL)
		return list;
	if(opt_yield & LOOKUP_YIELD) 
		sched_yield();

	SortedListElement_t* curr=list->next;
	while(curr!=list){
		if(strcmp(key,curr->key)==0)
			return curr;
		else
			curr=curr->next;
	}
	return NULL;
}

int SortedList_length(SortedList_t *list){
	int count=0;
	if(opt_yield & LOOKUP_YIELD) 
		sched_yield();
	SortedListElement_t* curr=list->next;
	while(curr!=list){
		count=count+1;
		curr=curr->next;
	}
	return count;
}