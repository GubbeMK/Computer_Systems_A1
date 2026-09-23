//To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
//To run (linux/mac): ./main.out example.bmp example_inv.bmp

//To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
//To run (win): main.exe example.bmp example_inv.bmp

#include <stdlib.h>
#include <stdio.h>
#include "cbmp.h"

unsigned char array_2d1[BMP_WIDTH][BMP_HEIGTH];
unsigned char array_2d2[BMP_WIDTH][BMP_HEIGTH];

unsigned char binary_threshold(unsigned char gray_px, unsigned char threshold) {
  if (gray_px >= threshold) {
    return 255;
  } else {
    return 0;
  }
}

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

void erode1(unsigned char array_2d1[BMP_WIDTH][BMP_HEIGTH], unsigned char array_2d2[BMP_WIDTH][BMP_HEIGTH]) {
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      int left  = (x > 0) ? array_2d1[x-1][y] : 0;
      int right = (x < BMP_WIDTH-1) ? array_2d1[x+1][y] : 0;
      int up    = (y > 0) ? array_2d1[x][y-1] : 0;
      int down  = (y < BMP_HEIGTH-1) ? array_2d1[x][y+1] : 0;
      
      if (array_2d1[x][y] && left && right && up && down) {
        array_2d2[x][y] = 255;
      } else {
        array_2d2[x][y] = 0;
      }
    }
  }
}

void erode2(unsigned char array_2d1[BMP_WIDTH][BMP_HEIGTH], unsigned char array_2d2[BMP_WIDTH][BMP_HEIGTH], int *count) {
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      int left  = (x > 0) ? array_2d2[x-1][y] : 0;
      int right = (x < BMP_WIDTH-1) ? array_2d2[x+1][y] : 0;
      int up    = (y > 0) ? array_2d2[x][y-1] : 0;
      int down  = (y < BMP_HEIGTH-1) ? array_2d2[x][y+1] : 0;
      
      if (array_2d2[x][y] && left && right && up && down) {
        array_2d1[x][y] = 255;
      } 
      else if (array_2d2[x][y] && !left && !right && !up && !down) 
      {
        array_2d1[x][y] = 0;
        (*count)++;
      } else {
        array_2d1[x][y] = 0;
      }
    }
  }
}

void detection(unsigned char array_2d1[BMP_WIDTH][BMP_HEIGTH]) {
  int detection_width = 12+2;
  int detection_height = 12+2;
  int c = 0;

  
  
  for (int x = 0+c; x < BMP_WIDTH-detection_width; x++) {
    for (int y = 0+c; y < BMP_HEIGTH-detection_height; y++) {
      for (int w = x; w < 14+x; w++) {
        for (int h = y; h < 14+y; w++) {
          if (h == x && array_2d1[])
        }
      }
    }
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
  unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
  unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];

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

  for (int test = 0; test < 12; test++) {
    if (test % 2 == 0) {
      erode1(array_2d1, array_2d2);
    }
    else {
      erode2(array_2d1, array_2d2, &count);
    }
  }

  printf("Count is: %d\n", count);
  
  get_output_image(output_image, array_2d1);

  //Save image to file
  write_bitmap(output_image, argv[2]);

  printf("Done!\n");
  return 0;
}
