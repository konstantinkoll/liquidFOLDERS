
// FootherGraph.h: Schnittstelle für Methoden zur Darstellung von Diagrammen
//

#define GraphSpacer     3

void FinishGraph(CDC& dc, CRect rect, BOOL Themed);
void DrawSolidColor(CDC& dc, CRect rect, COLORREF clr, BOOL Themed);
void DrawLegend(CDC& dc, CRect& rect, COLORREF clr, CString Text, BOOL Themed);
void DrawGraphCaption(CDC&, CRect& rect, UINT nID);
void DrawBarChart(CDC& dc, CRect& rect, INT64* Values, COLORREF* Colors, UINT Count, UINT Height, BOOL Themed);
void DrawChartLegend(CDC& dc, CRect& rect, INT64 Count, INT64 Size, COLORREF clr, CString Legend, BOOL Themed);
