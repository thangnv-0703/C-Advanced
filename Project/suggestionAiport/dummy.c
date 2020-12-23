#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../Libfdr/dllist.h"
#include "../Libfdr/jrb.h"
#include "../Libfdr/jval.h"


JRB input(){
    FILE* ptr = fopen("out.dat","r+");
    JRB source = make_jrb();
    char *tmpId = (char*) malloc(sizeof(char));
    int id;
    char *tmp1 = (char*) malloc(sizeof(char));
    char *airPortName = (char*) malloc(sizeof(char)*200);
    char *cityName = (char*) malloc(sizeof(char)*200);
    char *countryName = (char*) malloc(sizeof(char)*200);
    memset(airPortName, '\0', sizeof(airPortName));
    memset(cityName, '\0', sizeof(cityName));
    memset(countryName, '\0', sizeof(countryName));
    while(!feof(ptr)){
        fgets(tmp1, 1000, ptr);
        tmpId = strtok(tmp1, ",");
        airPortName = strtok(NULL, ",");
        cityName = strtok(NULL, ",");
        countryName = strtok(NULL, ",");
        id = atoi(tmpId);
        //printf("%d-%s-%s-%s\n", id,airPortName, cityName, countryName);
        JRB findingNode = jrb_find_str(source,cityName);
        //printf("%d\n", findingNode->key);
        if(findingNode == NULL){
            JRB tree  = make_jrb();
            jrb_insert_int(tree,id,new_jval_s(strdup(airPortName)));
            jrb_insert_str(source, strdup(cityName), new_jval_v(tree));
        }
        else{
            printf("%s\n",cityName);
            JRB listAirports = (JRB)jval_v(findingNode->val);
            jrb_insert_int(listAirports, id, new_jval_s(strdup(airPortName)));
        }
        
    }
    // JRB tmp;
    // jrb_traverse(tmp, source) {
    //     printf("%-40s\n", jval_s(tmp->key));
    //     JRB tree = (JRB) jval_v(tmp->val);
    //     JRB subNode;
    //     jrb_traverse(subNode,tree){
    //         printf("%-40s%-5d%s\n","",jval_i(subNode->key),jval_s(subNode->val));
    //     }
    // }

    fclose(ptr);
    // free(tmpId);
    // free(tmp1);
    // free(airPortName);
    // free(cityName);
    // free(countryName);
    return source;
}
 
int main(){
    JRB testTree = input();
    //JRB findingNode = jrb_find_str(testTree, "Hofn");
    //JRB tmp;
    // jrb_traverse(tmp, testTree) {
    //     printf("%s\n", jval_s(tmp->key));
    // }
    return 0;
}