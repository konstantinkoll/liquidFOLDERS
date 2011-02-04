
// FootherGraph.h: Schnittstelle für Methoden zur Darstellung von Diagrammen
//

#define GraphSpacer     3

void Finish(CDC& dc, CRect rect, BOOL Themed);
void DrawSolidColor(CDC& dc, CRect rect, COLORREF clr, BOOL Themed);
void DrawLegend(CDC& dc, CRect& rect, COLORREF clr, CString Text, BOOL Themed);
