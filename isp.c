#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef unsigned char uint8;
typedef unsigned short uint16;
uint8 True = 1;
uint8 False = 0;

long int findImageSize(char *fileName);
uint8 readImage(char* fileName, long int raw_size, int width, int height,
    uint8 *mipi_buffer);
uint8 decodeMIPItoWord(int width, int height, long int raw_size,
    int bitDepth, uint8 *mipi_buffer,
    long int rgb16_size, uint16 *rgb16_buffer);
uint8 linearizeBayer(int width, int height,
    int bitDepth, int offset,
    long int rgb16_size, uint16 *rgb16_buffer);
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
uint8 correctGamma(int width, int height, float gammaRValue, float gammaGValue, float gammaBValue,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel);
uint8 balanceWhite(int width, int height, float r2g, float b2g,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel);
uint8 convert2YUV444(int width, int height,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel,
    uint8 *Y_channel, uint8 *U_channel, uint8 *V_channel);
uint8 subsample2YUV420(int width, int height, long int YUV_size,
    uint8 *Y_channel, uint8 *U_channel, uint8 *V_channel,
    uint8 *Y_420, uint8 *UV_420);
uint8 writeFinalYUVImage(int width, int height,
    uint8 *Y_420, uint8 *UV_420);

int main(int argc, char *argv[])
{
    char *fileName;
    char *defaultRaw = "IMG_Guitar.raw";
    if (argv == 0)
      fileName = defaultRaw;
    else
      fileName = argv[1];
    printf("%s\n",fileName);
    printf("%s\n",argv[0]);
    printf("%d\n",argc);
    int width = 4032;//4224;
    int height = 3024;//3136;
    int bitDepth = 10;

    int offset = -60;

    float CCM[9] = { 1.1000,-0.0500,-0.0500,
                    -0.1000, 1.2000,-0.1000,
                    -0.0000,-0.0000, 1.0000};

/*    float CCM[9] = {1,0,0,
                    0,1,0,
                    0,0,1};
*/
    float gammaRValue = 2.5;
    float gammaGValue = 2.5;
    float gammaBValue = 2.5;

    short int window_size[2] = {3,3};
    short int window_height = window_size[0];
    short int window_width = window_size[1];

    float R2G = 1.00;
    float B2G = 1.00;

    long int raw_size;
    raw_size = findImageSize(fileName);
    if (raw_size == False)
        printf("\nNo image found");

    uint8 *mipi_buffer;
    mipi_buffer = malloc(raw_size);
    if (False == readImage(fileName, raw_size, width, height, mipi_buffer))
        printf("\nError in reading Image File");

    uint16 *rgb16_buffer;
    long int rgb16_size = (raw_size * 2 * 4) / 5;
    long int rgb8_size = rgb16_size/2;
    rgb16_buffer = malloc(rgb16_size);
    if (False == decodeMIPItoWord(width, height, raw_size, bitDepth, mipi_buffer, rgb16_size, rgb16_buffer))
        printf("\nError in Decoding MIPI");

    if (False == linearizeBayer(width, height, bitDepth, offset, rgb16_size, rgb16_buffer))
        printf("\nError in linearization");

    uint16 *R_channel, *G_channel, *B_channel;
    R_channel = malloc(rgb16_size);
    G_channel = malloc(rgb16_size);
    B_channel = malloc(rgb16_size);
    if (False == demosaicImageData(width, height, rgb16_buffer,
        R_channel, G_channel, B_channel))
        printf("\nError in Demosaic");

    unsigned char *R_8channel;
    unsigned char *G_8channel;
    unsigned char *B_8channel;
    R_8channel = malloc(rgb8_size);
    G_8channel = malloc(rgb8_size);
    B_8channel = malloc(rgb8_size);
    if (False == convertRGBto8bit(width, height,
        R_channel, G_channel, B_channel,
        R_8channel, G_8channel, B_8channel))
        printf("\nError in Convertion to 8 bit");

    /*Noise Reduction using Median Filter*/
    if (False == reduceNoise(width, height,
        window_height, window_width,
        R_8channel, G_8channel, B_8channel))
       printf("\nError in reducing Noise");

    if (False == correctColors(width, height, CCM,
        R_8channel, G_8channel, B_8channel))
        printf("\nError in color correction");

    if (False == correctGamma(width, height, gammaRValue, gammaGValue, gammaBValue,
        R_8channel, G_8channel, B_8channel))
        printf("\nError in gamma correction");

    if (False == balanceWhite(width, height, R2G, B2G,
        R_8channel, G_8channel, B_8channel))
        printf("\n Error in white balance");

    unsigned char *Y_channel,*U_channel,*V_channel;
    Y_channel = malloc(rgb8_size);
    U_channel = malloc(rgb8_size);
    V_channel = malloc(rgb8_size);
    if (False == convert2YUV444(width, height,
        R_8channel, G_8channel, B_8channel,
        Y_channel, U_channel, V_channel))
        printf("\nError in YUV444 convertion");

    unsigned char *Y_420;
    unsigned char *UV_420;
    long int YUV_size = rgb16_size;
    Y_420 = malloc(rgb16_size/2);
    UV_420 = malloc(rgb16_size/4);
    if (False == subsample2YUV420(width, height, YUV_size,
        Y_channel, U_channel, V_channel,
        Y_420, UV_420))
      printf("\nError in Chroma Subsampling");

    if (False == writeFinalYUVImage(width, height,
        Y_420, UV_420))
      printf("\nError in writing final YUV");



    /*Clear out all the memory*/
    free(Y_420);
    free(UV_420);
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
    char ffmpegString[100]; /*= "ffmpeg -s 4224x3136 -pix_fmt nv12 -i Out.yuv guitar.jpg";*/
    snprintf(ffmpegString,100,"ffmpeg -s %dx%d -pix_fmt nv12 -i Out.yuv guitar.jpg",width,height);
    system(ffmpegString);

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
      return False;
    fseek(raw_image, 0L, SEEK_END);
    raw_size = ftell(raw_image);
    fclose(raw_image);
    return(raw_size);
};
uint8 readImage(char* fileName, long int raw_size, int width, int height,
    uint8 *mipi_buffer)
{
    FILE* raw_image;
    /*Reading File and finding size(3136 x 4224 x 10 bit BGGR)*/
    raw_image = fopen(fileName,"rb");
    if ( raw_image == NULL)
      return False;
    /*Read raw contents into mipi_buffer*/
    fread(mipi_buffer,1,raw_size,raw_image);
    fclose(raw_image);
    return True;
};

