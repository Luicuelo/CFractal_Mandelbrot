#include "div.h"
#ifndef __cplusplus
    #include <stdbool.h>
#endif
#include <math.h>
#include <stdio.h>
#include "stsbar.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int max_iterations;
bool invertColors;

typedef struct tagBITMAPINFO_E
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[COLOR_COUNT];
}
BITMAPINFO_E;

BITMAPINFO_E  bmi;
BYTE Pixels[WINDOW_HEIGHT][WINDOW_WIDTH];

int color_offset;
int temp_rect;

// Draw a filled square/rectangle.
void drawSquare(int x, int y, int tam, BYTE iterations){

    BYTE color;
	if(iterations==MANDELBROTPOINT_VALUE) color= (COLOR_COUNT-1); //point in set.
	else color = (BYTE)((double)iterations * (COLOR_COUNT-1) / (double)max_iterations);	
    // Scale color based on max_iterations for better distribution

    if(tam==1){
        if (x >= 0 && x < WINDOW_WIDTH && y >= 0 && y < WINDOW_HEIGHT) {
            Pixels[y][x] = color;
        }
    }
    else{
        int w=tam;
        int h=tam;
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x + w > WINDOW_WIDTH - 1) w = WINDOW_WIDTH - 1 - x;
        if (y + h > WINDOW_HEIGHT - 1) h = WINDOW_HEIGHT - 1 - y;
        for (int a = 0; a < h; a++)
            memset(&Pixels[y + a][x], color, w);
    }
}

void color(int i,BYTE r,BYTE g, BYTE b)
{
    bmi.bmiColors[i].rgbRed = r;
    bmi.bmiColors[i].rgbGreen = g;
    bmi.bmiColors[i].rgbBlue = b;
    bmi.bmiColors[i].rgbReserved = 0;
}

// Draw a border/outline rectangle with the specified color
/*
void drawBorder(int x, int y, int w, int h, BYTE color) {
    int a;

    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + w > WINDOW_WIDTH - 1) w = WINDOW_WIDTH - 1 - x;
    if (y + h > WINDOW_HEIGHT - 1) h = WINDOW_HEIGHT - 1 - y;

    for (a = 1; a < (h - 1); a++) {
        memset(&Pixels[y + a][x], color, 1);
        memset(&Pixels[y + a][x + w - 1], color, 1);
    }
    memset(&Pixels[y + 0][x], color, w);
    memset(&Pixels[y + h - 1][x], color, w);
}*/

// Fill the color palette with smooth gradient colors
void fillColors(void) {
    int i = 0;

    bmi.bmiColors[0].rgbRed = 0;
    bmi.bmiColors[0].rgbGreen = 0;
    bmi.bmiColors[0].rgbBlue = 0;
    bmi.bmiColors[0].rgbReserved = 0;

    //Mandelbrot set Color
    bmi.bmiColors[COLOR_COUNT-1].rgbRed = 0;
    bmi.bmiColors[COLOR_COUNT-1].rgbGreen = 0;
    bmi.bmiColors[COLOR_COUNT-1].rgbBlue = 0;
    bmi.bmiColors[COLOR_COUNT-1].rgbReserved = 0;

    
    for (int i = 0; i < (COLOR_COUNT - 2); i++) {

        int green = i % 256;
        int blue = (i * 2) % 256;
        int red = (i / 2) % 256;
   
     //int green = (int)(127.5 * (1 + cos(i * 0.1 + 2 * M_PI / 3)));
   
        red = (red % 256);
        green = (green % 256);
        blue = (blue % 256);

        if (invertColors){
            red=255-red;
            green=255-green;
            blue=255-blue;
        }

        int ca = ((i + color_offset) % (COLOR_COUNT - 2)) + 1;
        //char debug_msg[50];
        //sprintf(debug_msg, "i=%d, ca=%d\n", i, ca);
        //OutputDebugString(debug_msg);
        bmi.bmiColors[ca].rgbRed = red;
        bmi.bmiColors[ca].rgbGreen = green;
        bmi.bmiColors[ca].rgbBlue = blue;
        bmi.bmiColors[ca].rgbReserved = 0;
    }
}

