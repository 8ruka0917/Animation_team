#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "led-matrix-c.h"
#include "mnb_bmp.h"

int main(int argc, char const **argv)
{
  struct RGBLedMatrixOptions options;
  struct RGBLedMatrix *matrix;
  struct LedCanvas *offscreen_canvas;
  char rgb_sequence[] = "GBR";

  memset(&options, 0, sizeof(options));
  options.rows = options.cols = 64;
  options.led_rgb_sequence = rgb_sequence;
  matrix = led_matrix_create_from_options(&options, &argc, (char ***)&argv);
  if (matrix == NULL)
    return 1;
  offscreen_canvas = led_matrix_create_offscreen_canvas(matrix);

  pthread_t recthread;
  if (pthread_create(&recthread, NULL, (void *)takeTimelapse, NULL))
  {
    return -1;
  }

  int width, height;
  led_canvas_get_size(offscreen_canvas, &width, &height);


  //あらかじめ宣言されいて使用可能な変数は，以下の4つ
  //  int width, heigh; //width=64, height=64です
  //  struct RGBLedMatrix *matrix; //LEDディスプレイパネルを表す変数
  //  struct LedCanvas *offscreen_canvas; //バッファ
  //LEDディスプレイパネルを光らせるのに使用する特別な関数は以下の4つのみ
  //  void led_canvas_clear(struct LedCanvas *canvas); //LEDディスプレイパネル前面を黒にする
  //  void led_canvas_set_pixel(struct LedCanvas *canvas, int x, int y,
  //                            uint8_t r, uint8_t g, uint8_t b); //canvasのx,yにr,g,bの色を出す
  //  struct LedCanvas *led_matrix_swap_on_vsync(struct RGBLedMatrix *matrix,
  //                                             struct LedCanvas *canvas); //canvasの内容をLEDパネルに転送
  //  int usleep(useconds_t usec); //usecマイクロ秒待機する．usecは1000000（1秒）まで

  #include <math.h>
  led_canvas_clear(offscreen_canvas); //canvasをクリア
  int i, j, k, y, x, f, s;
  double r;
  r = 25;
  double n = 3.3;
  int frames[150][64][64][3];

  int p, q;
  img bmp_img_data;                   //BMPファイルを読み込むためのimg変数
  bmp_img_data.width = 0;             //img変数を初めて使うとき，これを付けて．
    bmp_img_data.height = 0;            //img変数を初めて使うとき，これを付けて．
    bmp_img_data.data = NULL;           //初めて使うとき，これを付けて．
    ReadBmp2("cat.bmp", &bmp_img_data); //BMPファイルの読み込み．


    for (f = 0; f < 32; ++f)
    {
      //背景
      for (i = 0; i < 64 * 64; ++i)
      {
        frames[f][i / 64][i % 64][0] = 0;
        frames[f][i / 64][i % 64][1] = 255;
        frames[f][i / 64][i % 64][2] = 0;
      }

      //楕円
      for (i = 0 - 32; i <= 32; i++)
      {
        for (j = 0 - 32; j <= 32; ++j)
        {
          if (j * j + i * i * n * n - r * r <= r * n)
          {
            x = j + 32;
            y = i + 32;

            frames[f][x][y][0] = 0;
            frames[f][x][y][1] = 0;
            frames[f][x][y][2] = 0;
          }
        }
      }

      //ねこ
      int a = 26;
      int b = 28;
      for (j = 0; j < a; j++)
      {
        for (k = 0; k < f; k++)
        {
          int index = f - k;
          if (index < 0 || index >= b)
            continue;
          int red = bmp_img_data.data[index][j].r;
          int green = bmp_img_data.data[index][j].g;
          int blue = bmp_img_data.data[index][j].b;
          if (red == 0 && green == 0 && blue == 255)
            continue;
          x = 32 - a / 2 + j;
          y = 32 - k;
          frames[f][x][y][0] = red;
          frames[f][x][y][1] = green;
          frames[f][x][y][2] = blue;
        }
      }
    }

  while(1==1){
    for (p = 0; p < 32; p++)
    {
      led_canvas_clear(offscreen_canvas);
      for (s = 0; s < 64; ++s)
        for (q = 0; q < 64; ++q)
          led_canvas_set_pixel(offscreen_canvas, s, q, frames[p][s][q][0], frames[p][s][q][1], frames[p][s][q][2]);
      led_matrix_swap_on_vsync(matrix, offscreen_canvas);
      usleep(100000);
    }
    for (p = 32 - 1; p >= 0; p--)
    {
    led_canvas_clear(offscreen_canvas);
    for (s = 0; s < 64; ++s)
      for (q = 0; q < 64; ++q)
        led_canvas_set_pixel(offscreen_canvas, s, q, frames[p][s][q][0], frames[p][s][q][1], frames[p][s][q][2]);
        led_matrix_swap_on_vsync(matrix, offscreen_canvas);
        usleep(100000);
      }
    }
    return 0;
}
