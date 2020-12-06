#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#define R 6373.0
#define M_PI 3.14159265358979323846
float distance(float lat1,float lat2,float lon1,float lon2){
    // lat1 = lat1*(M_PI/180);
    // lat2 = lat2*(M_PI/180);
    // lon1 = lon1*(M_PI/180);
    // lon2 = lon2*(M_PI/180);
    float dlat,dlon;
    dlat = lat2-lat1;
    dlon = lon2-lon1;
    //printf("%f %f %f %f\n",lat1,lat2,dlat,dlon);
    float a = sin(dlat/2)*sin(dlat/2) + cos(lat2)*cos(lat1)*sin(dlon/2)*sin(dlon/2);
    float c = 2 *atan2(sqrt(a),sqrt(1-a));
    return c*R;
}
int main(){
    FILE *ptr = fopen("Book1.csv","r+");
    FILE *ptr2 = fopen("distance.txt","w+");
    float lat1,lat2,lon1,lon2;
    int idSource,idDes;
    char sourceAir[30],DesAir[30];
    //printf("%d\n",ptr,"%s,%d,%s,%d,%f,%f,%f,%f\n",sourceAir,&idSource,DesAir,&idDes,&lat1,&lon1,&lat2,&lon2);
    while(!feof(ptr)){
        fscanf(ptr,"%s %d %s %d %f %f %f %f\n",sourceAir,&idSource,DesAir,&idDes,&lat1,&lon1,&lat2,&lon2);
        if(strcmp(sourceAir,"#N/A") != 0 && strcmp(sourceAir,"\\N") ) fprintf(ptr2,"%s %d %s %d %f %f %f %f %f\n",sourceAir,idSource,DesAir,idDes,lat1,lon1,lat2,lon2,distance(lat1,lat2,lon1,lon2));
        //printf("%f %f %f %f\n",lat1,lon1,lat2,lon2);
    }
    fclose(ptr);
    fclose(ptr2);
}