uint8 decodeMIPItoWord(int width, int height, long int raw_size,
    int bitDepth, uint8 *mipi_buffer,
    long int rgb16_size, uint16 *rgb16_buffer)
{
    /*Convert mipi into 16bitBayer RGB*/
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
    return True;
};

uint8 linearizeBayer(int width, int height,
    int bitDepth, int offset,
    long int rgb16_size, uint16 *rgb16_buffer)
{
    long int i;
    for (i = 0; i < width*height; i++){
        *(rgb16_buffer + i)+=offset;
    }
    return True;
}

uint8 demosaicImageData(int width, int height,
    uint16 *rgb16_buffer,
    uint16 *R_channel, uint16 *G_channel, uint16 *B_channel)
{
    int i,j;
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
    return True;
};

uint8 convertRGBto8bit(int width, int height,
    uint16 *R_channel, uint16 *G_channel, uint16 *B_channel,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel)
{
    int i,j;
    /*RGB888 Conversion*/
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            *(R_8channel + (i*width) + j) = (unsigned char)((*(R_channel + (i*width) + j)) >> 2);
            *(G_8channel + (i*width) + j) = (unsigned char)((*(G_channel + (i*width) + j)) >> 2);
            *(B_8channel + (i*width) + j) = (unsigned char)((*(B_channel + (i*width) + j)) >> 2);
        }
    }
    return True;
};

