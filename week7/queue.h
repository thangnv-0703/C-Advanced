#ifndef _Queue_
#define _Queue_


struct Node {
  char *nodePos;
  struct Node *next;
};

typedef struct Node* queue;
typedef char* element_t;
// create new empty stack
queue createQueue();

int isEmpty(queue s);

int isFull(queue s);

queue dequeue(queue s);

queue enqueue(queue s, element_t x);

queue peek(queue s);

queue freeQueue(queue *s);
int count(queue s);
#endif
