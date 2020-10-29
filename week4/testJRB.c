#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Libfdr/jrb.h"
#include "../Libfdr/jval.h"
#include "../Libfdr/fields.h"

#define MAX_LENGTH_STRING 25

int main(){
   JRB book = make_jrb();
   JRB node;
   int choice;
    do{
        printf("1.Importing Phone Book\n2.Add new phonebook\n3.Delete a phonebook\n4.Modify phonenumber\n5.Print the PhoneBook\n6.Exit\n");
        scanf("%d",&choice);
        switch(choice){
            case 1:
                printf("");
                FILE *ptr = fopen("phonebook.txt","r");
                char *name = (char*) malloc(sizeof(char) * MAX_LENGTH_STRING);
                long *sdt = (long*) malloc(sizeof(long));
                while(fscanf(ptr,"%s %ld\n",name,&sdt) == 2){
                    JRB isExist = jrb_find_str(book,name);
                    if( isExist != NULL){
                        isExist->val = new_jval_l(sdt);
                        sdt = (long*) malloc(sizeof(long)); // Assign sdt to new address
                    }else{
                        jrb_insert_str(book,strdup(name),new_jval_l(sdt));
                        sdt = (long*) malloc(sizeof(long));
                    }
                    
                }
                 // Assign sdt to new address
                fclose(ptr);
                break;
            case 2:
                printf("Enter the new name\n");
                fflush(stdin);
                scanf("%[^\n]s",name);
                printf("Enter the phone number\n");
                fflush(stdin);
                scanf("%ld",&sdt);
                JRB foundNode =  jrb_find_str(book,name);
                if(foundNode != NULL){
                    foundNode->val = new_jval_l(sdt);
                    sdt = (long*) malloc(sizeof(long)); // Assign sdt to new address
                }else
                    jrb_insert_str(book,strdup(name),new_jval_l(sdt));
                    //sdt = (char*) malloc(sizeof(char) *MAX_LENGTH_STRING); // Assign sdt to new address
                break;
            case 3:
                printf("Enter the name key\n");
                fflush(stdin);
                scanf("%[^\n]s",name);
                foundNode = jrb_find_str(book,name);
                jrb_delete_node(foundNode);
                break;
            case 4:
                printf("Enter the name\n");
                fflush(stdin);
                scanf("%[^\n]s",name);
                foundNode = jrb_find_str(book,name);
                if(foundNode != NULL){
                    printf("Enter the phone number\n");
                    fflush(stdin);
                    scanf("%ld",&sdt);
                    foundNode->val = new_jval_l(sdt);
                }else printf("Not Found!!!\n");
                sdt = (long*) malloc(sizeof(long)); // Assign sdt to new address
                break;
            case 5:
                printf("%-25s%s\n","Name","PhoneNumber");
                jrb_traverse(node,book){
                    printf("%s %ld\n",jval_s(node->key),jval_l(node->val));
                }
                break;
        }
    }while(choice != 6);
    
}