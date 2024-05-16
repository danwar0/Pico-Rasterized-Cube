#include <math.h>

#include "pico/stdlib.h"
#include "LCD/LCD_1in3.h"
#include "DEV_Config.h"
#include "Infrared/Infrared.h"

#define PI 3.141592654

#define TRIGSCALE 10000

#define FOCALLENGTH 120

#define TEXTUREWIDTH 10
#define TEXTUREHEIGHT 10

const uint A = 15;
const uint B = 17;
const uint X = 19;
const uint Y = 21;

const uint UP = 2;
const uint DOWN = 18;
const uint LEFT = 16;
const uint RIGHT = 20;

uint8_t RENDERMODE = 1;

typedef struct{
    int x, y;
}ScreenPos;

typedef struct{
    int x, y;
}Pos2D;

typedef struct{
    int x, y, z;
}Pos3D;

typedef struct{
    Pos3D p1, p2, p3;
    Pos3D normal;
    Pos3D faceCentre;
    Pos2D p1t, p2t, p3t;
    UWORD* texture;
    UWORD colour;
}Triangle;

int sinTable[360];
int cosTable[360];

UWORD screenBuffer[57600];

int angleX = 0;
int angleY = 0;

Pos3D lightPos = {0,0,-5000};


UWORD texture1[TEXTUREWIDTH*TEXTUREHEIGHT] = {
    0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0,
    0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000,
    0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0,
    0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000,
    0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0,
    0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000,
    0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0,
    0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000,
    0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0,
    0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000, 0x0ff0, 0xf000,
};

UWORD texture2[TEXTUREWIDTH*TEXTUREHEIGHT] = {
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 
    0xffff, 0xffff, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0x0000, 0xffff, 0xffff, 
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 
    0xffff, 0xffff, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0x0000, 0xffff, 0xffff, 
    0xffff, 0xffff, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff    
};

UWORD texture3[TEXTUREWIDTH*TEXTUREHEIGHT] = {
    0x00f8, 0x00f8, 0xe007, 0xe007, 0x1f00, 0x1f00, 0xffff, 0xffff, 0x1111, 0x1111,
    0x00f8, 0x00f8, 0xe007, 0xe007, 0x1f00, 0x1f00, 0xffff, 0xffff, 0x1111, 0x1111,
    0x00f8, 0x00f8, 0xe007, 0xe007, 0x1f00, 0x1f00, 0xffff, 0xffff, 0x1111, 0x1111,
    0x00f8, 0x00f8, 0xe007, 0xe007, 0x1f00, 0x1f00, 0xffff, 0xffff, 0x1111, 0x1111,
    0x00f8, 0x00f8, 0xe007, 0xe007, 0x1f00, 0x1f00, 0xffff, 0xffff, 0x1111, 0x1111,
    0x00f8, 0x00f8, 0xe007, 0xe007, 0x1f00, 0x1f00, 0xffff, 0xffff, 0x1111, 0x1111,
    0x00f8, 0x00f8, 0xe007, 0xe007, 0x1f00, 0x1f00, 0xffff, 0xffff, 0x1111, 0x1111,
    0x00f8, 0x00f8, 0xe007, 0xe007, 0x1f00, 0x1f00, 0xffff, 0xffff, 0x1111, 0x1111,
    0x00f8, 0x00f8, 0xe007, 0xe007, 0x1f00, 0x1f00, 0xffff, 0xffff, 0x1111, 0x1111,
    0x00f8, 0x00f8, 0xe007, 0xe007, 0x1f00, 0x1f00, 0xffff, 0xffff, 0x1111, 0x1111,
};




