#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;

typedef struct tagBITMAPFILEHEADER {
    WORD bfType;  //specifies the file type
    DWORD bfSize;  //specifies the size in bytes of the bitmap file
    WORD bfReserved1;  //reserved; must be 0
    WORD bfReserved2;  //reserved; must be 0
    DWORD bfOffBits;  //species the offset in bytes from the bitmapfileheader tothe bitmap bits
} BMPFileHeader;

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;  //specifies the number of bytes required by the struct       
    LONG biWidth;  //specifies width in pixels
    LONG biHeight;  //species height in pixels
    WORD biPlanes; //specifies the number of color planes, must be 1
    WORD biBitCount; //specifies the number of bit per pixel 
    DWORD biCompression;//spcifies the type of compression
    DWORD biSizeImage; //size of image in bytes
    LONG biXPelsPerMeter; //number of pixels per meter in x axis 
    LONG biYPelsPerMeter; //number of pixels per meter in y axis 
    DWORD biClrUsed; //number of colors used by th ebitmap        
    DWORD biClrImportant; //number of colors that are important 
} BMPInfoHeader;

int checkInput(int argc, char **argv) {
    if (argc != 5)
        return 0;
    if (argv[3][0]!= '0' && argv[3][0]!='.' && argv[3][0] !='1') {
        printf("Bad ratio.\n");
        return 0;
    }
    double ratio = atof(argv[3]);
    if (ratio > 1 || ratio < 0)
        return 0;
    return 1;
}

void readFileHeader(BMPFileHeader *imgPointer, FILE *filePointer) {
    fread(&(imgPointer->bfType), sizeof(WORD), 1, filePointer);
    fread(&(imgPointer->bfSize), sizeof(DWORD), 1, filePointer);
    fread(&(imgPointer->bfReserved1), sizeof(WORD), 1, filePointer);
    fread(&(imgPointer->bfReserved2), sizeof(WORD), 1, filePointer);
    fread(&(imgPointer->bfOffBits), sizeof(DWORD), 1, filePointer);
}

void readFileInfoHeader(BMPInfoHeader *imgPointer, FILE *filePointer) {
    fread(&(imgPointer->biSize), sizeof(DWORD), 1, filePointer);
    fread(&(imgPointer->biWidth), sizeof(LONG), 1, filePointer);
    fread(&(imgPointer->biHeight), sizeof(LONG), 1, filePointer);
    fread(&(imgPointer->biPlanes), sizeof(WORD), 1, filePointer);
    fread(&(imgPointer->biBitCount), sizeof(WORD), 1, filePointer);
    fread(&(imgPointer->biCompression), sizeof(DWORD), 1, filePointer);
    fread(&(imgPointer->biSizeImage), sizeof(DWORD), 1, filePointer);
    fread(&(imgPointer->biXPelsPerMeter), sizeof(LONG), 1, filePointer);
    fread(&(imgPointer->biYPelsPerMeter), sizeof(LONG), 1, filePointer);
    fread(&(imgPointer->biClrUsed), sizeof(DWORD), 1, filePointer);
    fread(&(imgPointer->biClrImportant), sizeof(DWORD), 1, filePointer);
}   

void readImage(BYTE *imgData, FILE *filePointer, int imgSize, int byteOffset) {
    fseek(filePointer, byteOffset, SEEK_SET);
    fread(imgData, imgSize, 1, filePointer);
}

void writeHeader(FILE *outPtr, BMPFileHeader *fH, BMPInfoHeader *fIH) {
    fseek(outPtr, 0, SEEK_SET);
    fwrite(&(fH->bfType), 1, sizeof(WORD), outPtr);
    fseek(outPtr, 2, SEEK_SET);
    fwrite(&(fH->bfSize), 1, sizeof(DWORD), outPtr);
    fseek(outPtr, 6, SEEK_SET);
    fwrite(&(fH->bfReserved1), 1, sizeof(WORD), outPtr);
    fseek(outPtr, 8, SEEK_SET);
    fwrite(&(fH->bfReserved2), 1, sizeof(WORD), outPtr);
    fseek(outPtr, 10, SEEK_SET);
    fwrite(&(fH->bfOffBits), 1, sizeof(DWORD), outPtr);
    fseek(outPtr, 14, SEEK_SET);
    fwrite(fIH, 1, fIH->biSize, outPtr);  
}

