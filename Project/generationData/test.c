#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define R 6373.0
#define M_PI 3.14159265358979323846
int main(){
    float lat1,lat2,lon1,lon2;
    lat1 = 52.2296756*(M_PI/180);
    lat2 = 52.406374*(M_PI/180);
    lon1 = 21.0122287*(M_PI/180);
    lon2 = 16.9251681*(M_PI/180);

    float dlat,dlon;
    dlat = lat2-lat1;
    dlon = lon2-lon1;
    float a = sin(dlat/2)*sin(dlat/2) + cos(lat2)*cos(lat1)*sin(dlon/2)*sin(dlon/2);
    float c = 2 *atan2(sqrt(a),sqrt(1-a));
    printf("%f\n",R*c);
}