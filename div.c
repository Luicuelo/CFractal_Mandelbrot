#include "div.h"
#ifndef __cplusplus
    #include <stdbool.h>
#endif

bool invierte;

typedef struct tagBITMAPINFO_E
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[ncolores];
}
BITMAPINFO_E;

BITMAPINFO_E  bmi;
BYTE Pixels[hgt][wid];


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
        if (x+w>wid-1) w=wid-1-x;
        if (y+h>hgt-1) h=hgt-1-y;
        for (a=0;a<(h);a++)
            memset (&Pixels[y+a][x],color,w);
}

 void bordeR(int x,int y,int w,int h,BYTE color)
{
        
        int a;

        if (x<0) x=0;
        if (y<0) y=0;
        if (x+w>wid-1) w=wid-1-x;
        if (y+h>hgt-1) h=hgt-1-y;

        for (a=1;a<(h-1);a++){
            memset (&Pixels[y+a][x],color,1);
            memset (&Pixels[y+a][x+w-1],color,1);
        }
            memset (&Pixels[y+0][x],color,w);
            memset (&Pixels[y+h-1][x],color,w);
}

int colini=30;

void llenaColores(void){
    int i;

      bmi.bmiColors[0].rgbRed = 0;
      bmi.bmiColors[0].rgbGreen = 0;
      bmi.bmiColors[0].rgbBlue = 0;
      bmi.bmiColors[0].rgbReserved = 0;
    for (i=1;i<ncolores;i++)
    {
            int color=colini+i+130;
            int color2=colini+i;
            if (color>254) color=color-254;
            if (color2>254) color2=color2-254;
            bmi.bmiColors[i].rgbRed = color2;
            bmi.bmiColors[i].rgbGreen = color2;
            bmi.bmiColors[i].rgbBlue = color;
            bmi.bmiColors[i].rgbReserved = 0;
    }

}

void _llenaColores2(void)
{
    int c;
    int i;
    if (colini-256>30) colini=30;
    for (i=0;i<ncolores;i++)
    {
        c = i + colini;
        if (c > 255)
        {
            c-= 256;
        }

        if (invierte && (i!=colMandel)) {
                c=255-c;
        }
        if (c>100)
        {
            bmi.bmiColors[i].rgbRed = (c-40);
            bmi.bmiColors[i].rgbGreen = (c-60);
        }
        else
        {

            bmi.bmiColors[i].rgbRed = c/2;
            bmi.bmiColors[i].rgbGreen = c/2;
            bmi.bmiColors[i].rgbBlue = c;
        }
        bmi.bmiColors[i].rgbReserved = 0;
    }
}


void CreateDIB(HWND hwndParent)
{
    RECT  rectP;
    GetWindowRect(hwndParent, &rectP);
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = wid;          // Width in pixels.
    bmi.bmiHeader.biHeight = -hgt;        // Height in pixels.
    bmi.bmiHeader.biPlanes = 1;           // 1 color plane.
    bmi.bmiHeader.biBitCount = bits;      // 8 bits per pixel.
    bmi.bmiHeader.biCompression = 0;      // BI_RGB; // No compression.
    bmi.bmiHeader.biSizeImage = 0;        // Unneeded with no compression.
    bmi.bmiHeader.biXPelsPerMeter = 0;    // Unneeded.
    bmi.bmiHeader.biYPelsPerMeter = 0;    // Unneeded.
    bmi.bmiHeader.biClrUsed = ncolores;   // # colors in color table
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
                    + wid*hgt;
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
        WriteFile(hFile, Pixels, wid*hgt, &bytesWritten, NULL);
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
	//BitBlt(hDC, 0, 0, wid, hgt, compat_dc, 0, 0,BLACKNESS); // Copy the compatible DC's image onto the window.
    BitBlt(hDC, 0, 0, wid, hgt, compat_dc, 0, 0,SRCCOPY); 

    DeleteDC (compat_dc); //Destroy the compatible DC.
    DeleteObject(dib);    ///destruir el Bitmap......
    ReleaseDC(hWnd,hDC);
}



// Implementadas como Macro para mayor rapidez.
// void rectangulo(int c,int f,int w,int h,BYTE color)
//{
//        int a;
//        if (f+h>hgt) h=hgt-f;
//        for (a=(wid-1-c-w);a<(wid-c);a++)
//            memset ((&Pixels[a][0])+f,color,h+1);
//}

//
//void setPixel(int x, int y, int color)
//{
//    int p;
//    p=wid-1-x;
//    Pixels[p][y] =color;
//}
