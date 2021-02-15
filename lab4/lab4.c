#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>

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
    if (argv[2][0]!= '0' && argv[2][0]!='.' && argv[2][0] !='1') {
        printf("Bad ratio.\n");
        return 0;
    }
    double brightness = atof(argv[2]);
    if (brightness > 1 || brightness < 0)
        return 0;
    if (argv[3][0] != '0' && argv[3][0] != '1')
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

void createImage1(BYTE *outData, BYTE *largerImg, float brightness, int width, int height) {
    int x, y;
    int bytesPerLine = width * 3;
    int brightnessOffset = (int)(brightness * 255);
    int padding = (width * 3) % 4;
    if (padding != 0)
        bytesPerLine = bytesPerLine + 4 - padding;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            BYTE redFinal, greenFinal, blueFinal;

            //Read Larger Pixel's Color
            BYTE red = getColor(largerImg, x, y, width, height, 0);
            BYTE green = getColor(largerImg, x, y, width, height, 1);
            BYTE blue = getColor(largerImg, x, y, width, height, 2);
            
            //Gauge Brightness
            int t;
            if (red != '\0')
                t = 3;
            int redInt = (int) red;
            redInt = redInt + brightnessOffset;
            redInt = (redInt > 255) ? 255 : redInt;
            redFinal = (BYTE) redInt;
            int greenInt = (int) green;
            greenInt = greenInt + brightnessOffset;
            greenInt = (greenInt > 255) ? 255 : greenInt;
            greenFinal = (BYTE) greenInt;
            int blueInt = (int) blue;
            blueInt = blueInt + brightnessOffset;
            blueInt = (blueInt > 255) ? 255 : blueInt;
            blueFinal = (BYTE) blueInt;
            
            outData[x*3 + y*bytesPerLine + 0] = redFinal;
            outData[x*3 + y*bytesPerLine + 1] = greenFinal;
            outData[x*3 + y*bytesPerLine + 2] = blueFinal;
        }
    }
}

int createImage2(BYTE *outData, BYTE *largerImg, float brightness, int width, int height) { 
    int x, y;
    int bytesPerLine = width * 3;
    int brightnessOffset = (int)(brightness * 255);
    int padding = (width * 3) % 4;
    if (padding != 0)
        bytesPerLine = bytesPerLine + 4 - padding;
    if (fork() == 0) {
        //Child Process
        for (y = (int)(height/2); y < height; y++) {
            for (x = 0; x < width; x++) {
                BYTE redFinal, greenFinal, blueFinal;

                //Read Larger Pixel's Color
                BYTE red = getColor(largerImg, x, y, width, height, 0);
                BYTE green = getColor(largerImg, x, y, width, height, 1);
                BYTE blue = getColor(largerImg, x, y, width, height, 2);
                
                //Gauge Brightness
                int redInt = (int) red;
                redInt = redInt + brightnessOffset;
                redInt = (redInt > 255) ? 255 : redInt;
                redFinal = (BYTE) redInt;
                int greenInt = (int) green;
                greenInt = greenInt + brightnessOffset;
                greenInt = (greenInt > 255) ? 255 : greenInt;
                greenFinal = (BYTE) greenInt;
                int blueInt = (int) blue;
                blueInt = blueInt + brightnessOffset;
                blueInt = (blueInt > 255) ? 255 : blueInt;
                blueFinal = (BYTE) blueInt;
                
                outData[x*3 + y*bytesPerLine + 0] = redFinal;
                outData[x*3 + y*bytesPerLine + 1] = greenFinal;
                outData[x*3 + y*bytesPerLine + 2] = blueFinal;
            }
        }
        return -1;
    }
    else {
        //Parent Process
        for (y = 0; y < height/2; y++) {
            for (x = 0; x < width; x++) {
                BYTE redFinal, greenFinal, blueFinal;

                //Read Larger Pixel's Color
                BYTE red = getColor(largerImg, x, y, width, height, 0);
                BYTE green = getColor(largerImg, x, y, width, height, 1);
                BYTE blue = getColor(largerImg, x, y, width, height, 2);
                
                //Gauge Brightness
                int redInt = (int) red;
                redInt = redInt + brightnessOffset;
                redInt = (redInt > 255) ? 255 : redInt;
                redFinal = (BYTE) redInt;
                int greenInt = (int) green;
                greenInt = greenInt + brightnessOffset;
                greenInt = (greenInt > 255) ? 255 : greenInt;
                greenFinal = (BYTE) greenInt;
                int blueInt = (int) blue;
                blueInt = blueInt + brightnessOffset;
                blueInt = (blueInt > 255) ? 255 : blueInt;
                blueFinal = (BYTE) blueInt;
                
                outData[x*3 + y*bytesPerLine + 0] = redFinal;
                outData[x*3 + y*bytesPerLine + 1] = greenFinal;
                outData[x*3 + y*bytesPerLine + 2] = blueFinal;
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[20]) {
    if (checkInput(argc, argv) == 0) {
        printf("Invalid input.\n");
        printf("Please format your input as the following:\n\n");
        printf("[programname] [imagefile1] [imagefile2] [ratio] [outputfile]\n\n");
        printf("Example:\nblendimages face.bmp butterfly.bmp 0.3 merged.bmp\n\n");
        return -1;
    }

    clock_t a = clock();

    //Initialize ratio command line argument
    float brightness;
    brightness = atof(argv[2]);
    char parallel = argv[3][0];

    //File 1 Input
    BMPFileHeader *imgHeader = (BMPFileHeader *) malloc(sizeof(BMPFileHeader));
    BMPInfoHeader *imgInfoHeader = (BMPInfoHeader *) malloc(sizeof(BMPInfoHeader));
    FILE *imgPtr;
    if ((imgPtr = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening the first input file.\n");
        return -1;
    }
    readFileHeader(imgHeader, imgPtr);
    readFileInfoHeader(imgInfoHeader, imgPtr);
    int imgSize = imgInfoHeader->biSizeImage;
    int byteOffset = imgHeader->bfOffBits;
    int width = imgInfoHeader->biWidth;
    int height = imgInfoHeader->biHeight;
    BYTE *imgData;
    imgData = (BYTE *)malloc(imgSize);
    readImage(imgData, imgPtr, imgSize, byteOffset);

    //Create output file
    FILE *outPtr;
    if ((outPtr = fopen(argv[4], "wb")) == NULL){
        printf("Could not open output file.\n");
        return -1;
    }

    //Allocating Out Memory
    BYTE *imgOutData = (BYTE *) mmap(NULL, imgSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);

    //Creates image with or without fork
    if (parallel == '0') {
        createImage1(imgOutData, imgData, brightness, width, height);
        printf("Completing brightening without forking.\n");
    }
    if (parallel == '1') {
        int returnNum = createImage2(imgOutData, imgData, brightness, width, height);
        if (returnNum == -1) {
            //This is for ending the child process but continuing the parent process
            return 0;
        }
        printf("Completing brightening with forking.\n");
    }
    writeHeader(outPtr, imgHeader, imgInfoHeader);

    //Write imgOutData to output Image
    fseek(outPtr, byteOffset, SEEK_SET);
    fwrite(imgOutData, imgSize, 1, outPtr);

    munmap(imgOutData, imgSize);

    clock_t b = clock();

    double timeDiff = b - a;
    double timeInSec = timeDiff / CLOCKS_PER_SEC;
    printf("Time of Completion: %f\n", timeInSec);


    
    return -1;
}