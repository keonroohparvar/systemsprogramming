#include <stdio.h>
#include <stdlib.h>

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;
typedef unsigned char BYTE;


typedef struct tagBITMAPFILEHEADER {
	char bfType[2];  //specifies the file type
	int bfSize;  //specifies the size in bytes of the bitmap file
	WORD bfReserved1;  //reserved; must be 0
	WORD bfReserved2;  //reserved; must be 0
	DWORD bfOffBits;  //species the offset in bytes 
} bmpFileHeader;


typedef struct tagBITMAPINFOHEADER {
	DWORD biSize;  //specifies the number of bytes required by the struct
	LONG biWidth;  //specifies width in pixels
	LONG biHeight;  //species height in pixels
	WORD biPlanes; //specifies the number of color planes, must be 1
	WORD biBitCount; //specifies the number of bit per pixel
	DWORD biCompression;//spcifies the type of compression
	DWORD biSizeImage;  //size of image in bytes
	LONG biXPelsPerMeter;  //number of pixels per meter in x axis
	LONG biYPelsPerMeter;  //number of pixels per meter in y axis
	DWORD biClrUsed;  //number of colors used by th ebitmap
	DWORD biClrImportant;  //number of colors that are important
} bmpInfoHeader;

//Read in file header
void readFileHeader(FILE *img, bmpFileHeader* fileheader) {
    fread(&(fileheader->bfType), 2, 1, img);
    fread(&(fileheader->bfSize), sizeof(int), 1, img);
    fread(&(fileheader->bfReserved1), sizeof(WORD), 1, img);
    fread(&(fileheader->bfReserved2), sizeof(WORD), 1, img);
    fread(&(fileheader->bfOffBits), sizeof(DWORD), 1, img);
} 

//Read in file infoheader
void readInfoHeader(FILE *img, bmpInfoHeader *infoHeader) {
    fread(&(infoHeader->biSize), sizeof(DWORD), 1, img);
    fread(&(infoHeader->biWidth), sizeof(LONG), 1, img);
    fread(&(infoHeader->biHeight), sizeof(LONG), 1, img);
    fread(&(infoHeader->biPlanes), sizeof(WORD), 1, img);
    fread(&(infoHeader->biBitCount), sizeof(WORD), 1, img);
    fread(&(infoHeader->biCompression), sizeof(DWORD), 1, img);
    fread(&(infoHeader->biSizeImage), sizeof(DWORD), 1, img);
    fread(&(infoHeader->biXPelsPerMeter), sizeof(LONG), 1, img);
    fread(&(infoHeader->biYPelsPerMeter), sizeof(LONG), 1, img);
    fread(&(infoHeader->biClrUsed), sizeof(DWORD), 1, img);
    fread(&(infoHeader->biClrImportant), sizeof(DWORD), 1, img);
}

//Get Color
BYTE getColor(BYTE *imgData, int x, int y, int width, int height, int colorOffset) {
    int padding = (width * 3) % 4;
    int bytesPerLine = width * 3;
    if (padding != 0)
        bytesPerLine = bytesPerLine + 4 - padding;
    BYTE color = imgData[x*3 + y*bytesPerLine + colorOffset];
    return color;
}

//Mix colors
void createImage(BYTE *outImg, BYTE *img1, BYTE *img2, int width, int height, int smallerWidth, int smallerHeight, float xRatio, float yRatio) {
    int x, y;
    int padding = (width * 3) % 4;
    int bytesPerLine = width * 3;
    if (padding != 0) {
        bytesPerLine = bytesPerLine + 4 - padding;
    }
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            //First Image's Bytes
            BYTE red1 = getColor(img1, x, y, width, height, 0);
            BYTE green1 = getColor(img1, x, y, width, height, 1);
            BYTE blue1 = getColor(img1, x, y, width, height, 2);

            //Finding smaller Image's coordinates
            float x2 = x * xRatio;
            float y2 = y * yRatio;
            BYTE red2 = getColor(img1, ((int)x2), ((int)y), smallerWidth, smallerHeight, 0);
            BYTE green2 = getColor(img1, (int)x2, (int)y2, smallerWidth, smallerHeight, 1);
            BYTE blue2 = getColor(img1, (int)x2, (int)y2, smallerWidth, smallerHeight, 2);

            //Combining colors
            BYTE redFinal = (red1 + red2) / 2;
            BYTE greenFinal = (green1 + green2) / 2;
            BYTE blueFinal = (blue1 + blue2) / 2; 

            outImg[x*3 + y*bytesPerLine] = redFinal;
            outImg[x*3 + y*bytesPerLine + 1] = greenFinal;
            outImg[x*3 + y*bytesPerLine + 2] = blueFinal;


        }
    }

}