Triangle cubeFaces[12] = {
    {{-1000,-1000,-1000},{-1000,1000,-1000},{1000,1000,-1000},{0,0,-1000},{0,0,-1000},{0,TEXTUREHEIGHT},{0,0},{TEXTUREWIDTH,0},texture1,0x00f8}, //Front Face RED
    {{-1000,-1000,-1000},{1000,1000,-1000},{1000,-1000,-1000},{0,0,-1000},{0,0,-1000},{0,TEXTUREHEIGHT},{TEXTUREWIDTH,0},{TEXTUREWIDTH,TEXTUREHEIGHT},texture1,0x00f8},
    {{1000,-1000,1000},{1000,1000,1000},{-1000, 1000,1000},{0,0,1000},{0,0,1000},{0,TEXTUREHEIGHT},{0,0},{TEXTUREWIDTH,0},texture1,0xe007}, //Back Face GREEN
    {{1000,-1000,1000},{-1000,1000,1000},{-1000,-1000,1000},{0,0,1000},{0,0,1000},{0,TEXTUREHEIGHT},{TEXTUREWIDTH,0},{TEXTUREWIDTH,TEXTUREHEIGHT},texture1,0xe007}, 
    {{-1000,-1000,1000},{-1000,1000,1000},{-1000,1000,-1000},{-1000,0,0},{-1000,0,0},{0,TEXTUREHEIGHT},{0,0},{TEXTUREWIDTH,0},texture2,0x1f00}, //Left Face BLUE
    {{-1000,-1000,1000},{-1000,1000,-1000},{-1000,-1000,-1000},{-1000,0,0},{-1000,0,0},{0,TEXTUREHEIGHT},{TEXTUREWIDTH,0},{TEXTUREWIDTH,TEXTUREHEIGHT},texture2,0x1f00},
    {{1000,-1000,-1000},{1000,1000,-1000},{1000,1000,1000},{1000,0,0},{1000,0,0},{0,TEXTUREHEIGHT},{0,0},{TEXTUREWIDTH,0},texture2,0xffff}, //Right Face WHITE
    {{1000,-1000,-1000},{1000,1000,1000},{1000,-1000,1000},{1000,0,0},{1000,0,0},{0,TEXTUREHEIGHT},{TEXTUREWIDTH,0},{TEXTUREWIDTH,TEXTUREHEIGHT},texture2,0xffff},
    {{-1000,-1000,1000},{-1000,-1000,-1000},{1000,-1000,-1000},{0,-1000,0},{0,-1000,0},{0,TEXTUREHEIGHT},{0,0},{TEXTUREWIDTH,0},texture3,0x1ff8}, //Bottom Face PURPLE
    {{-1000,-1000,1000},{1000,-1000,-1000},{1000,-1000,1000},{0,-1000,0},{0,-1000,0},{0,TEXTUREHEIGHT},{TEXTUREWIDTH,0},{TEXTUREWIDTH,TEXTUREHEIGHT},texture3,0x1ff8},
    {{-1000,1000,-1000},{-1000,1000,1000},{1000,1000,1000},{0,1000,0},{0,1000,0},{0,TEXTUREHEIGHT},{0,0},{TEXTUREWIDTH,0},texture3,0x0fff}, //Top Face YELLOW
    {{-1000,1000,-1000},{1000,1000,1000},{1000,1000,-1000},{0,1000,0},{0,1000,0},{0,TEXTUREHEIGHT},{TEXTUREWIDTH,0},{TEXTUREWIDTH,TEXTUREHEIGHT},texture3,0x0fff}
};






int intMin(int v1, int v2){
    return (v1 < v2) ? v1 : v2;
}

int intMax(int v1, int v2){
    return (v1 > v2) ? v1 : v2;
}

Pos2D projectPos3D(Pos3D p){
    Pos2D projected = {p.x * FOCALLENGTH / p.z, p.y * FOCALLENGTH / p.z};
    return projected;
}

Pos3D rotateAroundX(Pos3D p, int angle){
    Pos3D newP = {p.x, p.y*cosTable[angle]/TRIGSCALE-p.z*sinTable[angle]/TRIGSCALE, p.y*sinTable[angle]/TRIGSCALE+p.z*cosTable[angle]/TRIGSCALE};
    return newP;
}

Pos3D rotateAroundY(Pos3D p, int angle){
    Pos3D newP = {p.x*cosTable[angle]/TRIGSCALE-p.z*sinTable[angle]/TRIGSCALE, p.y, p.x*sinTable[angle]/TRIGSCALE+p.z*cosTable[angle]/TRIGSCALE};
    return newP;    
}

int dot(Pos3D a, Pos3D b){
    return a.x*b.x+a.y*b.y+a.z*b.z;
}

Pos3D cross(Pos3D a, Pos3D b){
    Pos3D out = {a.y*b.z-a.z*b.y, -1*(a.x*b.z-a.z*b.x), a.x*b.y-a.y*b.x};
    return out;    
}

