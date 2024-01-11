#include "constantes.h"
#include "stsbar.h"

#ifndef __cplusplus
    #include <stdbool.h>
#endif



HWND  hWndStatusbar;

BOOL CreateSBar(HWND hwndParent,char *initialText,int nrOfParts)
{


    RECT  rectP;
    RECT  rect;
	int cx;
	int cy;
	unsigned int flags;
	flags= SWP_NOOWNERZORDER;
	
    hWndStatusbar = CreateStatusWindow(WS_CHILD | WS_VISIBLE | WS_BORDER,
                                       initialText,
                                       hwndParent,
                                       IDM_STATUSBAR);
   

    

    GetWindowRect(hwndParent, &rectP);
    GetWindowRect(hWndStatusbar, &rect);
	cx=rectP.right-rectP.left;
	cy=(rectP.bottom-rectP.top)+(rect.bottom-rect.top);
    SetWindowPos(hwndParent,0,0,0,cx,cy,flags);

    if(hWndStatusbar)
    {
        InitializeStatusBar(hwndParent,nrOfParts);
        UpdateStatusBar(initialText, 0, 0);
        return TRUE;
    }

    return FALSE;
}


void InitializeStatusBar(HWND hwndParent,int nrOfParts)
{
    //const int cSpaceInBetween = 8;
    int   ptArray[40];   // Array defining the number of parts/sections
    HDC   hDC;

    /* * Fill in the ptArray...  */

    hDC = GetDC(hwndParent);

    //RECT  rect;
    //GetClientRect(hwndParent, &rect);

    ptArray[0] =wid/2;
    ptArray[1] = wid;


    //---TODO--- Add code to calculate the size of each part of the status
    // bar here.

    ReleaseDC(hwndParent, hDC);
    SendMessage(hWndStatusbar,
                SB_SETPARTS,
                nrOfParts,
                (LPARAM)(LPINT)ptArray);


    //---TODO--- Add code to update all fields of the status bar here.
    // As an example, look at the calls commented out below.

    //    UpdateStatusBar("Cursor Pos:", 1, SBT_POPOUT);
    //    UpdateStatusBar("Time:", 3, SBT_POPOUT);
}




/*------------------------------------------------------------------------
 Procedure:     UpdateStatusBar ID:1
 Purpose:       Updates the statusbar control with the appropiate
                text
 Input:         lpszStatusString: Charactar string that will be shown
                partNumber: index of the status bar part number.
                displayFlags: Decoration flags
 Output:        none
 Errors:        none

------------------------------------------------------------------------*/
void UpdateStatusBar(LPSTR lpszStatusString, WORD partNumber, WORD displayFlags)
{
    SendMessage(hWndStatusbar,
                SB_SETTEXT,
                partNumber | displayFlags,
                (LPARAM)lpszStatusString);

}
