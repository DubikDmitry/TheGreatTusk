#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lodepng.h"

int main(){
    const unsigned char* filename = "Task1.png";
    unsigned weight, height;
    unsigned char* image;
    unsigned error;
    
    error = lodepng_decode32_file(&image, &weight, &height, filename);

    if (error){
        printf("Провал\n");
        return 1;
    }

    printf("weight: %u, height: %u\n", weight, height);
    printf("R G B A \n");
    for (int i = 0; i < weight*height; i ++){
       unsigned char r = image[i*4];
       unsigned char g = image[i*4 + 1];
       unsigned char b = image[i*4 + 2];
       unsigned char a = image[i*4 + 3];
        printf("%u %u %u %u\n", r, g, b, a);
    }
    free(image);
    return 0;
}