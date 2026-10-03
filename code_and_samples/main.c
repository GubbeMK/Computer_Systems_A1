//To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
//To run (linux/mac): ./main.out example.bmp example_inv.bmp

//To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
//To run (win): main.exe example.bmp example_inv.bmp

#include <stdlib.h>
#include <stdio.h>
#include "cbmp.h"
#include <string.h>

unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char watershed_array[BMP_WIDTH][BMP_HEIGTH];

void cross(int x, int y) {
  int thickness = 1; 

  for (int t = -thickness; t <= thickness; t++) {
    // Horizontal arm
    for (int l = -8; l < 8; l++) {
      if (x+l >= 0 && x+l < BMP_WIDTH && y+t >= 0 && y+t < BMP_HEIGTH) {
        input_image[x+l][y+t][0] = 255;
        input_image[x+l][y+t][1] = 0;
        input_image[x+l][y+t][2] = 0;
      }
    }
    // Vertical arm
    for (int v = -8; v < 8; v++) {
      if (y+v >= 0 && y+v < BMP_HEIGTH && x+t >= 0 && x+t < BMP_WIDTH) {
        input_image[x+t][y+v][0] = 255;
        input_image[x+t][y+v][1] = 0;
        input_image[x+t][y+v][2] = 0;
      }
    }
  }
}

void watershed(unsigned char in[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char ws[BMP_WIDTH][BMP_HEIGTH]) {
  for (int x = 0; x < BMP_WIDTH; x++) {
    for (int y = 0; y < BMP_HEIGTH; y++) {
      if ((in[x][y][0]+in[x][y][1]+in[x][y][2])/ 3 < 95) {
        ws[x][y] = 0;
      } else {
        int l  = (x > 0) ? ws[x-1][y] : 0;
        int u  = (y > 0) ? ws[x][y-1] : 0;
        int tl = (x > 0 && y > 0) ? ws[x-1][y-1] : 0;
        int bl = (x > 0 && y < BMP_HEIGTH-1) ? ws[x-1][y+1] : 0;

        int lowest_side   = ((l < u) ? l : u) + 3;
        int lowest_corner = ((tl < bl) ? tl : bl) + 4;
        ws[x][y] = (lowest_side < lowest_corner) ? lowest_side : lowest_corner;
      }
    }
  }
}

void watershed_reverse(unsigned char in[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char ws[BMP_WIDTH][BMP_HEIGTH]) {
  for (int x = BMP_WIDTH-1; x >= 0; x--) {
    for (int y = BMP_HEIGTH-1; y >= 0; y--) {
      if ((in[x][y][0]+in[x][y][1]+in[x][y][2])/ 3 < 95) {
        ws[x][y] = 0;
      } else {
        int r  = (x < BMP_WIDTH-1) ? ws[x+1][y] : 0;
        int d  = (y < BMP_HEIGTH-1) ? ws[x][y+1] : 0;
        int br = (x < BMP_WIDTH-1 && y < BMP_HEIGTH-1) ? ws[x+1][y+1] : 0;
        int tr = (x < BMP_WIDTH-1 && y > 0) ? ws[x+1][y-1] : 0;

        int lowest_side   = ((r < d) ? r : d) + 3;
        int lowest_corner = ((br < tr) ? br : tr) + 4;

        int v = ws[x][y]; 
        if (lowest_side < v)   v = lowest_side;
        if (lowest_corner < v) v = lowest_corner;
        ws[x][y] = v;
      }
    }
  }
}

void find_peaks(unsigned char in[BMP_WIDTH][BMP_HEIGTH], int *count) {
  int maxv = 0;
  for (int x = 0; x < BMP_WIDTH; x++)
    for (int y = 0; y < BMP_HEIGTH; y++)
      if (in[x][y] > maxv) maxv = in[x][y];

  int min_peak = 12;   // ignore anything smaller than this, tune it

  for (int level = maxv; level >= min_peak; level--) {
    for (int x = 0; x < BMP_WIDTH; x++) {
      for (int y = 0; y < BMP_HEIGTH; y++) {
        int c = in[x][y];
        if (c != level) continue;

        int l  = (x > 0) ? in[x-1][y] : 0;
        int r  = (x < BMP_WIDTH-1) ? in[x+1][y] : 0;
        int u  = (y > 0) ? in[x][y-1] : 0;
        int d  = (y < BMP_HEIGTH-1) ? in[x][y+1] : 0;
        int tl = (x > 0 && y > 0) ? in[x-1][y-1] : 0;
        int tr = (x < BMP_WIDTH-1 && y > 0) ? in[x+1][y-1] : 0;
        int bl = (x > 0 && y < BMP_HEIGTH-1) ? in[x-1][y+1] : 0;
        int br = (x < BMP_WIDTH-1 && y < BMP_HEIGTH-1) ? in[x+1][y+1] : 0;

        if (c >= l && c >= r && c >= u && c >= d &&
            c >= tl && c >= tr && c >= bl && c >= br) {
          cross(x, y);
          (*count)++;

          int half = c / 3 + 3;
          int start = (y - half < 0) ? 0 : y - half;
          int end   = (y + half >= BMP_HEIGTH) ? BMP_HEIGTH - 1 : y + half;

          for (int i = x - half; i <= x + half; i++) {
            if (i < 0 || i >= BMP_WIDTH) continue;
            memset(&in[i][start], 0, (end - start + 1) * sizeof(in[0][0]));
          }
        }
      }
    }
  }
}

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

  read_bitmap(argv[1], input_image);

  int count = 0;

  watershed(input_image, watershed_array);
  watershed_reverse(input_image, watershed_array);
  find_peaks(watershed_array, &count);

  write_bitmap(input_image, argv[2]);
  
  printf("Count is: %d\n", count);


  printf("Done!\n");
  return 0;
}