//Write Output header
void writeOutHeader(FILE *outFile, bmpFileHeader *header, bmpInfoHeader *infoHeader){
    fseek(outFile, 0, SEEK_SET);
    fwrite(&(header->bfType), 2, 1, outFile);
    fseek(outFile, 2, SEEK_SET);
    fwrite(&(header->bfSize), 4, 1, outFile);
    fseek(outFile, 6, SEEK_SET);
    fwrite(&(header->bfReserved1), sizeof(WORD), 1, outFile);
    fseek(outFile, 8, SEEK_SET);
    fwrite(&(header->bfReserved2), sizeof(WORD), 1, outFile);
    fseek(outFile, 10, SEEK_SET);
    fwrite(&(header->bfOffBits), sizeof(DWORD), 1, outFile);
    fseek(outFile, 14, SEEK_SET);

    fwrite(infoHeader, sizeof(infoHeader), 1, outFile);
    
}

int main() {
    FILE *img1;
    if ((img1 = fopen("nopadding.bmp", "rb")) == NULL) {
        printf("Cannot open file.");
        return 1;
    }
    FILE *img2;
    if ((img2 = fopen("nopadding2.bmp", "rb")) == NULL) {
        printf("Cannot open file.");
        return 1;
    }
    
    bmpFileHeader *img1Header = (bmpFileHeader *)malloc(14);
    bmpInfoHeader *img1InfoHeader = (bmpInfoHeader *)malloc(40);
    BYTE *img1Data = (BYTE *)malloc(img1InfoHeader->biSizeImage);
    readFileHeader(img1, img1Header);
    readInfoHeader(img1, img1InfoHeader);
    fread(img1Data, img1InfoHeader->biSizeImage, 1, img1);
    

    bmpFileHeader *img2Header = (bmpFileHeader *)malloc(14); 
    // I am getting a malloc() corrupted top size error and I can't fix it :(
    bmpInfoHeader *img2InfoHeader = (bmpInfoHeader *)malloc(40);
    BYTE *img2Data = (BYTE *)malloc(img2InfoHeader->biSizeImage);
    readFileHeader(img2, img2Header);
    readInfoHeader(img2, img2InfoHeader);
    fread(img2Data, img2InfoHeader->biSizeImage, 1, img2);
    
    //Out file
    FILE *outFile = fopen("out.bmp", "wb");
    BYTE *outData;
    
    
    if (img1Header->bfSize >= img2Header->bfSize) {
        //Img1 is larger
        outData = (BYTE *)malloc(img1InfoHeader->biSizeImage);
        float xRatio = img1InfoHeader->biWidth / img2InfoHeader->biWidth;
        float yRatio = img1InfoHeader->biHeight / img2InfoHeader->biHeight;
        createImage(outData, img1Data, img2Data, img1InfoHeader->biWidth, img1InfoHeader->biHeight, img2InfoHeader->biWidth, img2InfoHeader->biHeight, xRatio, yRatio);
        writeOutHeader(outFile, img1Header, img1InfoHeader);
        fseek(outFile, img1Header->bfOffBits, SEEK_SET);
        fwrite(outData, sizeof(outData), 1, outFile);
    }
    else {
        outData = (BYTE *)malloc(img1InfoHeader->biSizeImage);
        float xRatio = img1InfoHeader->biWidth / img2InfoHeader->biWidth;
        float yRatio = img1InfoHeader->biHeight / img2InfoHeader->biHeight;
        createImage(outData, img2Data, img1Data, img2InfoHeader->biWidth, img2InfoHeader->biHeight, img1InfoHeader->biWidth, img1InfoHeader->biHeight, xRatio, yRatio);
        writeOutHeader(outFile, img2Header, img2InfoHeader);
        fseek(outFile, img2Header->bfOffBits, SEEK_SET);
        fwrite(outData, sizeof(outData), 1, outFile);
    }

    

    free(img1Data);
    free(img2Data);
    free(img1Header);
    free(img1InfoHeader);
    free(img2Header);
    free(img2InfoHeader);
    free(outData);
    fclose(img1);
    fclose(img2);
    fclose(outFile);

}