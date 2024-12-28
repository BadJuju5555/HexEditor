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
	Window* pThis = nullptr;	// Ptr auf die Instanz der Window-Klasse

	// Überprüfung, ob die Nachricht WM_NCCREATE ist (Fenster wird erstellt)
	if (uMsg == WM_NCCREATE)
	{
		// CAST der lParam zu CREATESTRUCT, um auf die Erstellungsdaten zuzugreifen
		CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);

		// Extrahieren des Zeigers auf die Window-Instanz aus lpCreateParams
		pThis = static_cast<Window*>(cs->lpCreateParams);

		// Speichern des Zeigers in den Fenster-Daten für späteren Zugriff
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
	}
	else
	{
		// Abrufen des Ptr auf die Window-Instanz aus den Fenster-Daten
		pThis = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}

	// Wenn der Zeiger gültig ist, leite die Nachricht an die Instanzfensterprozedur weiter
	if (pThis)
	{
		return pThis->WindowProc(hwnd, uMsg, wParam, lParam);
	}

	// Standardfensterprozedur für alle anderen Fälle
	return DefWindowProc(hwnd, uMsg, wParam, lParam);

}

// Konstruktor der Window-Klasse
Window::Window(HINSTANCE hInst, int nCmdShow)
	: hInstance(hInst), hwnd(NULL), hStatus(NULL),
	scrollX(0), scrollY(0), maxScrollX(0), maxScrollY(0),
	selectedIndex(SIZE_MAX), isEditing(false), editPos(0)
{
	// Initialisierung des Bearbeitungsbuffers mit null-terminierten Zeichen
	editBuffer[0] = L'\0';
	editBuffer[1] = L'\0';
	editBuffer[2] = L'\0';

}

// Destruktor der Window-Klasse
Window::~Window()
{
	// Ressourcen freigeben, falls erforderlich (z.B. dynamisch allokierter Speicher)
}

// Methode zur Erstellung des Hauptfensters
bool Window::Create()
{
	// Name der Fensterklasse
	const wchar_t CLASS_NAME[] = L"HexEditorWindowClass";

	// Definition der Fensterklasse
	WNDCLASS wc = { };
	wc.lpfnWndProc = WindowProcStatic;				// Statische Fensterprozedur
	wc.hInstance = hInstance;						// Anwendungshandle
	wc.lpszClassName = CLASS_NAME;					// Name der Fensterklasse
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);		// Standard-Cursor
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);		// Standard Icon
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);	// Hintergrundfarbe

	// Registrierung der Fensterklasse
	if (!RegisterClass(&wc))
	{
		// Fehlermeldung anzeigen, wenn die Registrierung fehlschlägt
		MessageBox(NULL, L"Fensterklasse konnte nicht registriert werden.", L"Fehler", MB_OK | MB_ICONERROR);

		return false;
	}

	// Laden des Menüs aus den Ressourcen
	HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1));
	if (!hMenu)
	{
		// Fehlermeldung anzeigen, wenn das Menü nicht geladen werden kann
		MessageBox(NULL, L"Menü konnte nicht geladen werden.", L"Fehler", MB_OK | MB_ICONERROR);
		return false;
	}

	// Erstellung des Fensters mit den definierten EIgenschaften
	hwnd = CreateWindowEx(
		0,														// Erweiterte Fensterstile (keine)
		CLASS_NAME,												// Name der Fensterklasse
		L"Hex-Editor",											// Fenstertitel
		WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,			// Fensterstile (normal, vertikale & horizontale Scrollbars)
		CW_USEDEFAULT, CW_USEDEFAULT,							// Standardposition
		800, 600,												// Fenstergröße (Breite, Höhe)
		NULL,													// Kein übergeordnetes Fenster
		hMenu,													// Geladenes Menü
		hInstance,												// Anwendungshandle
		this													// Ptr auf die Window-Instanz (wichtig für die Fensterprozedur)
	);

	// Überprüfung, ob das Fenster erfolgreich erstellt wurde
	if (hwnd == NULL)
	{
		// Fehlermeldung anzeigen, wenn das Fenster nicht erstellt werden kann
		MessageBox(NULL, L"Fenster konnte nicht erstellt werden.", L"Fehler", MB_OK | MB_ICONERROR);

		return false;
	}

	// Fenster anzeigen und aktualisieren
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	return true; // Erfolg
}

// Methode zur Nachrichten-Schleife (wird aufgerufen, um die Anwendung laufen zu lassen)
void Window::Run()
{
	MSG msg = { };

	// Schleife, die Nachrichten abruft und verarbeitet, bis WM_QUIT empfangen wird
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);							// Übersetzt Tastaturnachrichten
		DispatchMessage(&msg);							//Sendet die Nachricht an die Fensterprozedur
	}
}

