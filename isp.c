#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main()
{
    int width, height;
    width = 4224;
    height = 3136;

    /*Reading File and finding size(3136 x 4224 x 10 bit BGGR)*/
    FILE *raw_image;
    long int raw_size;
    raw_image = fopen("IMG_Guitar.raw","rb");
    fseek(raw_image, 0L, SEEK_END);
    raw_size = ftell(raw_image);
    rewind(raw_image);

    /*Read raw contents into mipi_buffer*/
    unsigned char *mipi_buffer;
    mipi_buffer = malloc(raw_size);
    fread(mipi_buffer,1,raw_size,raw_image);
    fclose(raw_image);

    /*Convert mipi into 16bitBayer RGB*/
    unsigned short *rgb16_buffer;
    long int rgb16_size = (raw_size * 2 * 4) / 5;
    long int write_size = width*height*2;
    unsigned short temp[5];
    long int i,j;
    rgb16_buffer = malloc(rgb16_size); 
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

    /*Demosaic*/
    unsigned short *R_channel,*G_channel,*B_channel;
    R_channel = malloc(rgb16_size);
    G_channel = malloc(rgb16_size);
    B_channel = malloc(rgb16_size);
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

            //*(rgb16_buffer + ((i*height)+j))
        }
    }

    /*RGB888 Conversion*/
    unsigned char *R_8channel;
    unsigned char *G_8channel;
    unsigned char *B_8channel;
    R_8channel = malloc(rgb16_size/2);
    G_8channel = malloc(rgb16_size/2);
    B_8channel = malloc(rgb16_size/2);

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            *(R_8channel + (i*width) + j) = (unsigned char)((*(R_channel + (i*width) + j)) >> 2);
            *(G_8channel + (i*width) + j) = (unsigned char)((*(G_channel + (i*width) + j)) >> 2);
            *(B_8channel + (i*width) + j) = (unsigned char)((*(B_channel + (i*width) + j)) >> 2);
        }
    }

    /*From here on Only one set of buffers will be used.
     * Local variables should be declared for processing and 
     * after that output should go to <Color>_8channel variable*/
    float pixR;
    float pixG;
    float pixB;

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

    /*YUV convertion*/
    unsigned char *Y_channel,*U_channel,*V_channel;
    Y_channel = malloc(rgb16_size/2);
    U_channel = malloc(rgb16_size/2);
    V_channel = malloc(rgb16_size/2);
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

    /*YUV 420 creation*/
    unsigned char *Y_420;
    unsigned char *UV_420;
    Y_420 = malloc(rgb16_size/2);
    memcpy(Y_420,Y_channel,rgb16_size/2);
    UV_420 = malloc(rgb16_size/4);
    for (i = 0; i < height; i+=2) {
        for (j = 0; j < width; j+=2) {
            *(UV_420 + (i*width/2) + j)      = *(U_channel + (i*width) + j);
            *(UV_420 + (i*width/2) + j + 1)  = *(V_channel + (i*width) + j);
        }
    }

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
    system("ffmpeg -s 4224x3136 -pix_fmt nv12 -i Out.yuv guitar.jpg");

    /*Success*/
    return 1;
}