void drawPoint(ScreenPos p, UWORD colour){
    if(p.x < 0 || p.y < 0 || p.x >= 240 || p.y >= 240){
        return;
    }
    screenBuffer[p.y*240+p.x] = colour;
}

void drawLine(ScreenPos p1, ScreenPos p2){
    int xDiff = p2.x - p1.x;
    int yDiff = p2.y - p1.y;

    ScreenPos currentPos = p1;
    if(abs(xDiff) > abs(yDiff)){
        //draw along x
        int grad = yDiff*10000/xDiff;
        for(int i = 0; i < abs(xDiff); i++){
            drawPoint(currentPos, 0xffff);
            currentPos.x = p1.x + i*(xDiff > 0 ? 1 : -1);
            currentPos.y = p1.y + grad*i*(xDiff > 0 ? 1 : -1)/10000;
        }
    }else{
        //draw along y
        int grad = xDiff*10000/yDiff;
        for(int i = 0; i < abs(yDiff); i++){
            drawPoint(currentPos, 0xffff);
            currentPos.y = p1.y + i*(yDiff > 0 ? 1 : -1);
            currentPos.x = p1.x + grad*i*(yDiff > 0 ? 1 : -1)/10000;
        }
    }

}





void fillTriangle(ScreenPos p1, ScreenPos p2, ScreenPos p3, UWORD colour){
    //Sort triangle coordinates by y
    ScreenPos first = p1;
    ScreenPos second = p2;
    ScreenPos third = p3;
    if(second.y < first.y){
        ScreenPos prevFirst = first;
        ScreenPos prevSecond = second;
        first = prevSecond;
        second = prevFirst;
    }
    if(third.y < first.y){
        ScreenPos prevFirst = first;
        ScreenPos prevThird = third;
        first = prevThird;
        third = prevFirst;
    }
    if(third.y < second.y){
        ScreenPos prevSecond = second;
        ScreenPos prevThird = third;
        second = prevThird;
        third = prevSecond;
    }

    int part1Height = second.y - first.y;
    int part2Height = third.y - second.y;
    int totalHeight = third.y - first.y;

    int gradAB = (second.x-first.x)*10000/part1Height;
    int gradAC = (third.x-first.x)*10000/totalHeight;
    int gradBC = (third.x-second.x)*10000/part2Height;

    int x1, x2;
    int minX, maxX;
    int xDistance;

    //part 1
    for(int i = 0; i < part1Height; i++){
        x1 = first.x + i*gradAB/10000;
        x2 = first.x + i*gradAC/10000;
        minX = intMin(x1,x2);
        maxX = intMax(x1,x2);
        xDistance = maxX-minX;
        for(int j = 0; j < xDistance+1; j++){
            ScreenPos p = {minX+j, first.y+i};
            drawPoint(p, colour);
        }
    }

    //part 2
    for(int i = 0; i < part2Height; i++){
        x1 = first.x + part1Height*gradAC/10000 + i*gradAC/10000;
        x2 = second.x + i*gradBC/10000;
        minX = intMin(x1,x2);
        maxX = intMax(x1,x2);
        xDistance = maxX-minX;
        for(int j = 0; j < xDistance+1; j++){
            ScreenPos p = {minX+j, second.y+i};
            drawPoint(p, colour);
        }
    }

    return;
}


int interpolate(int i1, int i2, int offset, int thing){
    return offset+thing*i1 / i2;
}

UWORD sampleTexture(UWORD* texture, int x, int y){
    if(x < 0 || y < 0 || x > TEXTUREWIDTH || y > TEXTUREHEIGHT){
        return 0;
    }
    int arrX = x;
    int arrY = y;
    if(arrX == TEXTUREWIDTH){
        arrX -= 1;
    }
    if(arrY == TEXTUREHEIGHT){
        arrY -= 1;
    }
    return texture[arrY*TEXTUREWIDTH+arrX];
}


