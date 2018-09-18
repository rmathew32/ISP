#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef unsigned char uint8;
typedef unsigned short uint16;

long int findImageSize(char *fileName);
uint8 readImage(char* fileName, int width, int height,
    uint8 *mipi_buffer);
uint8 decodeMIPItoWord(int width, int height,
    int bitdepth, uint8 *mipi_buffer,
    uint16 *rgb16_buffer);
uint8 demosaicImageData(int width, int height,
    uint16 *rgb16_buffer,
    uint16 *R_channel, uint16 *G_channel, uint16 *B_channel);
uint8 convertRGBto8bit(int width, int height,
    uint16 *R_channel, uint16 *G_channel, uint16 *B_channel,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel);
uint8 reduceNoise(int width, int height,
    int window_height, int window_width,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel);
uint8 correctColors(int width, int height, float *CCM,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel);
uint8 correctGamma(int width, int height, float gammaValue,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel);
uint8 balanceWhite(int width, int height, float r2g, float b2g,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel);
uint8 convert2YUV444(int width, int height,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel,
    uint8 *Y_channel, uint8 *U_channel, uint8 *V_channel);
uint8 subsample2YUV420(int width, int height,
    uint8 *Y_channel, uint8 *U_channel, uint8 *V_channel,
    uint8 *Y_420, uint8 *UV_420);
uint8 writeFinalYUVImage(int width, int height,
    uint8 *Y_420, uint8 *UV_420);

int main(int argv, char *argc[])
{
    char *fileName = "IMG_Guitar.raw";
    int width = 4224;
    int height = 3136;
    int bitDepth = 10;
    long int raw_size;

    raw_size = findImageSize(char *fileName);

    mipi_buffer = malloc(raw_size);
    uint8 readImage(char* fileName, int width, int height,
        uint8 *mipi_buffer);

    uint16 *rgb16_buffer;
    rgb16_buffer = malloc(rgb16_size);
    uint8 decodeMIPItoWord(int width, int height,
        int bitdepth, uint8 *mipi_buffer,
        uint16 *rgb16_buffer);

    uint16 *R_channel, uint16 *G_channel, uint16 *B_channel;
    R_channel = malloc(rgb16_size);
    G_channel = malloc(rgb16_size);
    B_channel = malloc(rgb16_size);
    uint8 demosaicImageData(int width, int height,
        uint16 *rgb16_buffer,
        uint16 *R_channel, uint16 *G_channel, uint16 *B_channel);

    unsigned char *R_8channel;
    unsigned char *G_8channel;
    unsigned char *B_8channel;
    R_8channel = malloc(rgb16_size/2);
    G_8channel = malloc(rgb16_size/2);
    B_8channel = malloc(rgb16_size/2);
    uint8 convertRGBto8bit(int width, int height,
        uint16 *R_channel, uint16 *G_channel, uint16 *B_channel,
        uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel);

    /*Noise Reduction using Median Filter*/
    short int window_size[2] = {1,1};
    short int window_height = window_size[0];
    short int window_width = window_size[1];
    short int window_R[window_height*window_width];
    short int window_G[window_height*window_width];
    short int window_B[window_height*window_width];
    short int w,x,y;
    short int p, key, q, n;
    n = window_height * window_width;
    uint8 reduceNoise(int width, int height,
        int window_height, int window_width,
        uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel);

    uint8 correctColors(int width, int height, float *CCM,
        uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel);

    uint8 correctGamma(int width, int height, float gammaValue,
        uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel);

    uint8 balanceWhite(int width, int height, float r2g, float b2g,
        uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel);

    unsigned char *Y_channel,*U_channel,*V_channel;
    Y_channel = malloc(rgb16_size/2);
    U_channel = malloc(rgb16_size/2);
    V_channel = malloc(rgb16_size/2);
    uint8 convert2YUV444(int width, int height,
        uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel,
        uint8 *Y_channel, uint8 *U_channel, uint8 *V_channel);

    unsigned char *Y_420;
    unsigned char *UV_420;
    Y_420 = malloc(rgb16_size/2);
    memcpy(Y_420,Y_channel,rgb16_size/2);
    UV_420 = malloc(rgb16_size/4);
    uint8 subsample2YUV420(int width, int height,
        uint8 *Y_channel, uint8 *U_channel, uint8 *V_channel,
        uint8 *Y_420, uint8 *UV_420);

    uint8 writeFinalYUVImage(int width, int height,
        uint8 *Y_420, uint8 *UV_420);



    /*From here on Only one set of buffers will be used.
     * Local variables should be declared for processing and 
     * after that output should go to <Color>_8channel variable*/
    float pixR;
    float pixG;
    float pixB;
    /*Clear out all the memory*/
    free(R_8channel);
    free(G_8channel);
    free(B_8channel);
    free(Y_channel);
    free(U_channel);
    free(V_channel);
    free(R_channel);
    free(G_channel);
    free(B_channel);
    free(rgb16_buffer);
    free(mipi_buffer);

    /*JPEG Convertion*/
    system("ffmpeg -s 4224x3136 -pix_fmt nv12 -i Out.yuv guitar.jpg");

    /*Success*/
    return 1;
}
long int findImageSize(char* fileName)
{
    long int raw_size;
    FILE* raw_image;
    /*Reading File and finding size(3136 x 4224 x 10 bit BGGR)*/
    raw_image = fopen(fileName,"rb");
    if ( raw_image == NULL)
      return false;
    fseek(raw_image, 0L, SEEK_END);
    raw_size = ftell(raw_image);
    fclose(raw_image);
    return(raw_size);
}
uint8 readImage(char* fileName, int width, int height,
    uint8 *mipi_buffer)
{
    long int raw_size;
    FILE* raw_image;
    /*Reading File and finding size(3136 x 4224 x 10 bit BGGR)*/
    raw_image = fopen(fileName,"rb");
    if ( raw_image == NULL)
      return false;
    fseek(raw_image, 0L, SEEK_END);
    raw_size = ftell(raw_image);
    rewind(raw_image);
    /*Read raw contents into mipi_buffer*/
    mipi_buffer = malloc(raw_size);
    fread(mipi_buffer,1,raw_size,raw_image);
    fclose(raw_image);
    return true;
}

