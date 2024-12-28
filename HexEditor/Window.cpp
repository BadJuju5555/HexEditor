// Window.cpp
#include "Window.h"			// Header-Datei der Window-Klasse
#include "resource.h"		// Ressourcen-Header (Menü-IDs)
#include "FileHandler.h"	// Header für die Dateiverwaltung
#include <commdlg.h>		// Funktionen für Common Dialogs (Öffnen/Speichern)
#include <commctrl.h>		// Funktionen für Common Controls (Status Bar)
#include <string>
#include <algorithm>

using namespace std;

#pragma comment(lib, "comctl32.lib") // Verlinkt die comctl32.lib für Common Controls

// Statische Fensterprozedur, die als Einstiegspunkt für Fensternachrichten dient
LRESULT CALLBACK Window::WindowProcStatic(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

}
