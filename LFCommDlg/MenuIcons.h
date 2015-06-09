
// MenuIcons.h: Schnittstelle für Methoden zur Darstellung von Menu-Icons
//

void SetMenuItemBitmap(HMENU hMenu, UINT item, HBITMAP hBmp);
HBITMAP IconToBitmap(HICON hIcon, INT cx, INT cy);
HBITMAP SetMenuItemIcon(HMENU hMenu, UINT item, HICON hIcon, INT cx, INT cy);
HBITMAP SetMenuItemIcon(HMENU hMenu, UINT item, WORD ResID);