void fillTexturedTriangle(ScreenPos p1, ScreenPos p2, ScreenPos p3, UWORD* texture, int dotLN, int bothMagnitude, int p1z, int p2z, int p3z, Pos2D p1t, Pos2D p2t, Pos2D p3t){
    //Sort triangle coordinates by y
    Pos2D firstT = p1t;
    Pos2D secondT = p2t;
    Pos2D thirdT = p3t;
    int firstZ = p1z;
    int secondZ = p2z;
    int thirdZ = p3z;
    ScreenPos first = p1;
    ScreenPos second = p2;
    ScreenPos third = p3;
    if(second.y < first.y){
        //swap second and first
        ScreenPos prevFirst = first;
        ScreenPos prevSecond = second;
        first = prevSecond;
        second = prevFirst;

        Pos2D prevFirstT = firstT;
        Pos2D prevSecondT = secondT;
        firstT = prevSecondT;
        secondT = prevFirstT;

        int prevFirstZ = firstZ;
        int prevSecondZ = secondZ;
        firstZ = prevSecondZ;
        secondZ = prevFirstZ;
    }
    if(third.y < first.y){
        //swap third and first
        ScreenPos prevFirst = first;
        ScreenPos prevThird = third;
        first = prevThird;
        third = prevFirst;

        Pos2D prevFirstT = firstT;
        Pos2D prevThirdT = thirdT;
        firstT = prevThirdT;
        thirdT = prevFirstT;

        int prevFirstZ = firstZ;
        int prevThirdZ = thirdZ;
        firstZ = prevThirdZ;
        thirdZ = prevFirstZ;
    }
    if(third.y < second.y){
        //swap third and second
        ScreenPos prevSecond = second;
        ScreenPos prevThird = third;
        second = prevThird;
        third = prevSecond;

        Pos2D prevSecondT = secondT;
        Pos2D prevThirdT = thirdT;
        secondT = prevThirdT;
        thirdT = prevSecondT;

        int prevSecondZ = secondZ;
        int prevThirdZ = thirdZ;
        secondZ = prevThirdZ;
        thirdZ = prevSecondZ;
    }

    int part1Height = second.y - first.y;
    int part2Height = third.y - second.y;
    int totalHeight = third.y - first.y;

    int gradAB = (second.x-first.x)*10000/part1Height;
    int gradAC = (third.x-first.x)*10000/totalHeight;
    int gradBC = (third.x-second.x)*10000/part2Height;

    int x1, x2;
    int minX, maxX;
    int xDistance;

    //texturing
    int oneOverZ1 = 10000000/firstZ;
    int oneOverZ2 = 10000000/secondZ;
    int oneOverZ3 = 10000000/thirdZ;

    int uOverZ1 = 10000000*firstT.x/firstZ;
    int uOverZ2 = 10000000*secondT.x/secondZ;
    int uOverZ3 = 10000000*thirdT.x/thirdZ;

    int vOverZ1 = 10000000*firstT.y/firstZ;
    int vOverZ2 = 10000000*secondT.y/secondZ;
    int vOverZ3 = 10000000*thirdT.y/thirdZ;

    int gradOneOverZ1Z2 = (oneOverZ2 - oneOverZ1) / part1Height;
    int gradOneOverZ1Z3 = (oneOverZ3 - oneOverZ1) / totalHeight;
    int gradOneOverZ2Z3 = (oneOverZ3 - oneOverZ2) / part2Height;

    int gradUOverZ1Z2 = (uOverZ2 - uOverZ1) / part1Height;
    int gradUOverZ1Z3 = (uOverZ3 - uOverZ1) / totalHeight;
    int gradUOverZ2Z3 = (uOverZ3 - uOverZ2) / part2Height;

    int gradVOverZ1Z2 = (vOverZ2 - vOverZ1) / part1Height;
    int gradVOverZ1Z3 = (vOverZ3 - vOverZ1) / totalHeight;
    int gradVOverZ2Z3 = (vOverZ3 - vOverZ2) / part2Height;

    int z1, z2, u1, u2, v1, v2;
    int minZ, maxZ, minU, maxU, minV, maxV;
    int z, u, v;
    int sampleX, sampleY;

    //part 1
    for(int i = 0; i < part1Height; i++){
        //texturing
        z1 = oneOverZ1 + i*gradOneOverZ1Z2;
        z2 = oneOverZ1 + i*gradOneOverZ1Z3;
        u1 = uOverZ1 + i*gradUOverZ1Z2;
        u2 = uOverZ1 + i*gradUOverZ1Z3;
        v1 = vOverZ1 + i*gradVOverZ1Z2;
        v2 = vOverZ1 + i*gradVOverZ1Z3;
        //
        x1 = first.x + i*gradAB/10000;
        x2 = first.x + i*gradAC/10000;
        if(x1<x2){
            minX = x1;
            maxX = x2;
            minZ = z1;
            maxZ = z2;
            minU = u1;
            maxU = u2;
            minV = v1;
            maxV = v2;
        }else{
            minX = x2;
            maxX = x1;
            minZ = z2;
            maxZ = z1;
            minU = u2;
            maxU = u1;
            minV = v2;
            maxV = v1;
        }
        xDistance = maxX-minX;
        for(int j = 0; j < xDistance+1; j++){
            ScreenPos p = {minX+j, first.y+i};
            z = interpolate(j, xDistance+1, minZ, maxZ-minZ);
            u = interpolate(j, xDistance+1, minU, maxU-minU);
            v = interpolate(j, xDistance+1, minV, maxV-minV);
            sampleX = u/z;
            sampleY = v/z;
            UWORD col = sampleTexture(texture, sampleX, sampleY);

            int red = (col & 0b0000000011111000) >> 3;
            int green = (col & 0b111) << 3 | (col & 0b1110000000000000) >> 10;
            int blue = (col & 0b0001111100000000) >> 8;

            red = red * dotLN / bothMagnitude;
            if(red <= 0){
                red = 1;
            }
            green = green * dotLN / bothMagnitude;
            if(green <= 0){
                green = 1;
            }
            blue = blue * dotLN / bothMagnitude;
            if(blue <= 0){
                blue = 1;
            }

            col = 0b0000000000000000 | (red & 0b11111)<<3 | (blue & 0b11111)<<8 | (green & 0b111)<<13 | (green & 0b111000)>>3;

            drawPoint(p, col);
        }
    }

    //part 2
    for(int i = 0; i < part2Height; i++){
        //texturing
        z1 = oneOverZ1 + part1Height*gradOneOverZ1Z3 + i*gradOneOverZ1Z3;
        z2 = oneOverZ2 + i*gradOneOverZ2Z3;
        u1 = uOverZ1 + part1Height*gradUOverZ1Z3 + i*gradUOverZ1Z3;
        u2 = uOverZ2 + i*gradUOverZ2Z3;
        v1 = vOverZ1 + part1Height*gradVOverZ1Z3 + i*gradVOverZ1Z3;
        v2 = vOverZ2 + i*gradVOverZ2Z3;
        //
        x1 = first.x + part1Height*gradAC/10000 + i*gradAC/10000;
        x2 = second.x + i*gradBC/10000;
        if(x1<x2){
            minX = x1;
            maxX = x2;
            minZ = z1;
            maxZ = z2;
            minU = u1;
            maxU = u2;
            minV = v1;
            maxV = v2;
        }else{
            minX = x2;
            maxX = x1;
            minZ = z2;
            maxZ = z1;
            minU = u2;
            maxU = u1;
            minV = v2;
            maxV = v1;
        }
        xDistance = maxX-minX;
        for(int j = 0; j < xDistance+1; j++){
            ScreenPos p = {minX+j, second.y+i};
            z = interpolate(j, xDistance+1, minZ, maxZ-minZ);
            u = interpolate(j, xDistance+1, minU, maxU-minU);
            v = interpolate(j, xDistance+1, minV, maxV-minV);
            sampleX = u/z;
            sampleY = v/z;
            UWORD col = sampleTexture(texture, sampleX, sampleY);
            
            int red = (col & 0b0000000011111000) >> 3;
            int green = (col & 0b111) << 3 | (col & 0b1110000000000000) >> 10;
            int blue = (col & 0b0001111100000000) >> 8;

            red = red * dotLN / bothMagnitude;
            if(red <= 0){
                red = 1;
            }
            green = green * dotLN / bothMagnitude;
            if(green <= 0){
                green = 1;
            }
            blue = blue * dotLN / bothMagnitude;
            if(blue <= 0){
                blue = 1;
            }

            col = 0b0000000000000000 | (red & 0b11111)<<3 | (blue & 0b11111)<<8 | (green & 0b111)<<13 | (green & 0b111000)>>3;

            drawPoint(p, col);
        }
    }

    return;

}





