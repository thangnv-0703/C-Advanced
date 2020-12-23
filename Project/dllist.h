#ifndef _pQ_H_
#define _pQ_H_

typedef int element_t;

struct Node {
  int id;
  float cost;
  struct Node *next;
  struct Node *prev;
};

typedef struct Node* pQ;
typedef struct Node* node_t;

// create new empty list
pQ create_list();
node_t make_node(int id,float cost);
// check whether a list is empty
int pQ_is_empty(pQ l);

// remove an element
pQ pQ_remove(pQ l);

//insert element
pQ pQ_insert(pQ l,node_t t);

// first node
node_t pQ_first(pQ l);

void pQ_freeNode(node_t n);

node_t pQ_hasNode(pQ l,int id);
// free a list
void pQ_free_list(pQ l);

pQ reArrangeQueue(pQ l,node_t t);
pQ removeAt(pQ l,node_t t);
#endif