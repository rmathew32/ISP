#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
  int width, height;
  width = 4224;
  height = 3136;

  /*Reading File and finding size(3136 x 4224 x 10 bit BGGR)*/
  FILE *raw_image;
  long int raw_size;
  raw_image = fopen("IMG_20180314_215440.raw","rb");
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

  /*YUV convertion*/
  unsigned short *Y_channel,*U_channel,*V_channel;
  Y_channel = malloc(rgb16_size);
  U_channel = malloc(rgb16_size);
  V_channel = malloc(rgb16_size);
  for(i = 0; i < height; i++) {
    for(j = 0; j < width; j++) {
      *(Y_channel + (i*width) + j) = 16 +  (((66  * (*(R_channel + (i*width) + j))
                                                + 129 * (*(G_channel + (i*width) + j))
                                                 + 25  * (*(B_channel + (i*width) + j)))) >> 8);
      *(U_channel + (i*width) + j) = 128 + (((38  * (*(R_channel + (i*width) + j))
                                                - 74  * (*(G_channel + (i*width) + j))
                                                 + 112 * (*(B_channel + (i*width) + j)))) >> 8);
      *(V_channel + (i*width) + j) = 128 + (((112 * (*(R_channel + (i*width) + j))
                                                - 94  * (*(G_channel + (i*width) + j))
                                                 - 18  * (*(B_channel + (i*width) + j)))) >> 8);
    }
  }

  /*YUV 420 8 bit conversion*/
  unsigned char *Y_8channel;
  unsigned char *U_8channel;
  unsigned char *V_8channel;
  Y_8channel = malloc(rgb16_size/2);
  U_8channel = malloc(rgb16_size/2);
  V_8channel = malloc(rgb16_size/2);
  
  for (i = 0; i < height; i++) {
    for (j = 0; j < height; j++) {
     *(Y_8channel + (i*width) + j) = (unsigned char)((*(Y_channel + (i*width) + j)) >> 2);
     *(U_8channel + (i*width) + j) = (unsigned char)((*(U_channel + (i*width) + j)) >> 2);
     *(V_8channel + (i*width) + j) = (unsigned char)((*(V_channel + (i*width) + j)) >> 2);
    }
  }
  
  /*YUV 420 creation*/
  unsigned char *Y_420;
  unsigned char *UV_420;
  Y_420 = malloc(rgb16_size/2);
  memcpy(Y_420,Y_8channel,rgb16_size/2);
  UV_420 = malloc(rgb16_size/4);
  for (i = 0; i < height; i+=2) {
    for (j = 0; j < width; j+=2) {
      *(UV_420 + (i*width/2) + j)      = *(U_8channel + (i*width) + j);
      *(UV_420 + (i*width/2) + j + 1)  = *(V_8channel + (i*width) + j);
    }
  }

  /*Writing final image*/
  FILE *out_image;
  out_image = fopen("Out.yuv","wb");
  fwrite(Y_420,sizeof(char),rgb16_size/2,out_image);
 // fwrite(UV_420,sizeof(char),rgb16_size/4,out_image);
  fwrite(U_8channel,sizeof(char),rgb16_size/2,out_image);
  fwrite(V_8channel,sizeof(char),rgb16_size/2,out_image);
  fclose(out_image);
  
  /*Clear out all the memory*/
  free(Y_420);
  free(UV_420);
  free(Y_8channel);
  free(U_8channel);
  free(V_8channel);
  free(Y_channel);
  free(U_channel);
  free(V_channel);
  free(R_channel);
  free(G_channel);
  free(B_channel);
  free(rgb16_buffer);
  free(mipi_buffer);

  /*Success*/
  return 1;
}
