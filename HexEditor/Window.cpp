// Window.cpp
#include "Window.h"			// Header-Datei der Window-Klasse
#include "resource.h"		// Ressourcen-Header (Men�-IDs)
#include "FileHandler.h"	// Header f�r die Dateiverwaltung
#include <commdlg.h>		// Funktionen f�r Common Dialogs (�ffnen/Speichern)
#include <commctrl.h>		// Funktionen f�r Common Controls (Status Bar)
#include <string>
#include <algorithm>

using namespace std;

#pragma comment(lib, "comctl32.lib") // Verlinkt die comctl32.lib f�r Common Controls

// Statische Fensterprozedur, die als Einstiegspunkt f�r Fensternachrichten dient
LRESULT CALLBACK Window::WindowProcStatic(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

}