void renderCubeWireframe(){
    for(int i = 0; i < 12; i++){
        Triangle currentTriangle = cubeFaces[i];
        
        Pos3D p1Rotated = currentTriangle.p1;
        Pos3D p2Rotated = currentTriangle.p2;
        Pos3D p3Rotated = currentTriangle.p3;
        p1Rotated = rotateAroundX(p1Rotated,angleX);
        p2Rotated = rotateAroundX(p2Rotated,angleX);
        p3Rotated = rotateAroundX(p3Rotated,angleX);
        p1Rotated = rotateAroundY(p1Rotated,angleY);
        p2Rotated = rotateAroundY(p2Rotated,angleY);
        p3Rotated = rotateAroundY(p3Rotated,angleY);

        p1Rotated.z += 3000;
        p2Rotated.z += 3000;
        p3Rotated.z += 3000;

        Pos2D p1Projected = projectPos3D(p1Rotated);
        Pos2D p2Projected = projectPos3D(p2Rotated);
        Pos2D p3Projected = projectPos3D(p3Rotated);

        ScreenPos screenPos1 = {p1Projected.x + 120, p1Projected.y*-1+120};
        ScreenPos screenPos2 = {p2Projected.x + 120, p2Projected.y*-1+120};
        ScreenPos screenPos3 = {p3Projected.x + 120, p3Projected.y*-1+120};

        drawLine(screenPos1, screenPos2);
        drawLine(screenPos2, screenPos3);
        drawLine(screenPos3, screenPos1);
    }

    return;
}


