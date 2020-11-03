#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"

stack create_stack() {
  // TODO
  return NULL;
}
stack make_node(element_t x){
  stack n = (stack) malloc(sizeof(stack));
  n->nodePos = x;
  n->next = NULL;
  return n;
}
int is_empty(stack s) {
  // TODO
  return (s == NULL);
}

int is_full(stack s) {
  // TODO
  char *x = (char*) malloc(sizeof(char));
  x[0] = 'A';
  stack n = make_node(x);
  return (n == NULL);
}

element_t top(stack s) {
  // TODO
  return s->nodePos;
}

stack push(stack s, element_t x) {
  // TODO
 
  if(is_full(s)){
    return s;
  }
  stack n = make_node(x);
  if(s == NULL){
    s = n;
    return n;
  }
  n->next = s;
  return n;
}

stack pop(stack s) {
  // TODO
  if(is_empty(s)){
    return NULL;
  }
  stack n = s->next;
  free(s);
  return n;
}

void free_stack(stack *s) {
  // TODO
   stack tmp;

   while ((*s) != NULL)
    {
       tmp = (*s);
       (*s) = (*s)->next;
       free(tmp);
    }
}