BYTE getColor(BYTE *imageData, int x, int y, int imagewidth, int imageheight, int colorOffset) {
    int padding = (imagewidth * 3) % 4;
    int bytesPerLine = imagewidth * 3;
    if (padding != 0) {
        bytesPerLine = bytesPerLine + 4 - padding; 
    }
    BYTE pix = imageData[x*3 + y*bytesPerLine + colorOffset];
    return pix;
}

BYTE calculateColor(unsigned char *imageData, float x, float y, int imageWidth, int imageHeight, int colorOffset) {
    int padding = (imageWidth * 3) % 4; 
    int x1 = (int) x;
    int x2 = x1 + 1;
    x2 = (x2 > imageWidth) ? imageWidth : x2;
    float dx = x - x1;
    int y1 = (int) y;
    int y2 = y1 + 1;
    y2 = (y2 > imageHeight) ? imageHeight : y2;
    float dy = y - y1;

    // Grabbing 4 Pixels' specified color (dependent on colorOffset)
    BYTE x1y1Col = getColor(imageData, x1, y1, imageWidth, imageHeight, colorOffset);
    BYTE x1y2Col = getColor(imageData, x1, y2, imageWidth, imageHeight, colorOffset);
    BYTE x2y1Col = getColor(imageData, x2, y1, imageWidth, imageHeight, colorOffset);
    BYTE x2y2Col = getColor(imageData, x2, y2, imageWidth, imageHeight, colorOffset);

    //Bilinear Interpollation Step 
    BYTE topXCol = x1y1Col * (1 - dx) + x2y1Col * dx;
    BYTE bottomXCol = x1y2Col * (1 - dx) + x2y2Col * dx;
    BYTE finalCol = topXCol * (1 - dy) + bottomXCol * dy;

    return finalCol;
}

void createImage(BYTE *outData, BYTE *largerImg, BYTE *smallerImg, float ratio, int width, int height, int width2, int height2, float xRatio, float yRatio) {
    int x, y;
    int bytesPerLine = width * 3;
    int padding = (width * 3) % 4;
    if (padding != 0)
        bytesPerLine = bytesPerLine + 4 - padding;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            BYTE redFinal, greenFinal, blueFinal;

            //Read Larger Pixel's Color
            BYTE red1 = getColor(largerImg, x, y, width, height, 0);
            BYTE green1 = getColor(largerImg, x, y, width, height, 1);
            BYTE blue1 = getColor(largerImg, x, y, width, height, 2);

            //Calculate Smaller Pixel
            float smallX = x * xRatio;
            float smallY = y * yRatio;
            BYTE red2 = calculateColor(smallerImg, smallX, smallY, width2, height2, 0);
            BYTE green2 = calculateColor(smallerImg, smallX, smallY, width2, height2, 1);
            BYTE blue2 = calculateColor(smallerImg, smallX, smallY, width2, height2, 2);
            
            //Calculate Average Color
            redFinal = red1 * ratio + red2 * (1 - ratio);
            greenFinal = green1 * ratio + green2 * (1 - ratio);
            blueFinal = blue1 * ratio + blue2 * (1 - ratio);
            
            outData[x*3 + y*bytesPerLine + 0] = redFinal;
            outData[x*3 + y*bytesPerLine + 1] = greenFinal;
            outData[x*3 + y*bytesPerLine + 2] = blueFinal;
        }
    }
}