void renderCubeWireframeCulled(){
    for(int i = 0; i < 12; i++){
        Triangle currentTriangle = cubeFaces[i];
        
        Pos3D p1Rotated = currentTriangle.p1;
        Pos3D p2Rotated = currentTriangle.p2;
        Pos3D p3Rotated = currentTriangle.p3;
        Pos3D normal = currentTriangle.normal;

        p1Rotated = rotateAroundX(p1Rotated,angleX);
        p2Rotated = rotateAroundX(p2Rotated,angleX);
        p3Rotated = rotateAroundX(p3Rotated,angleX);
        p1Rotated = rotateAroundY(p1Rotated,angleY);
        p2Rotated = rotateAroundY(p2Rotated,angleY);
        p3Rotated = rotateAroundY(p3Rotated,angleY);

        p1Rotated.z += 3000;
        p2Rotated.z += 3000;
        p3Rotated.z += 3000;

        normal = rotateAroundX(normal,angleX);
        normal = rotateAroundY(normal,angleY);

        Pos3D centre = {(p1Rotated.x+p2Rotated.x+p3Rotated.x/3),(p1Rotated.y+p2Rotated.y+p3Rotated.y/3),(p1Rotated.z+p2Rotated.z+p3Rotated.z/3)};
        Pos3D oc = centre;        

        if(dot(normal, oc) < 0){
            Pos2D p1Projected = projectPos3D(p1Rotated);
            Pos2D p2Projected = projectPos3D(p2Rotated);
            Pos2D p3Projected = projectPos3D(p3Rotated);

            ScreenPos screenPos1 = {p1Projected.x + 120, p1Projected.y*-1+120};
            ScreenPos screenPos2 = {p2Projected.x + 120, p2Projected.y*-1+120};
            ScreenPos screenPos3 = {p3Projected.x + 120, p3Projected.y*-1+120};

            drawLine(screenPos1, screenPos2);
            drawLine(screenPos2, screenPos3);
            drawLine(screenPos3, screenPos1);
        }
    }

    return;
}


