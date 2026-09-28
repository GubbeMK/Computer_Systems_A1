//To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
//To run (linux/mac): ./main.out example.bmp example_inv.bmp

//To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
//To run (win): main.exe example.bmp example_inv.bmp

#include <stdlib.h>
#include <stdio.h>
#include "cbmp.h"
#include <string.h>

unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char array_2d1[BMP_WIDTH][BMP_HEIGTH];
unsigned char array_2d2[BMP_WIDTH][BMP_HEIGTH];

void black_n_white(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char array_2d1[BMP_WIDTH][BMP_HEIGTH], unsigned char array_2d2[BMP_WIDTH][BMP_HEIGTH]) {
  unsigned char th = 90;
  
  for (int x = 0; x < BMP_WIDTH; x++) 
  {
    for (int y = 0; y < BMP_HEIGTH; y++) 
    {
      unsigned char gray_px = (input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2]) / 3;
      if (gray_px >= th) {
        array_2d1[x][y] = 255;
      }
      else {
        array_2d1[x][y] = 0;
      }
    }
  }
}

void erode(unsigned char in[BMP_WIDTH][BMP_HEIGTH], unsigned char out[BMP_WIDTH][BMP_HEIGTH]) {
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      int left  = (x > 0) ? in[x-1][y] : 0;
      int right = (x < BMP_WIDTH-1) ? in[x+1][y] : 0;
      int up    = (y > 0) ? in[x][y-1] : 0;
      int down  = (y < BMP_HEIGTH-1) ? in[x][y+1] : 0;
      
      if (in[x][y] && left && right && up && down) {
        out[x][y] = 255;
      } else {
        out[x][y] = 0;
      }
    }
  }
}

void cross(int x, int y) {
  for (int l = -8; l < 8; l++) {
    if (x+l > 0 && x+l < BMP_WIDTH) {
      input_image[x+l][y][0] = 255;
      input_image[x+l][y][1] = 0;
      input_image[x+l][y][2] = 0;
    }
  }
  for (int v = -8; v < 8; v++) {
    if (y+v > 0 && y+v < BMP_HEIGTH) {
      input_image[x][y+v][0] = 255;
      input_image[x][y+v][1] = 0;
      input_image[x][y+v][2] = 0;
    }
  }
}

void detection(unsigned char in[BMP_WIDTH][BMP_HEIGTH], int *count, int *end) {
  int detection_length = 14;
  int toppoint = 0;
  int rightpoint = 0;

  int exclude = 0;
  int detected = 0;
  int end_checker = 1;


  while (1) {
  //for (int t = 0; t < 50; t++) {
    for (int x = rightpoint; x < (rightpoint + detection_length); x++) {
      for (int y = toppoint; y < (toppoint + detection_length); y++) {
        //printf("%d, %d\n", x, y);
        if (in[x][y]) {
          detected = 1;
          end_checker = 0;
        }

        if ((x == rightpoint || x == rightpoint + detection_length-1 || y == toppoint || y == toppoint + detection_length-1) && in[x][y]) {
          exclude = 1;
          end_checker = 0;
          break;
        }
      }
      if (exclude) {
        break;
      }
    }

    if (!exclude && detected) {
      //change arrays bits within detection_length to 0000
      //printf("Rightpoint: %d, Toppoint: %d\n", rightpoint, toppoint);
      
      for (int k = rightpoint; k < rightpoint + detection_length; k++) {
        memset(in[k] + toppoint, 0, (detection_length+2)*sizeof(unsigned char));
      }
      
      cross(rightpoint+(detection_length/2), toppoint+(detection_length/2));

      (*count)++;
    }


    if (toppoint == BMP_HEIGTH - detection_length && rightpoint == BMP_WIDTH - detection_length) {
      //printf("Break at (%d, %d)\n", rightpoint, toppoint);
      if (end_checker) {
        *end = 0;
      }
      break;
    }

    if (rightpoint >= BMP_WIDTH - detection_length) {
      rightpoint = 0;
      toppoint += 1;
    } else if (exclude == 0 && detected == 0) {
      rightpoint = ((rightpoint + detection_length) > (BMP_WIDTH - detection_length)) ? (BMP_WIDTH - detection_length) : (rightpoint + (detection_length/2)); 
    } else {
      rightpoint += 1;
    }

    exclude = 0;
    detected = 0;
  }
}



void get_output_image(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char array_2d1[BMP_WIDTH][BMP_HEIGTH]) {
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      for (int c = 0; c < BMP_CHANNELS; c++) {
        output_image[x][y][c] = array_2d1[x][y];
      }
    }
  }
}

  //Declaring the array to store the image (unsigned char = unsigned 8 bit)


//Main function
int main(int argc, char** argv)
{
  //argc counts how may arguments are passed
  //argv[0] is a string with the name of the program
  //argv[1] is the first command line argument (input image)
  //argv[2] is the second command line argument (output image)

  //Checking that 2 arguments are passed
  if (argc != 3)
  {
      fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
      exit(1);
  }

  printf("Example program - 02132 - A1\n");

  //Load image from file
  read_bitmap(argv[1], input_image);

  //Run inversion
  //invert(input_image,output_image);

  //Gray_scale and apply binary threshold
  black_n_white(input_image, array_2d1, array_2d2);

  //Erode image
  int count = 0;
  int end = 1;
  int test_amount = 0;

  /* 
  test_amount = 11;
  
  for (int test = 0; test < test_amount; test++) {
    if (test % 2 == 0) {
      erode(array_2d1, array_2d2);
      detection(array_2d2, &count);
    }
    else {
      erode(array_2d2, array_2d1);
      detection(array_2d1, &count);
    }
  }*/

  while (end) {
    if (test_amount % 2 == 0) {
      erode(array_2d1, array_2d2);
      detection(array_2d2, &count, &end);
    }
    else {
      erode(array_2d2, array_2d1);
      detection(array_2d1, &count, &end);
    }
    test_amount++; 
  }
  
  
  if (test_amount % 2 == 0) {
    get_output_image(output_image, array_2d1);
  } else {
    get_output_image(output_image, array_2d2);
  }
  
  
  //Save image to file
  write_bitmap(input_image, argv[2]);
  
  printf("Count is: %d\n", count);


  printf("Done!");
  return 0;
}