int main(int argc, char *argv[20] ) {
    if (checkInput(argc, argv) == 0) {
        printf("Invalid input.\n");
        printf("Please format your input as the following:\n\n");
        printf("[programname] [imagefile1] [imagefile2] [ratio] [outputfile]\n\n");
        printf("Example:\nblendimages face.bmp butterfly.bmp 0.3 merged.bmp\n\n");
        return -1;
    }

    //Initialize ratio command line argument
    float ratio;
    ratio = atof(argv[3]);

    //File 1 Input
    BMPFileHeader *img1Header = (BMPFileHeader *) malloc(sizeof(BMPFileHeader));
    BMPInfoHeader *img1InfoHeader = (BMPInfoHeader *) malloc(sizeof(BMPInfoHeader));
    FILE *img1Ptr;
    if ((img1Ptr = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening the first input file.\n");
        return -1;
    }
    readFileHeader(img1Header, img1Ptr);
    readFileInfoHeader(img1InfoHeader, img1Ptr);
    int img1Size = img1InfoHeader->biSizeImage;
    BYTE *img1Data;
    img1Data = (BYTE *)malloc(img1Size);
    readImage(img1Data, img1Ptr, img1Size, img1Header->bfOffBits);
    

    //File 2 Input
    BMPFileHeader *img2Header = (BMPFileHeader *) malloc(sizeof(BMPFileHeader));
    BMPInfoHeader *img2InfoHeader = (BMPInfoHeader *) malloc(sizeof(BMPInfoHeader));
    FILE *img2Ptr;
    if ((img2Ptr = fopen(argv[2], "rb")) == NULL) {
        printf("Error opening the second input file.\n");
        return -1;
    }
    readFileHeader(img2Header, img2Ptr);
    readFileInfoHeader(img2InfoHeader, img2Ptr);
    int img2Size = img2InfoHeader->biSizeImage;
    BYTE *img2Data = (BYTE *)malloc(img2Size);
    readImage(img2Data, img2Ptr, img2Size, img2Header->bfOffBits);

    //Create output file
    FILE *outPtr;
    if ((outPtr = fopen(argv[4], "wb")) == NULL){
        printf("Could not open output file.\n");
        return -1;
    }

    
    
    // Calculating larger size and ratio
    int largerSize = (img1Size > img2Size) ? img1Size : img2Size;
    BYTE *imgOutData = (BYTE *) malloc(largerSize);

    // Calculating and writing the local larger image
    int byteOffset, largerWidth, largerHeight, smallerWidth, smallerHeight;
    if (img1Size >= img2Size) {
        //If img1 is larger
        byteOffset = img1Header->bfOffBits;
        float xRatio = (float) img2InfoHeader->biWidth / (int) img1InfoHeader->biWidth;
        float yRatio = (float) img2InfoHeader->biHeight / (int) img1InfoHeader->biHeight;
        largerWidth = img1InfoHeader->biWidth;
        largerHeight = img1InfoHeader->biHeight;
        smallerWidth = img2InfoHeader->biWidth;
        smallerHeight = img2InfoHeader->biHeight;
        createImage(imgOutData, img1Data, img2Data, ratio, largerWidth, largerHeight, smallerWidth, smallerHeight, xRatio, yRatio);
        writeHeader(outPtr, img1Header, img1InfoHeader);
    }
    else {
        //If img2 is larger
        byteOffset = img2Header->bfOffBits;
        float xRatio = (float) img1InfoHeader->biWidth / (int) img2InfoHeader->biWidth;
        float yRatio = (float) img1InfoHeader->biHeight / (int) img2InfoHeader->biHeight;
        largerWidth = img2InfoHeader->biWidth;
        largerHeight = img2InfoHeader->biHeight;
        smallerWidth = img1InfoHeader->biWidth;
        smallerHeight = img1InfoHeader->biHeight;
        createImage(imgOutData, img2Data, img1Data, (1 - ratio), largerWidth, largerHeight, smallerWidth, smallerHeight, xRatio, yRatio);
        writeHeader(outPtr, img2Header, img2InfoHeader);
    }

    
    //Write imgOutData to output Image
    fseek(outPtr, byteOffset, SEEK_SET);
    fwrite(imgOutData, largerSize, 1, outPtr);
    
    //Freeing Memory
    free(img1Header);
    free(img1InfoHeader);
    free(img1Data);
    free(img2Header);
    free(img2InfoHeader);
    free(img2Data);
    free(imgOutData);

    //Closing Files
    fclose(outPtr);
    fclose(img1Ptr);
    fclose(img2Ptr);
    
}