void renderCubeFilled(){
    for(int i = 0; i < 12; i++){
        Triangle currentTriangle = cubeFaces[i];
        
        Pos3D p1Rotated = currentTriangle.p1;
        Pos3D p2Rotated = currentTriangle.p2;
        Pos3D p3Rotated = currentTriangle.p3;
        Pos3D normal = currentTriangle.normal;

        p1Rotated = rotateAroundX(p1Rotated,angleX);
        p2Rotated = rotateAroundX(p2Rotated,angleX);
        p3Rotated = rotateAroundX(p3Rotated,angleX);
        p1Rotated = rotateAroundY(p1Rotated,angleY);
        p2Rotated = rotateAroundY(p2Rotated,angleY);
        p3Rotated = rotateAroundY(p3Rotated,angleY);

        p1Rotated.z += 3000;
        p2Rotated.z += 3000;
        p3Rotated.z += 3000;

        normal = rotateAroundX(normal,angleX);
        normal = rotateAroundY(normal,angleY);

        Pos3D centre = {(p1Rotated.x+p2Rotated.x+p3Rotated.x/3),(p1Rotated.y+p2Rotated.y+p3Rotated.y/3),(p1Rotated.z+p2Rotated.z+p3Rotated.z/3)};
        Pos3D oc = centre;

        if(dot(normal, oc) < 0){
            Pos2D p1Projected = projectPos3D(p1Rotated);
            Pos2D p2Projected = projectPos3D(p2Rotated);
            Pos2D p3Projected = projectPos3D(p3Rotated);

            ScreenPos screenPos1 = {p1Projected.x + 120, p1Projected.y*-1+120};
            ScreenPos screenPos2 = {p2Projected.x + 120, p2Projected.y*-1+120};
            ScreenPos screenPos3 = {p3Projected.x + 120, p3Projected.y*-1+120};

            fillTriangle(screenPos1, screenPos2, screenPos3, currentTriangle.colour);
        }
    }

    return;
}


void renderCubeLit(){
    for(int i = 0; i < 12; i++){
        Triangle currentTriangle = cubeFaces[i];
        
        Pos3D p1Rotated = currentTriangle.p1;
        Pos3D p2Rotated = currentTriangle.p2;
        Pos3D p3Rotated = currentTriangle.p3;
        Pos3D normal = currentTriangle.normal;
        Pos3D centre = currentTriangle.faceCentre;

        p1Rotated = rotateAroundX(p1Rotated,angleX);
        p2Rotated = rotateAroundX(p2Rotated,angleX);
        p3Rotated = rotateAroundX(p3Rotated,angleX);
        p1Rotated = rotateAroundY(p1Rotated,angleY);
        p2Rotated = rotateAroundY(p2Rotated,angleY);
        p3Rotated = rotateAroundY(p3Rotated,angleY);

        centre = rotateAroundX(centre,angleX);
        centre = rotateAroundY(centre,angleY);

        p1Rotated.z += 3000;
        p2Rotated.z += 3000;
        p3Rotated.z += 3000;
        centre.z += 3000;

        normal = rotateAroundX(normal,angleX);
        normal = rotateAroundY(normal,angleY);

        Pos3D oc = centre;

        if(dot(normal, oc) < 0){
            Pos2D p1Projected = projectPos3D(p1Rotated);
            Pos2D p2Projected = projectPos3D(p2Rotated);
            Pos2D p3Projected = projectPos3D(p3Rotated);

            ScreenPos screenPos1 = {p1Projected.x + 120, p1Projected.y*-1+120};
            ScreenPos screenPos2 = {p2Projected.x + 120, p2Projected.y*-1+120};
            ScreenPos screenPos3 = {p3Projected.x + 120, p3Projected.y*-1+120};

            Pos3D cl = {lightPos.x - centre.x, lightPos.y - centre.y, lightPos.z - centre.z};
            int dotLN = dot(normal, cl);

            int normalLength = 1000;
            int clLength = sqrt(cl.x*cl.x + cl.y*cl.y + cl.z*cl.z);
            int both = normalLength*clLength;

            int red = (currentTriangle.colour & 0b0000000011111000) >> 3;
            int green = (currentTriangle.colour & 0b111) << 3 | (currentTriangle.colour & 0b1110000000000000) >> 10;
            int blue = (currentTriangle.colour & 0b0001111100000000) >> 8;

            red = red * dotLN / both;
            if(red <= 0){
                red = 1;
            }
            green = green * dotLN / both;
            if(green <= 0){
                green = 1;
            }
            blue = blue * dotLN / both;
            if(blue <= 0){
                blue = 1;
            }

            UWORD col = 0b0000000000000000 | (red & 0b11111)<<3 | (blue & 0b11111)<<8 | (green & 0b111)<<13 | (green & 0b111000)>>3;

            fillTriangle(screenPos1, screenPos2, screenPos3, col);
        }
    }

    return;
}