// Methode zum Öffnen eines Datei-Öffnen-Dialogs
wstring Window::OpenFileDialog(HWND hwnd)
{
	wchar_t szFileName[MAX_PATH] = L"";					// Puffer für den Dateipfad

	OPENFILENAME ofn = { };
	ofn.lStructSize = sizeof(ofn);						// Größe der Struktur
	ofn.hwndOwner = hwnd;								// Übergeordnetes Fenster
	ofn.lpstrFilter = L"Alle Dateien\0*.*\0";			// Filter für die Dateiauswahl
	ofn.lpstrFile = szFileName;							// Puffer für den Dateipfad
	ofn.nMaxFile = MAX_PATH;							// Maximale Anzahl der Zeichen im Pfad
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;	// Flags (Datei muss existieren, Ausblenden der "Nur Lesen" -Option)
	ofn.lpstrDefExt = L"";								// Standard-Dateierweiterung (keine)

	// Anzeigen des Öffnen-Dialogs
	if (GetOpenFileName(&ofn))
	{
		return wstring(szFileName);						// Rückgabe des ausgewählten Dateipfads
	}

	return L"";			// Rückgabe eines leeren Strings, wenn kein Dateipfad ausgewählt wurde
}

// Methode zum Öffnen eines Datei-Speichern-Dialogs
wstring Window::OpenSaveFileDialog(HWND hwnd)
{
	wchar_t szFileName[MAX_PATH] = L"";					// Puffer für den Dateipfad

	OPENFILENAME ofn = { };
	ofn.lStructSize = sizeof(ofn);						// Größe der Struktur
	ofn.hwndOwner = hwnd;								// Übergeordnetes Fenster
	ofn.lpstrFilter = L"Alle Dateien\0*.*\0";			// Filter für die Dateiauswahl
	ofn.lpstrFile = szFileName;							// Puffer für den Dateipfad
	ofn.nMaxFile = MAX_PATH;							// Maximale Anzahl der Zeichen im Pfad
	ofn.Flags = OFN_OVERWRITEPROMPT;					// Flag (Warnung vor Überschreiben existierender Dateien)
	ofn.lpstrDefExt = L"";								// Standard-Dateierweiterung (keine)

	// Anzeige des Speichern-Dialogs
	if (GetSaveFileName(&ofn))
	{
		return wstring(szFileName);		// Rückgabe des ausgewählten Dateipfads
	}

	return L"";			// Rückgabe eines leeren Strings, wenn kein Dateipfad ausgewählt wurde
}

// Methode die aufgerufen wird, wenn das Fenster erstellt wird (WM_CREATE)
void Window::OnCreate(HWND hwnd)
{
	// Erstellen der Statusleiste am unteren Rand des Fensters
	hStatus = CreateWindowEx(
		0,											// Erweiterte Fensterstile (keine)
		STATUSCLASSNAME,							// Name der Statusleistenklasse
		NULL,										// Kein Fenstertitel
		WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,		// Fensterstile (Childfenster, sichtbar, Größe-Griff)
		0, 0, 0, 0,									// Position und Größe
		hwnd,										// Übergeordnetes Fenster (Hauptfenster)
		NULL,										// Kein Menü oder Steuerung
		hInstance,									// Anwendungshandle
		NULL										// Keine zusätzlichen Daten
	);
}

// Methode, die aufgerufen wird, wenn das Fenster zerstört wird (WM_DESTROY)
void Window::OnDestroy(HWND hwnd)
{
	PostQuitMessage(0);		// Sendet eine WM_QUIT Nachricht, um die Anwendung zu beenden
}

// Methode zur Handhabung der Größenänderung des Fensters (WM_SIZE)
void Window::OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	// Anpassung der Statusleiste an die neue Fenstergröße
	if (hStatus)
	{
		SendMessage(hStatus, WM_SIZE, 0, 0);
	}

	// Berechnung der Anzahl der sichtbaren Zeilen basierend auf der Fensterhöhe
	int visibleLines = (cy - lineHeight) / lineHeight;

	// Berechnung der Gesamtanzahl der Zeilen basierend auf der Dateigröße und Bytes pro Zeile
	int totalLines = (hexEditor.GetSize() + bytesPerLine - 1) / bytesPerLine;

	// Bestimmung der maximalen vertikalrn Scrollposition
	maxScrollY = max(0, totalLines - visibleLines);

	// Berechnung der Breite der hexadezimalen Datenanzeige( Adresse + Hex + ASCII)
	int hexDataWidth = addressWidth + (3 * bytesPerLine) + bytesPerLine + 1;

	// Berechnung der sichtbaren Spalten basierend auf der Fensterbreite
	int visibleColumns = cx / charWidth;

	// Bestimmung der maximalen horizontalen Scrollposition
	maxScrollX = max(0, hexDataWidth - visibleColumns);


	// Einstellen der Scroll-Informationen für die vertikale Scrollbar
	SCROLLINFO siV = { sizeof(SCROLLINFO) };
	siV.fMask = SIF_RANGE | SIF_PAGE;
	siV.nMin = 0;
	siV.nMax = maxScrollY;
	siV.nPage = visibleLines;
	SetScrollInfo(hwnd, SB_VERT, &siV, TRUE);

	// Einstellen der Scroll-Informationen für die horizontale Scrollbar
	SCROLLINFO siH = { sizeof(SCROLLINFO) };
	siH.fMask = SIF_RANGE | SIF_PAGE;
	siH.nMin = 0;
	siH.nMax = hexDataWidth;
	siH.nPage = visibleColumns;
	SetScrollInfo(hwnd, SB_HORZ, &siH, TRUE);

	// Neuzeichnen des gesamten Fensters
	InvalidateRect(hwnd, NULL, TRUE);
}