uint8 decodeMIPItoWord(int width, int height,
    int bitdepth, uint8 *mipi_buffer,
    uint16 *rgb16_buffer)
{
    /*Convert mipi into 16bitBayer RGB*/
    long int raw_size = sizeof(mipi_buffer);
    long int rgb16_size = (raw_size * 2 * 4) / 5;
    long int write_size = width*height*2;
    unsigned short temp[5];
    long int i,j;
    for (i = 0, j = 0; i < raw_size; i += 5, j += 4)
    {
        temp[0] = *(mipi_buffer + (i + 0)); 
        temp[1] = *(mipi_buffer + (i + 1)); 
        temp[2] = *(mipi_buffer + (i + 2)); 
        temp[3] = *(mipi_buffer + (i + 3)); 
        temp[4] = *(mipi_buffer + (i + 4));
        *(rgb16_buffer + 0 + j) = (( temp[0] << 2 ) | ((temp[4] & 0x03) >> 0)) & 0x03FF; 
        *(rgb16_buffer + 1 + j) = (( temp[1] << 2 ) | ((temp[4] & 0x0C) >> 2)) & 0x03FF; 
        *(rgb16_buffer + 2 + j) = (( temp[2] << 2 ) | ((temp[4] & 0x30) >> 4)) & 0x03FF; 
        *(rgb16_buffer + 3 + j) = (( temp[3] << 2 ) | ((temp[4] & 0xC0) >> 6)) & 0x03FF;
    }
}