void createDIB(HWND hwndParent)
{
    RECT rectP = {0};
    GetWindowRect(hwndParent, &rectP);
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = WINDOW_WIDTH;          // Width in pixels.
    bmi.bmiHeader.biHeight = -WINDOW_HEIGHT;        // Height in pixels.
    bmi.bmiHeader.biPlanes = 1;           // 1 color plane.
    bmi.bmiHeader.biBitCount = COLOR_DEPTH;      // 8 bits per pixel.
    bmi.bmiHeader.biCompression = 0;      // BI_RGB; // No compression.
    bmi.bmiHeader.biSizeImage = 0;        // Unneeded with no compression.
    bmi.bmiHeader.biXPelsPerMeter = 0;    // Unneeded.
    bmi.bmiHeader.biYPelsPerMeter = 0;    // Unneeded.
    bmi.bmiHeader.biClrUsed = COLOR_COUNT;   // # colors in color table
    bmi.bmiHeader.biClrImportant = 0;     // # important colors. 0 means all.
    fillColors();
}

// Save the current fractal image as a BMP file
void saveFractal(LPCTSTR lpszFileName, BOOL bOverwriteExisting) {
    DWORD bytesWritten = 0;
    BOOL writeResult = FALSE;

    if (lpszFileName == NULL) return;

    HANDLE hFile = CreateFile(lpszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, bOverwriteExisting ? CREATE_ALWAYS : CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        BITMAPFILEHEADER bmfh;
        bmfh.bfType = MAKEWORD('B', 'M');
        bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (bmi.bmiHeader.biClrUsed * sizeof(RGBQUAD)) + WINDOW_WIDTH * WINDOW_HEIGHT;
        bmfh.bfReserved1 = 0;
        bmfh.bfReserved2 = 0;
        bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (bmi.bmiHeader.biClrUsed * sizeof(RGBQUAD));

        writeResult = WriteFile(hFile, &bmfh, sizeof(BITMAPFILEHEADER), &bytesWritten, NULL);
        if (writeResult && bytesWritten == sizeof(BITMAPFILEHEADER)) {
            writeResult = WriteFile(hFile, &(bmi.bmiHeader), sizeof(BITMAPINFOHEADER), &bytesWritten, NULL);
        }
        if (writeResult && bytesWritten == sizeof(BITMAPINFOHEADER)) {
            writeResult = WriteFile(hFile, bmi.bmiColors, bmi.bmiHeader.biClrUsed * sizeof(RGBQUAD), &bytesWritten, NULL);
        }
        if (writeResult && bytesWritten == (bmi.bmiHeader.biClrUsed * sizeof(RGBQUAD))) {
            WriteFile(hFile, Pixels, WINDOW_WIDTH * WINDOW_HEIGHT, &bytesWritten, NULL);
        }
        CloseHandle(hFile);
    }
}


// Draw the complete fractal image to the window
void drawFractal(HWND hWnd) {
    HDC compat_dc = NULL;
    HDC hDC = GetDC(hWnd);
    HBITMAP dib = NULL;
    compat_dc = CreateCompatibleDC(hDC); // Create a compatible device context.

    dib = CreateDIBitmap(hDC,
                         &(bmi.bmiHeader), CBM_INIT, Pixels,
                         (BITMAPINFO*)&bmi, DIB_RGB_COLORS);
    SelectObject(compat_dc, dib); // Select the DIB into the compatible DC.

    BitBlt(hDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, compat_dc, 0, 0, SRCCOPY);

    DeleteDC(compat_dc); // Destroy the compatible DC.
    DeleteObject(dib);   // Destroy the Bitmap.
    ReleaseDC(hWnd, hDC);
}


