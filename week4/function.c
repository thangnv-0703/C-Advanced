#ifndef __Function__
#define __Function__
#include "function.h"
#include "data.h"

#include <stdlib.h>
#include <stdio.h>

rbNode_t *createNode(int data){
    rbNode_t *newNode = (rbNode_t *) malloc(sizeof(rbNode_t));
    newNode->data = data;
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->color = RED;
    newNode->parent = NULL;
    return newNode;
}

void rotateLeft(rbNode_t **root,rbNode_t *pt){
    rbNode_t *ptRight = pt->right; // Set the right node of pt

    pt->right = ptRight->left; // make the left of ptRight in to rightChild of the pt node (greater than ....)
    
    if(ptRight->left != NULL) ptRight->left->parent = pt; 

    ptRight->parent = pt->parent; // Change parent of ptRight into parent of pt

    if (pt->parent == NULL)  // Check if the pt is root
        *(root) = ptRight;
    else if(pt == pt->parent->left)
        pt->parent->left = ptRight;
    else pt->parent->right = ptRight; // Put the valid child of parent of pt to the ptRight


    ptRight->left = pt; // Swap position of pt and ptRight
    pt->parent = ptRight;  // pt become leftChild of ptRight
    
}

void rotateRight(rbNode_t **root,rbNode_t *pt){
    rbNode_t *ptLeft = pt->left; // Set the left node of pt

    pt->left = ptLeft->right; // make the right of ptLeft into left child of the pt node (less than ....)

    if(ptLeft->right != NULL) ptLeft->right->parent = pt; 

    ptLeft->parent = pt->parent; // Change parent of ptLeft into parent of pt
    
    if( pt->parent == NULL) *(root) = ptLeft; // Check if the pt is root
    else if(pt == pt->parent->left) pt->parent->left = ptLeft;
    else pt->parent->right = ptLeft; // Put the valid child of parent of pt to the ptRight
    
    ptLeft->right = pt; // Swap position of pt and ptLeft
    pt->parent = ptLeft; //pt become rightChild of ptLeft
    
}

int height(rbNode_t *node) { 
    if (node==NULL) 
        return 0; 
    else
    { 
        /* compute the height of each subtree */
        int lheight = height(node->left); 
        int rheight = height(node->right); 
  
        /* use the larger one */
        if (lheight > rheight) 
            return(lheight+1); 
        else return(rheight+1); 
    } 
}

/* Print nodes at a given level */
void printGivenLevel(rbNode_t* root, int level) { 
    if (root == NULL) 
        return; 
    if (level == 1){
        printf("%d ", root->data);
        if(root->color == BLACK) printf("BLACK\n");
        else printf("RED\n");
    } 
    else if (level > 1) 
    { 
        printGivenLevel(root->left, level-1); 
        printGivenLevel(root->right, level-1); 
    } 
}

void printLevelOrder(rbNode_t* root) 
{ 
    int h = height(root); 
    int i; 
    for (i=1; i<=h; i++) 
        printGivenLevel(root, i); 
} 

void printColor(rbNode_t *node){
    if(node->color == BLACK) printf("BLACK\n"); else printf("RED\n");
}

void fixViolation(rbNode_t **root, rbNode_t *pt){
    rbNode_t *parentPt = NULL;
    rbNode_t *gprPt = NULL;
    rbNode_t *unclePt = NULL;
    rbNode_t *roottemp = *(root);
    while((pt != (*root) ) && (pt->color == RED) && (pt->parent->color == RED)){
        parentPt = pt->parent;
        gprPt = pt->parent->parent;
        if(gprPt->left == parentPt) unclePt = gprPt->right;
        else unclePt = gprPt->left;

        // Case 1: Red-Red-Violation

        if( unclePt != NULL && unclePt->color == RED){
            parentPt->color = BLACK;
            unclePt->color = BLACK;
            gprPt->color = RED;
            pt = gprPt;   
        }else if(gprPt->color == BLACK && pt == parentPt->left && parentPt == gprPt->left){
            parentPt->color = BLACK;
            gprPt->color = RED;
            rotateRight(root,gprPt);
            pt = parentPt;
        }else if( gprPt->color == BLACK && pt == parentPt->right && parentPt == gprPt->left){
            rotateLeft(root,parentPt);
            pt = parentPt;
        }else if( gprPt->color == BLACK && gprPt->right == parentPt && pt == parentPt->right){
            pt->color = BLACK;
            parentPt->color = RED;
            rotateLeft(root,parentPt);
            pt = parentPt;
        }else if (gprPt->color == BLACK && gprPt->right == parentPt && pt == parentPt->left){
            rotateRight(root,parentPt);
            pt = parentPt;
        }

        roottemp = *(root);
        if(roottemp->color == RED) roottemp->color = BLACK;
    }
}