uint8 demosaicImageData(int width, int height,
    uint16 *rgb16_buffer,
    uint16 *R_channel, uint16 *G_channel, uint16 *B_channel)
{
    /*Demosaic*/
    for(i = 0; i < height; i+=2) {
        for(j = 0; j < width; j+=2) {
            *(R_channel + i*width +j) =( *(rgb16_buffer + (((i-1)*width) + (j-1)))
                    + *(rgb16_buffer + (((i-1)*width) + (j+1)))
                    + *(rgb16_buffer + (((i+1)*width) + (j-1)))
                    + *(rgb16_buffer + (((i+1)*width) + (j+1))))/4;

            *(G_channel + i*width +j) = (*(rgb16_buffer + (((i-1)*width) + (j)))
                    + *(rgb16_buffer + (((i)*width) + (j+1)))
                    + *(rgb16_buffer + (((i)*width) + (j-1)))
                    + *(rgb16_buffer + (((i+1)*width) + (j))))/4; 

            *(B_channel + i*width +j) = (*(rgb16_buffer + (((i)*width) + (j))));

            *(R_channel + i*width + j+1) = (*(rgb16_buffer + (((i-1)*width) + (j+1)))
                    + *(rgb16_buffer + (((i+1)*width) + (j+1))))/2;

            *(G_channel + i*width + j+1) = (*(rgb16_buffer + (((i)*width) + (j+1))));

            *(B_channel + i*width + j+1) = (*(rgb16_buffer + (((i)*width) + (j-1+1)))
                    + *(rgb16_buffer + (((i)*width) + (j+1+1))))/2;

            *(R_channel + (i+1)*width +j) = (*(rgb16_buffer + (((i+1)*width) + (j-1)))
                    + *(rgb16_buffer + (((i+1)*width) + (j+1))))/2;

            *(G_channel + (i+1)*width +j) = (*(rgb16_buffer + (((i+1)*width) + (j))));

            *(B_channel + (i+1)*width +j) = (*(rgb16_buffer + (((i-1+1)*width) + (j)))
                    + *(rgb16_buffer + (((i+1+1)*width) + (j))))/2;

            *(R_channel + (i+1)*width +j+1) = (*(rgb16_buffer + (((i+1)*width) + (j+1))));

            *(G_channel + (i+1)*width +j+1) = (*(rgb16_buffer + (((i-1+1)*width) + (j+1)))
                    + *(rgb16_buffer + (((i+1)*width) + (j+1+1)))
                    + *(rgb16_buffer + (((i+1)*width) + (j-1+1)))
                    + *(rgb16_buffer + (((i+1+1)*width) + (j+1))))/4; 

            *(B_channel + (i+1)*width +j+1) = (*(rgb16_buffer + (((i-1+1)*width) + (j-1+1)))
                    + *(rgb16_buffer + (((i-1+1)*width) + (j+1+1)))
                    + *(rgb16_buffer + (((i+1+1)*width) + (j-1+1)))
                    + *(rgb16_buffer + (((i+1+1)*width) + (j+1+1))))/4;

        }
    }
}

uint8 convertRGBto8bit(int width, int height,
    uint16 *R_channel, uint16 *G_channel, uint16 *B_channel,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel)
{
    /*RGB888 Conversion*/
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            *(R_8channel + (i*width) + j) = (unsigned char)((*(R_channel + (i*width) + j)) >> 2);
            *(G_8channel + (i*width) + j) = (unsigned char)((*(G_channel + (i*width) + j)) >> 2);
            *(B_8channel + (i*width) + j) = (unsigned char)((*(B_channel + (i*width) + j)) >> 2);
        }
    }
}

uint8 reduceNoise(int width, int height,
    int window_height, int window_width,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel)
{
#if 0
    for (i = window_height/2; i < height - window_height/2; i++) {
        for (j = window_width/2; j < width - window_width/2; j++) {
            w = 0;
            for (x = i - window_height/2; x <= i + window_height/2; x++) {
                for (y = i - window_width/2; y <= i + window_width/2; y++) {
                    window_R[w] = *(R_8channel + (x*width) + y);
                    window_G[w] = *(G_8channel + (x*width) + y);
                    window_B[w] = *(B_8channel + (x*width) + y);
                    w++;
                }
            }
            /*Insertion sort implemeted*/
            for (p = 1; p < n; p++)
            {
                /*R channel sort*/
                key = window_R[p];
                q = p-1;
                while (q >= 0 && window_R[q] > key)
                {
                    window_R[q+1] = window_R[q];
                    q = q-1;
                }
                window_R[q+1] = key;
                /*G channel sort*/
                key = window_G[p];
                q = p-1;
                while (q >= 0 && window_G[q] > key)
                {
                    window_G[q+1] = window_G[q];
                    q = q-1;
                }
                window_G[q+1] = key;
                /*B channel sort*/
                key = window_B[p];
                q = p-1;
                while (q >= 0 && window_B[q] > key)
                {
                    window_B[q+1] = window_B[q];
                    q = q-1;
                }
                window_B[q+1] = key;
            }

            *(R_8channel + (i*width) + j) = window_R[window_height*window_width/2];
            *(G_8channel + (i*width) + j) = window_G[window_height*window_width/2];
            *(B_8channel + (i*width) + j) = window_B[window_height*window_width/2];
        }
    }
#endif
}