uint8 reduceNoise(int width, int height,
    int window_height, int window_width,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel)
{
    short int window_R[window_height*window_width];
    short int window_G[window_height*window_width];
    short int window_B[window_height*window_width];
    short int w,x,y;
    short int p, key, q, n;
    int i,j;
    n = window_height * window_width;
    for (i = window_height/2; i < height - window_height/2; i++) {
        for (j = window_width/2; j < width - window_width/2; j++) {
            w = 0;
            for (x = i - window_height/2; x <= i + window_height/2; x++) {
                for (y = j - window_width/2; y <= j + window_width/2; y++) {
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
    return True;
};

uint8 correctColors(int width, int height, float *CCM,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel)
{
    /*Color Correction(for subtracting out the overlap 
     * in light colors in pixels, due to imperfections 
     * of color pixels or BAYER filters*/
    /*From here on Only one set of buffers will be used.
     * Local variables should be declared for processing and 
     * after that output should go to <Color>_8channel variable*/
    int i,j;
    float pixR;
    float pixG;
    float pixB;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            pixR = (float)(*(R_8channel + (i*width) + j));
            pixG = (float)(*(G_8channel + (i*width) + j));
            pixB = (float)(*(B_8channel + (i*width) + j));
            *(R_8channel + (i*width) + j) = pixR*CCM[0] + pixG*CCM[1] + pixB*CCM[2];
            *(G_8channel + (i*width) + j) = pixR*CCM[3] + pixG*CCM[4] + pixB*CCM[5];
            *(B_8channel + (i*width) + j) = pixR*CCM[6] + pixG*CCM[7] + pixB*CCM[8];
        }
    }
    return True;
};

uint8 correctGamma(int width, int height, float gammaRValue, float gammaGValue, float gammaBValue,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel)
{
    /*From here on Only one set of buffers will be used.
     * Local variables should be declared for processing and 
     * after that output should go to <Color>_8channel variable*/
    int i,j;
    float pixR;
    float pixG;
    float pixB;
    /*Gamma Correction*/
    float gammaR = 1/gammaRValue;
    float gammaG = 1/gammaGValue;
    float gammaB = 1/gammaBValue;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            pixR = (float)(*(R_8channel + (i*width) + j));
            pixG = (float)(*(B_8channel + (i*width) + j));
            pixB = (float)(*(G_8channel + (i*width) + j));
            *(R_8channel + (i*width) + j) = 255 * pow(pixR/255 , gammaR);
            *(G_8channel + (i*width) + j) = 255 * pow(pixG/255 , gammaG);
            *(B_8channel + (i*width) + j) = 255 * pow(pixB/255 , gammaB);
        }
    }
    return True;
};

uint8 balanceWhite(int width, int height, float R2G, float B2G,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel)
{
    /*From here on Only one set of buffers will be used.
     * Local variables should be declared for processing and 
     * after that output should go to <Color>_8channel variable*/
    int i,j;
    float pixR;
    float pixG;
    float pixB;
    /*White Balance*/
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
    return True;
};

uint8 convert2YUV444(int width, int height,
    uint8 *R_8channel, uint8 *G_8channel, uint8 *B_8channel,
    uint8 *Y_channel, uint8 *U_channel, uint8 *V_channel)
{
    int i,j;
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
    return True;
};

uint8 subsample2YUV420(int width, int height, long int YUV_size,
    uint8 *Y_channel, uint8 *U_channel, uint8 *V_channel,
    uint8 *Y_420, uint8 *UV_420)
{
    int i,j;
    memcpy(Y_420,Y_channel,YUV_size/2);
    /*YUV 420 creation*/
    for (i = 0; i < height; i+=2) {
        for (j = 0; j < width; j+=2) {
            *(UV_420 + (i*width/2) + j)      = *(U_channel + (i*width) + j);
            *(UV_420 + (i*width/2) + j + 1)  = *(V_channel + (i*width) + j);
        }
    }
    return True;
};

uint8 writeFinalYUVImage(int width, int height,
    uint8 *Y_420, uint8 *UV_420)
{
    /*Writing final image*/
    long int write_size = width*height*2;
    FILE *out_image;
    out_image = fopen("Out.yuv","wb");
    printf("\nWriting to YUV420\n");
    //fwrite(rgb16_buffer,sizeof(char),write_size,out_image);
    //fwrite(R_8channel,sizeof(char),write_size/2,out_image);
    //fwrite(G_8channel,sizeof(char),write_size/2,out_image);
    //fwrite(B_8channel,sizeof(char),write_size/2,out_image);
    fwrite(Y_420,sizeof(char),write_size/2,out_image);
    fwrite(UV_420,sizeof(char),write_size/4,out_image);
    //fwrite(U_channel,sizeof(char),write_size/2,out_image);
    //fwrite(V_channel,sizeof(char),write_size/2,out_image);
    fclose(out_image);
    return True;
};

