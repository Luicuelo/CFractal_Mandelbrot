#include "div.h"
#ifndef __cplusplus
    #include <stdbool.h>
#endif
#include <math.h> // Asegúrate de incluir esta cabecera para las funciones sin y cos

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


bool invierte;

typedef struct tagBITMAPINFO_E
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[color_count];
}
BITMAPINFO_E;

BITMAPINFO_E  bmi;
BYTE Pixels[window_height][window_width];

int color_offset;
int temp_rect;

void color(int i,BYTE r,BYTE g, BYTE b)
{
    bmi.bmiColors[i].rgbRed = r;
    bmi.bmiColors[i].rgbGreen = g;
    bmi.bmiColors[i].rgbBlue = b;
    bmi.bmiColors[i].rgbReserved = 0;
}

 void cuadradoR(int x,int y,int w,int h,BYTE color)
{
        
        int a;

        if (x<0) x=0;
        if (y<0) y=0;
        if (x+w>window_width-1) w=window_width-1-x;
        if (y+h>window_height-1) h=window_height-1-y;
        for (a=0;a<(h);a++)
            memset (&Pixels[y+a][x],color,w);
}

 void bordeR(int x,int y,int w,int h,BYTE color)
{
        
        int a;

        if (x<0) x=0;
        if (y<0) y=0;
        if (x+w>window_width-1) w=window_width-1-x;
        if (y+h>window_height-1) h=window_height-1-y;

        for (a=1;a<(h-1);a++){
            memset (&Pixels[y+a][x],color,1);
            memset (&Pixels[y+a][x+w-1],color,1);
        }
            memset (&Pixels[y+0][x],color,w);
            memset (&Pixels[y+h-1][x],color,w);
}



void llenaColores(void){
    int i;

      bmi.bmiColors[0].rgbRed = 0;
      bmi.bmiColors[0].rgbGreen = 0;
      bmi.bmiColors[0].rgbBlue = 0;
      bmi.bmiColors[0].rgbReserved = 0;

      bmi.bmiColors[255].rgbRed = 0;
      bmi.bmiColors[255].rgbGreen = 0;
      bmi.bmiColors[255].rgbBlue = 0;
      bmi.bmiColors[255].rgbReserved = 0;

    for (int i = 1; i < color_count-1; i++) {

        //int red = ((i+10) * 4) % color_count;  
        //int green = (i * 20) % color_count;  
        //int blue = (i * 21) % color_count;  

        // Generar colores suaves usando funciones seno y coseno
        int red = (int)(127.5 * (1 + cos(i * 0.1)));
        int green = (int)(127.5 * (1 + cos(i * 0.1 + 2 * M_PI / 3)));
        int blue = (int)(127.5 * (1 + cos(i * 0.1 + 4 * M_PI / 3)));

        int ca=(i+color_offset)  % color_count;
        bmi.bmiColors[ca].rgbRed = red;
        bmi.bmiColors[ca].rgbGreen = green;
        bmi.bmiColors[ca].rgbBlue = blue;
        bmi.bmiColors[ca].rgbReserved = 0;
    }
}


void CreateDIB(HWND hwndParent)
{
    RECT  rectP;
    GetWindowRect(hwndParent, &rectP);
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = window_width;          // Width in pixels.
    bmi.bmiHeader.biHeight = -window_height;        // Height in pixels.
    bmi.bmiHeader.biPlanes = 1;           // 1 color plane.
    bmi.bmiHeader.biBitCount = color_depth;      // 8 bits per pixel.
    bmi.bmiHeader.biCompression = 0;      // BI_RGB; // No compression.
    bmi.bmiHeader.biSizeImage = 0;        // Unneeded with no compression.
    bmi.bmiHeader.biXPelsPerMeter = 0;    // Unneeded.
    bmi.bmiHeader.biYPelsPerMeter = 0;    // Unneeded.
    bmi.bmiHeader.biClrUsed = color_count;   // # colors in color table
    bmi.bmiHeader.biClrImportant = 0;     // # important colors. 0 means all.
    llenaColores();
}

void SaveDib (LPCTSTR lpszFileName, BOOL bOverwriteExisting)
{
     DWORD bytesWritten;

    // The .BMP file format is as follows:
    // BITMAPFILEHEADER / BITMAPINFOHEADER / color table / pixel data

    // Now we can put them in a file.
    HANDLE hFile = CreateFile(lpszFileName, GENERIC_WRITE,FILE_SHARE_READ, NULL, bOverwriteExisting ? CREATE_ALWAYS : CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        // .BMP file begins with a BITMAPFILEHEADER,
        BITMAPFILEHEADER bmfh;
        bmfh.bfType = MAKEWORD('B','M');
        bmfh.bfSize = sizeof(BITMAPFILEHEADER)
                    + sizeof(BITMAPINFOHEADER)
                    + (bmi.bmiHeader.biClrUsed * sizeof(RGBQUAD))
                    + window_width*window_height;
        bmfh.bfReserved1 = 0;
        bmfh.bfReserved2 = 0;
        bmfh.bfOffBits = sizeof(BITMAPFILEHEADER)
                    + sizeof(BITMAPINFOHEADER)
                    + (bmi.bmiHeader.biClrUsed * sizeof(RGBQUAD));

        WriteFile(hFile, &bmfh, sizeof(BITMAPFILEHEADER),&bytesWritten, NULL);
        // Then it's followed by the BITMAPINFOHEADER structure
        WriteFile(hFile, &(bmi.bmiHeader), sizeof(BITMAPINFOHEADER),&bytesWritten,NULL);

        // Then the colour table.
        WriteFile(hFile, bmi.bmiColors, bmi.bmiHeader.biClrUsed * sizeof(RGBQUAD),&bytesWritten,NULL);

        // Then the pixel data.
        WriteFile(hFile, Pixels, window_width*window_height, &bytesWritten, NULL);
        CloseHandle(hFile);
    }
}


/*Subroutine DrawDIB draws the DIB onto the program's form.
'It creates a compatible device context,
'uses SelectObject to copy the DIB into the device context,
'uses BitBlt to copy the device context's picture onto the form,
'and calls DeleteDC to delete the device context.
*/
void DrawDIB(HWND hWnd)
{
    HDC compat_dc;
    HDC hDC = GetDC(hWnd);
    HBITMAP dib;
    compat_dc = CreateCompatibleDC(hDC); // Create a compatible device context.

    dib=CreateDIBitmap(hDC,
                         &(bmi.bmiHeader), CBM_INIT, Pixels,
                         (BITMAPINFO*)&bmi, DIB_RGB_COLORS);
    SelectObject (compat_dc, dib); //Select the DIB into the compatible DC.

    //int success;
    //success =
	//BitBlt(hDC, 0, 0, window_width, window_height, compat_dc, 0, 0,BLACKNESS); // Copy the compatible DC's image onto the window.
    BitBlt(hDC, 0, 0, window_width, window_height, compat_dc, 0, 0,SRCCOPY); 

    DeleteDC (compat_dc); //Destroy the compatible DC.
    DeleteObject(dib);    ///destruir el Bitmap......
    ReleaseDC(hWnd,hDC);
}



// Implementadas como Macro para mayor rapidez.
// void rectangulo(int c,int f,int w,int h,BYTE color)
//{
//        int a;
//        if (f+h>window_height) h=window_height-f;
//        for (a=(window_width-1-c-w);a<(window_width-c);a++)
//            memset ((&Pixels[a][0])+f,color,h+1);
//}

//
//void setPixel(int x, int y, int color)
//{
//    int p;
//    p=window_width-1-x;
//    Pixels[p][y] =color;
//}
