#ifndef __Data__
#define __Data__

enum nodeColor {
  RED,
  BLACK
};

typedef struct rbNode{
  int data, color;
  struct rbNode *left;
  struct rbNode *right;
  struct rbNode *parent;
}rbNode_t;

#endif