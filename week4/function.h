#ifndef __Function__
#define __Function__
#include "data.h"

rbNode_t *createNode(int data);
void fixViolation(rbNode_t **root, rbNode_t *pt);
void insert(rbNode_t **root,rbNode_t *newNode);
void deletion(rbNode_t **root,rbNode_t *deletedNode);
void traversal(rbNode_t root);
void rotateLeft(rbNode_t **root,rbNode_t *pt);
void rotateRight(rbNode_t **root,rbNode_t *pt);
int height(rbNode_t *node);
void printLevelOrder(rbNode_t* root);
void printGivenLevel(rbNode_t* root, int level);
void fixViolationOfDeletion(rbNode_t **root,rbNode_t *pt);
#endif