void insert(rbNode_t **root,rbNode_t *newNode){
    rbNode_t *pr,*runNode;
    runNode = *(root);
    
    while(runNode != NULL){
        pr = runNode;
        if(runNode->data < newNode->data)
            runNode = runNode->right;
        else if(runNode->data > newNode->data) runNode = runNode->left;
        else return;
    }

    if(*(root) != NULL) newNode->parent = pr;

    if(*(root) == NULL){
        newNode->color = BLACK;
        *(root) = newNode; 
    }
    else if(newNode->data > pr->data) pr->right = newNode;
    else pr->left = newNode;

    fixViolation(root,newNode);
}

void fixViolationOfDeletion(rbNode_t **root,rbNode_t *pt){
    rbNode_t *siblingNodePt;
    int isLeftChild = 0;
    if(pt->parent->left == pt){
        siblingNodePt = pt->parent->right;
        isLeftChild = 1;
    } else siblingNodePt = pt->parent->left;

    int siblingColor = (siblingNodePt == NULL) ? BLACK : RED;

    if(pt->color == RED) return;
    else if(pt->color == BLACK && siblingColor == RED){
        siblingNodePt->color = BLACK;
        pt->parent->color = RED;
        if(isLeftChild){
            rotateLeft(root,pt->parent);
            siblingNodePt = pt->parent->right;
        }else{
            rotateRight(root,pt->parent);
            siblingNodePt = pt->parent->left;
        }
    }else if(pt->color == BLACK && )
}

void deletion(rbNode_t **root,rbNode_t *deletedNode){
    if(*(root) == NULL) return;
    rbNode_t *runNode = *(root);
    while(runNode->data != deletedNode->data){
        if(deletedNode->data < runNode->data) runNode = runNode->left;
        else if (deletedNode->data > runNode->data) runNode = runNode->right;
    }
    
    rbNode_t *pr = runNode->parent;
    rbNode_t *leftMostRightChild;
    rbNode_t *replaceNode = NULL;

    if(runNode->left == NULL && runNode->right == NULL){
            if(pr != NULL && pr->left == runNode)
                pr->left = NULL;
            else if(pr != NULL && pr->right == runNode)
                pr->right = NULL;
            if(pr == NULL) *(root) = NULL;  
    }else if(runNode->left != NULL && runNode->right != NULL){
        leftMostRightChild = runNode->right;
        while(leftMostRightChild->left != NULL || leftMostRightChild->right != NULL){
            leftMostRightChild = leftMostRightChild->left;
        }
        int flag = leftMostRightChild->parent != runNode;

        if(flag) leftMostRightChild->parent->left = NULL;

        if(pr != NULL && pr->left == runNode) pr->left = leftMostRightChild;
        else if(pr != NULL && pr->right == runNode)pr->right = leftMostRightChild;

        if(flag) leftMostRightChild->right = runNode->right;
        leftMostRightChild->left = runNode->left;
        leftMostRightChild->parent = pr;
        if (flag) runNode->right->parent = leftMostRightChild;

        if(pr == NULL) *(root) = leftMostRightChild;
        else replaceNode = leftMostRightChild;
        //Fix up violation
    }else{
        if(runNode->left != NULL){
            if(pr != NULL){
                if(pr->left == runNode){
                    pr->left = runNode->left;
                    runNode->left->parent = pr;
                }
                else{
                    pr->right = runNode->left;
                    runNode->right->parent = pr;
                }
            }else *(root) = runNode->left;
        }else if(runNode->right != NULL){
            if(pr != NULL){
                if(pr->left == runNode){
                    pr->left = runNode->right;
                    runNode->right->parent = pr;
                }
                else{
                    pr->right = runNode->right;
                    runNode->right->parent = pr;
                }
            }else *(root) = runNode->right;
        }
        
    }
    
    if(pr != NULL) fixViolationOfDeletion(root,deletedNode);
    
    free(runNode);
}
  


#endif