void renderCubeTextured(){
        for(int i = 0; i < 12; i++){
        Triangle currentTriangle = cubeFaces[i];
        
        Pos3D p1Rotated = currentTriangle.p1;
        Pos3D p2Rotated = currentTriangle.p2;
        Pos3D p3Rotated = currentTriangle.p3;
        Pos3D normal = currentTriangle.normal;
        Pos3D centre = currentTriangle.faceCentre;

        p1Rotated = rotateAroundX(p1Rotated,angleX);
        p2Rotated = rotateAroundX(p2Rotated,angleX);
        p3Rotated = rotateAroundX(p3Rotated,angleX);
        p1Rotated = rotateAroundY(p1Rotated,angleY);
        p2Rotated = rotateAroundY(p2Rotated,angleY);
        p3Rotated = rotateAroundY(p3Rotated,angleY);

        centre = rotateAroundX(centre,angleX);
        centre = rotateAroundY(centre,angleY);

        p1Rotated.z += 3000;
        p2Rotated.z += 3000;
        p3Rotated.z += 3000;
        centre.z += 3000;

        normal = rotateAroundX(normal,angleX);
        normal = rotateAroundY(normal,angleY);

        Pos3D oc = centre;

        if(dot(normal, oc) < 0){
            Pos2D p1Projected = projectPos3D(p1Rotated);
            Pos2D p2Projected = projectPos3D(p2Rotated);
            Pos2D p3Projected = projectPos3D(p3Rotated);

            ScreenPos screenPos1 = {p1Projected.x + 120, p1Projected.y*-1+120};
            ScreenPos screenPos2 = {p2Projected.x + 120, p2Projected.y*-1+120};
            ScreenPos screenPos3 = {p3Projected.x + 120, p3Projected.y*-1+120};

            Pos3D cl = {lightPos.x - centre.x, lightPos.y - centre.y, lightPos.z - centre.z};
            int dotLN = dot(normal, cl);

            int normalLength = 1000;
            int clLength = sqrt(cl.x*cl.x + cl.y*cl.y + cl.z*cl.z);
            int both = normalLength*clLength;

            fillTexturedTriangle(screenPos1, screenPos2, screenPos3, currentTriangle.texture, dotLN, both, p1Rotated.z, p2Rotated.z, p3Rotated.z, currentTriangle.p1t, currentTriangle.p2t, currentTriangle.p3t);
        }
    }

    return;
}









int main(){
    stdio_init_all();
    DEV_Module_Init();
    LCD_1IN3_Init(HORIZONTAL);

    //Trig Tables
    for(int i = 0; i < 360; i++){
        sinTable[i] = (int)(sin(i / 180.0 * PI) * TRIGSCALE);
        cosTable[i] = (int)(cos(i / 180.0 * PI) * TRIGSCALE);
    }

    SET_Infrared_PIN(A);
    SET_Infrared_PIN(B);
    SET_Infrared_PIN(X);
    SET_Infrared_PIN(Y);

    SET_Infrared_PIN(UP);
    SET_Infrared_PIN(DOWN);
    SET_Infrared_PIN(LEFT);
    SET_Infrared_PIN(RIGHT);

    LCD_1IN3_Clear(0x0000);

    while(1){
        if(DEV_Digital_Read(A) == 0){
            RENDERMODE = 1; //wireframe
        }
        if(DEV_Digital_Read(B) == 0){
            RENDERMODE = 2; //wireframe with backface culling
        }
        if(DEV_Digital_Read(X) == 0){
            RENDERMODE = 3; //filled
        }
        if(DEV_Digital_Read(Y) == 0){
            RENDERMODE = 4; //lit
        }
        if(DEV_Digital_Read(UP) == 0){
            RENDERMODE = 5; //textured
        }

        angleY += 5;
        angleX += 3;

        if(angleX >= 360){
            angleX = 0;
        }
        if(angleY >= 360){
            angleY = 0;
        }

        //Clear Screen
        for(int i = 0; i < 57600; i++){
            screenBuffer[i] = 0;
        }
        
        //Draw Screen
        if(RENDERMODE == 2){
            renderCubeWireframeCulled();
        }else if(RENDERMODE == 3){
            renderCubeFilled();
        }else if(RENDERMODE == 4){
            renderCubeLit();
        }else if(RENDERMODE == 5){
            renderCubeTextured();
        }else{
            renderCubeWireframe();
        }

        //Show Screen
        LCD_1IN3_Display(screenBuffer);
    }



    return 0;
}