//
// WINDOW_H
//
#ifndef WINDOW_H
#define WINDOW_H

#include <Windows.h>
#include <vector>
#include <string>
#include "HexEditor.h"

using namespace std;

class Window
{
public:
	Window(HINSTANCE hInstance, int nCmdShow);  // Konstruktor	
	~Window();	// Destruktor
	bool Create();
	void Run();


private:
	static LRESULT CALLBACK WindowProcStatic(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnPaint(HWND hwnd);
	void OnSize(HWND hwnd, UINT state, int cx, int cy);
	void OnVScroll(HWND hwnd, HWND hWndCtl, UINT code, int pos);
	void OnHScroll(HWND hwnd, HWND hWndCtl, UINT code, int pos);
	void OnCommand(HWND hwnd, int id, HWND hWndCtl, UINT codeNotify);
	void OnLButtonDown(HWND hwnd, int x, int y, UINT keyFlags);
	void OnChar(HWND hwnd, TCHAR ch, int cRepeat);
	void OnCreate(HWND hwnd);
	void OnDestroy(HWND hwnd);

	void UpdateStatusBar();
	wstring OpenFileDialog(HWND hwnd);
	wstring OpenSaveFileDialog(HWND hwnd);

	HINSTANCE hInstance;
	HWND hwnd;
	HWND hStatus;
	HexEditor hexEditor;

	// Scroll-Parameter
	int scrollX;
	int scrollY;
	int maxScrollX;
	int maxScrollY;

	// Dims.
	static const int bytesPerLine = 16;
	static const int charWidth = 10; // Char breite
	static const int lineHeight = 20; // Zeilen Höhe 
	static const int addressWidth = 80; // Breite der Address Zeilen

	// Edit Tools
	size_t selectedIndex;
	bool isEditing;
	wchar_t editBuffer[3];
	int editPos;

};
//
#endif // WINDOW_H
//
