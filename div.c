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

// Set a single pixel with the specified color
void drawPixel(int x, int y, BYTE color) {
    if (x >= 0 && x < WINDOW_WIDTH && y >= 0 && y < WINDOW_HEIGHT) {
        Pixels[y][x] = color;
    }
}

void color(int i,BYTE r,BYTE g, BYTE b)
{
    bmi.bmiColors[i].rgbRed = r;
    bmi.bmiColors[i].rgbGreen = g;
    bmi.bmiColors[i].rgbBlue = b;
    bmi.bmiColors[i].rgbReserved = 0;
}

// Draw a filled square/rectangle with the specified color
void drawSquare(int x, int y, int w, int h, BYTE color) {
    int a;

    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + w > WINDOW_WIDTH - 1) w = WINDOW_WIDTH - 1 - x;
    if (y + h > WINDOW_HEIGHT - 1) h = WINDOW_HEIGHT - 1 - y;
    for (a = 0; a < h; a++)
        memset(&Pixels[y + a][x], color, w);
}

// Draw a border/outline rectangle with the specified color
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
}

// Fill the color palette with smooth gradient colors
void fillColors(void) {
    int i;

    bmi.bmiColors[0].rgbRed = 0;
    bmi.bmiColors[0].rgbGreen = 0;
    bmi.bmiColors[0].rgbBlue = 0;
    bmi.bmiColors[0].rgbReserved = 0;

    bmi.bmiColors[255].rgbRed = 0;
    bmi.bmiColors[255].rgbGreen = 0;
    bmi.bmiColors[255].rgbBlue = 0;
    bmi.bmiColors[255].rgbReserved = 0;

    for (int i = 1; i < COLOR_COUNT - 1; i++) {
        int red = (int)(127.5 * (1 + cos(i * 0.1)));
        int green = (int)(127.5 * (1 + cos(i * 0.1 + 2 * M_PI / 3)));
        int blue = (int)(127.5 * (1 + cos(i * 0.1 + 4 * M_PI / 3)));

        if (invertColors){
            red=255-red;
            green=255-green;
            blue=255-blue;
        }

        int ca = (i + color_offset) % COLOR_COUNT;
        bmi.bmiColors[ca].rgbRed = red;
        bmi.bmiColors[ca].rgbGreen = green;
        bmi.bmiColors[ca].rgbBlue = blue;
        bmi.bmiColors[ca].rgbReserved = 0;
    }
}

// Fill the color palette with alternate color scheme
void fillColorsAlternate(void) {
    int i;

    for (i = 0; i < COLOR_COUNT; i++) {
        bmi.bmiColors[i].rgbRed = i % 256;
        bmi.bmiColors[i].rgbGreen = (i * 2) % 256;
        bmi.bmiColors[i].rgbBlue = (i * 3) % 256;
        bmi.bmiColors[i].rgbReserved = 0;
    }
}

void createDIB(HWND hwndParent)
{
    RECT  rectP;
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
    DWORD bytesWritten;
    BOOL writeResult;

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
    HDC compat_dc;
    HDC hDC = GetDC(hWnd);
    HBITMAP dib;
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


