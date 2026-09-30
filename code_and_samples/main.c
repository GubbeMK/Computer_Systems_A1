//To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
//To run (linux/mac): ./main.out example.bmp example_inv.bmp

//To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
//To run (win): main.exe example.bmp example_inv.bmp

#include <stdlib.h>
#include <stdio.h>
#include "cbmp.h"
#include <stdbool.h>

  unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
  unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
  unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH];
  unsigned char erosion_image[BMP_WIDTH][BMP_HEIGTH];
  int cell_count = 0;
  int grayscale_increment = 50 ;
  bool white_found = false ;

  typedef struct {
    int x;
    int y;
  } cell_locations ;

  cell_locations locations[340] ;
  cell_locations *p = locations ;

void grayscale(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH]) {
  int x = 0 ;
  int y = 0 ;
  int offset = 10;          
  int absolute_min = 40;

  while (y < BMP_HEIGTH/grayscale_increment) {
    int RGB_total = 0 ;
    for (int i = x * grayscale_increment ; i < x * grayscale_increment + grayscale_increment ; i++ ) {
      for (int j = y * grayscale_increment ; j < y * grayscale_increment + grayscale_increment ; j++) {
        RGB_total += (input_image[i][j][0]+input_image[i][j][1]+input_image[i][j][2])/3 ;
      }
    }

    int RGB_average = RGB_total/(grayscale_increment*grayscale_increment) ;
    for (int i = x * grayscale_increment ; i < x * grayscale_increment + grayscale_increment ; i++ ) {
      for (int j = y * grayscale_increment ; j < y * grayscale_increment + grayscale_increment ; j++) {
        int gray_px = (input_image[i][j][0] + input_image[i][j][1] + input_image[i][j][2]) / 3;
        binary_image[i][j] = (gray_px > RGB_average + offset && gray_px > absolute_min) ? 255 : 0;
        }
      }
    x++ ;

    if (x >= BMP_WIDTH/grayscale_increment) {
      y++ ;
      x = 0 ;
    }
  }
}

void erode(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH]) {
  for (int x = 0; x < BMP_WIDTH; x++) {
    for (int y = 0; y < BMP_HEIGTH; y++) {
      unsigned char result = input_image[x][y]; // start from the actual pixel, not from nothing

      if (result == 255) {
        for (int dx = -1; dx <= 1; dx++) {
          if (dx == 0) continue;
          int nx = x + dx;
          if (nx < 0 || nx >= BMP_WIDTH || input_image[nx][y] == 0) {
            result = 0;
          }
        }
        for (int dy = -1; dy <= 1; dy++) {
          if (dy == 0) continue;
          int ny = y + dy;
          if (ny < 0 || ny >= BMP_HEIGTH || input_image[x][ny] == 0) {
            result = 0;
          }
        }
      }

      output_image[x][y] = result;
      if (result == 255) {
        white_found = true;
      }
    }
  }
}

void detection(unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH], cell_locations *f) {
  int x = 0 ;
  int y = 0 ;
  int detection_length = 13 ;
  int RMP = detection_length ;
  int TMP = detection_length ;

  while (y <= BMP_HEIGTH-detection_length) 
  {
    int new_RMP = detection_length ;
    int new_TMP = detection_length ;
    bool border_clear = 1 ;
    bool center_full = 0 ;
    for (int dy = 0 ; dy <= detection_length ; dy++) 
    {
      for (int dx = 0 ; dx <= detection_length ; dx++) 
      {
        if (binary_image[x+dx][y+dy] == 255) 
        {
          if (dx == 0 || dx == detection_length || dy == 0 || dy == detection_length) 
          {
            border_clear = 0 ;
          } else {
            center_full = 1 ;
          }
          if (new_RMP > dx) {
            if (dx - 1 > 1) {
              new_RMP = dx - 1 ;
            } else {
              new_RMP = 1 ;
            }
          }

          if (new_TMP > dy && dy > 1) {
            if (dy - 1 > 1) {
               new_TMP = dy - 1 ;
            } else {
              new_TMP = 1 ;
            }
          }
        }
      }
    }
    if (border_clear == 1 && center_full == 1) {
      cell_count++ ;
      new_TMP = detection_length ;
      new_RMP = detection_length ;
      p->x = x+detection_length/2 ;
      p->y = y+detection_length/2 ;
      p++ ;
      for (int yy = 1 ; yy <= detection_length - 1 ; yy++ ) 
      {
        for (int xx = 1 ; xx <= detection_length - 1 ; xx++) 
        {
          binary_image[x+xx][y+yy] = 0 ;
        }
      }
    }

    if (new_RMP < RMP) {
      RMP = new_RMP ;
    }

    if (new_TMP < TMP) {
      TMP = new_TMP ;
    }

    if (x == BMP_WIDTH - detection_length) {
      if (y == BMP_HEIGTH - detection_length) {
        break ; 
      }
      x = 0 ;
      y += TMP ;
      if (y > BMP_HEIGTH - detection_length) {
        y = BMP_HEIGTH - detection_length ;
      }
      TMP = detection_length ;
    } else {
      x += RMP ;
      if (x > BMP_WIDTH - detection_length) {
        x = BMP_WIDTH - detection_length ;
      }
      RMP = detection_length ;
    }
  }
}

void mark_cells(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], const cell_locations *f) {
  for (int i = 0; i < cell_count; i++) {
    for (int width = -1; width < 2; width++) {
      for (int dy = -10; dy <= 10; dy++) {
        int px = locations[i].x + width;
        int py = locations[i].y + dy;
        if (px >= 0 && px < BMP_WIDTH && py >= 0 && py < BMP_HEIGTH) {
          input_image[px][py][0] = 255;
          input_image[px][py][1] = 0;
          input_image[px][py][2] = 0;
        }
      }
    }
    for (int width = -1; width < 2; width++) {
      for (int dx = -10; dx <= 10; dx++) {
        int px = locations[i].x + dx;
        int py = locations[i].y + width;
        if (px >= 0 && px < BMP_WIDTH && py >= 0 && py < BMP_HEIGTH) {
          input_image[px][py][0] = 255;
          input_image[px][py][1] = 0;
          input_image[px][py][2] = 0;
        }
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

  read_bitmap(argv[1], input_image);

  grayscale(input_image, binary_image);

  bool cells_left = true ;
  while (cells_left) {
    erode(binary_image, erosion_image) ;
    if (white_found == false ) {
       cells_left = false ;
    }
    white_found = false ;

    detection(erosion_image, locations) ;    

    erode(erosion_image, binary_image) ;
    if (white_found == false) {
       cells_left = false ;
    }
    white_found = false ;

    detection(binary_image, locations) ;
  }

  mark_cells(input_image, locations) ;
  printf("Cells found: ") ;
  printf("%d\n", cell_count) ;
  
  write_bitmap(input_image, argv[2]);

  printf("Done!\n");
  return 0;
}