uint8 correctColors(int width, int height, float *CCM,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel)
{
    /*Color Correction(for subtracting out the overlap 
     * in light colors in pixels, due to imperfections 
     * of color pixels or BAYER filters*/
    float CCM[3][3] = {{ 1.7858,-0.7494,-0.0364},
                       {-0.1317, 1.2090,-0.0773},
                       {-0.0400,-0.7876, 1.7476}};
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            pixR = (float)(*(R_8channel + (i*width) + j));
            pixG = (float)(*(B_8channel + (i*width) + j));
            pixB = (float)(*(G_8channel + (i*width) + j));
            *(R_8channel + (i*width) + j) = pixR*CCM[0][0] + pixG*CCM[0][1] + pixB*CCM[0][2];
            *(G_8channel + (i*width) + j) = pixR*CCM[1][0] + pixG*CCM[1][1] + pixB*CCM[1][2];
            *(B_8channel + (i*width) + j) = pixR*CCM[2][0] + pixG*CCM[2][1] + pixB*CCM[2][2];
        }
    }


}

uint8 correctGamma(int width, int height, float gammaValue,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel)
{
    /*Gamma Correction*/
    float gammaCorrection = 1.5;
    float gamma = 1/gammaCorrection;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            pixR = (float)(*(R_8channel + (i*width) + j));
            pixG = (float)(*(B_8channel + (i*width) + j));
            pixB = (float)(*(G_8channel + (i*width) + j));
            *(R_8channel + (i*width) + j) = 255 * pow(pixR/255 , gamma);
            *(G_8channel + (i*width) + j) = 255 * pow(pixG/255 , gamma);
            *(B_8channel + (i*width) + j) = 255 * pow(pixB/255 , gamma);
        }
    }


}
uint8 balanceWhite(int width, int height, float r2g, float b2g,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel)
{
    /*White Balance*/
    float R2G = 1.00;
    float B2G = 1.00;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            pixR = (float)(*(R_8channel + (i*width) + j));
            pixG = (float)(*(B_8channel + (i*width) + j));
            pixB = (float)(*(G_8channel + (i*width) + j));
            *(R_8channel + (i*width) + j) = pixR/R2G;
            *(G_8channel + (i*width) + j) = pixG;
            *(B_8channel + (i*width) + j) = pixB/B2G;
        }
    }


}
uint8 convert2YUV444(int width, int height,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel,
    uint8 *Y_channel, uint8 *U_channel, uint8 *V_channel)
{
    /*YUV convertion*/
    for(i = 0; i < height; i++) {
        for(j = 0; j < width; j++) {
            *(Y_channel + (i*width) + j) = 16 +  (((66 * (*(R_8channel + (i*width) + j))
                            + 129 * (*(G_8channel + (i*width) + j))
                            + 25  * (*(B_8channel + (i*width) + j)))) >> 8);
            *(U_channel + (i*width) + j) = 128 + (((-38  * (*(R_8channel + (i*width) + j))
                            - 74  * (*(G_8channel + (i*width) + j))
                            + 112 * (*(B_8channel + (i*width) + j)))) >> 8);
            *(V_channel + (i*width) + j) = 128 + (((112 * (*(R_8channel + (i*width) + j))
                            - 94  * (*(G_8channel + (i*width) + j))
                            - 18  * (*(B_8channel + (i*width) + j)))) >> 8);
        }
    }


}

uint8 subsample2YUV420(int width, int height,
    uint8 *Y_channel, uint8 *U_channel, uint8 *V_channel,
    uint8 *Y_420, uint8 *UV_420)
{
    /*YUV 420 creation*/
    for (i = 0; i < height; i+=2) {
        for (j = 0; j < width; j+=2) {
            *(UV_420 + (i*width/2) + j)      = *(U_channel + (i*width) + j);
            *(UV_420 + (i*width/2) + j + 1)  = *(V_channel + (i*width) + j);
        }
    }
    free(Y_420);
    free(UV_420);


}
uint8 writeFinalYUVImage(int width, int height,
    uint8 *Y_420, uint8 *UV_420)
{
    /*Writing final image*/
    FILE *out_image;
    out_image = fopen("Out.yuv","wb");
    //fwrite(rgb16_buffer,sizeof(char),write_size,out_image);
    //fwrite(R_8channel,sizeof(char),write_size/2,out_image);
    //fwrite(G_8channel,sizeof(char),write_size/2,out_image);
    //fwrite(B_8channel,sizeof(char),write_size/2,out_image);
    fwrite(Y_420,sizeof(char),write_size/2,out_image);
    fwrite(UV_420,sizeof(char),write_size/4,out_image);
    //fwrite(U_channel,sizeof(char),write_size/2,out_image);
    //fwrite(V_channel,sizeof(char),write_size/2,out_image);
    fclose(out_image);
}

