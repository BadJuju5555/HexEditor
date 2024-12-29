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

// Methode zur Handhabung der vertikalen Srollaktion (WM_SCROLL)
void Window::OnVScroll(HWND hwnd, HWND hWndCtl, UINT code, int pos)
{
	// Abrufen der aktuellen Scroll-Information
	SCROLLINFO si = { sizeof(si) };
	si.fMask = SIF_ALL;
	GetScrollInfo(hwnd, SB_VERT, &si);
	int yOld = scrollY;		// Speichern der alten Scrollposition

	// Verarbeiten der Scrollaktion basierend auf dem Code
	switch (code)
	{
	case SB_TOP:
		scrollY = 0;
		break;
	case SB_BOTTOM:
		scrollY = maxScrollY;								// Scrollen zum Ende
		break;
	case SB_LINEUP:
		scrollY = max(scrollY - 1, 0);						// Eine Zeile nach oben scrollen
		break;
	case SB_LINEDOWN:
		scrollY = min(scrollY + 1, maxScrollY);				// Eine Zeile nach unten scrollen
		break;
	case SB_PAGEUP:
		scrollY = max(scrollY - si.nPage, 0);				// Eine Seite nach oben scrollen
		break;
	case SB_PAGEDOWN:
		scrollY = min(scrollY + si.nPage, maxScrollY);		// Eine Seite nach unten scrollen
		break;
	case SB_THUMBTRACK:
		scrollY = si.nTrackPos;								// Scrollen zu einer Position, die durch Ziehen des Scroll-Griffs bestimmt wird
		break;
	}


	// Wenn die Scrollposition sich geändert hat, aktualisieren und neu zeichnen
	if (scrollY != yOld)
	{
		SetScrollPos(hwnd, SB_VERT, scrollY, TRUE);		// Setzen der neuen Scrollposition
		InvalidateRect(hwnd, NULL, TRUE);				// Neuzeichnen des Fensters
	}
}

// Methode zur Handhabung der horizontalen Scrollaktion (WM_HSCROLL)
void Window::OnHScroll(HWND hwnd, HWND hWndCtl, UINT code, int pos)
{
	// Abrufen der aktuellen Scroll-Informationen
	SCROLLINFO si = { sizeof(si) };
	si.fMask = SIF_ALL;
	GetScrollInfo(hwnd, SB_HORZ, &si);
	int xOld = scrollX; // Speichern der alten Scrollposition

	// Verarbeiten der Scrollaktion basierend auf dem Code
	switch (code)
	{
	case SB_LEFT:
		scrollX = 0; // Scrollen zum linken Ende
		break;
	case SB_RIGHT:
		scrollX = maxScrollX; // Scrollen zum rechten Ende
		break;
	case SB_LINELEFT:
		scrollX = max(scrollX - 10, 0); // Um 10 Einheiten nach links scrollen
		break;
	case SB_LINERIGHT:
		scrollX = min(scrollX + 10, maxScrollX); // Um 10 Einheiten nach rechts scrollen
		break;
	case SB_PAGELEFT:
		scrollX = max(scrollX - si.nPage, 0); // Eine Seite nach links scrollen
		break;
	case SB_PAGERIGHT:
		scrollX = min(scrollX + si.nPage, maxScrollX); // Eine Seite nach rechts scrollen
		break;
	case SB_THUMBTRACK:
		scrollX = si.nTrackPos; // Scrollen zu einer Position, die durch Ziehen des Scroll-Griffs bestimmt wird
		break;
	}

	// Wenn die Scrollposition sich geändert hat, aktualisieren und neu zeichnen
	if (scrollX != xOld)
	{
		SetScrollPos(hwnd, SB_HORZ, scrollX, TRUE); // Setzen der neuen Scrollposition
		InvalidateRect(hwnd, NULL, TRUE);           // Neuzeichnen des Fensters
	}
}

// Methode zur Handhabung von Befehlen (z.B. Menüaktion) (WM_COMMAND)
void Window::OnCommand(HWND hwnd, int id, HWND hWndCtl, UINT codeNotify)
{
	switch (id)
	{
		case ID_MENU_FILE_OPEN:
		{
			// Öffnen eines Datei-Dialogs
			wstring filename = OpenFileDialog(hwnd);
			if (!filename.empty())
			{
				// Laden der ausgewählten Datei im HexEditor
				if (hexEditor.LoadFile(filename))
				{
					// Aktualisieren der Scrollbars basierend auf der neuen Dateigröße
					RECT clientRect;
					GetClientRect(hwnd, &clientRect);
					OnSize(hwnd, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
				}
				else
				{
					// Fehlermeldung anzeigen, wenn die Datei nicht geöffnet werden konnte
					MessageBox(hwnd, L"Datei konnte nicht geöffnet werden.", L"Fehler", MB_OK | MB_ICONERROR);
				}
			}
		}
		break;

		case ID_MENU_FILE_SAVE:
		{
			// Öffnen eines Datei-Speichern-Dialogs
			wstring filename = OpenSaveFileDialog(hwnd);
			if (!filename.empty())
			{
				// Speichern der aktuellen Daten im HexEditor
				if (!hexEditor.SaveFile(filename))
				{
					// Fehlermeldung anzeigen, wenn die Datei nicht gespeichert werden konnte
					MessageBox(hwnd, L"Datei konnte nicht gespeichert werden.", L"Fehler", MB_OK | MB_ICONERROR);
				}
			}
		}
		break;

		case ID_MENU_FILE_EXIT:
			PostQuitMessage(0); // Beenden der Anwendung
			break;
	}
}

// Methode zur Handhabung von Mausklicks mit der linken Taste (WM_LBUTTONDOWN)
void Window::OnLButtonDown(HWND hwnd, int x, int y, UINT keyFlags)
{
	// Umrechnung der Mausklick-Position basierend auf der aktuellen Scrollposition
	int actualX = x / charWidth + scrollX;
	int actualY = y / lineHeight + scrollY;

	// Berechnung, welche Zeile und Spalte angeclickt wurde
	int line = actualY;
	int column = (actualX - (addressWidth / charWidth)) / 3;		// 3 Zeichen pro Byte (z.B "FF ")

	// Berechnung des Byte-Index basierend auf Zeile und Spalte
	size_t index = line * bytesPerLine + column;
	if (index < hexEditor.GetSize())
	{
		selectedIndex = index;		// Setzen des ausgewählten Byte-Index
		isEditing = true;			// Aktivieren des Bearbeitungsmodus
		editPos = 0;				// Initialisieren des Bearbeitungsmodus
		editBuffer[0] = L'\0';		// Leeren des Bearbeitungsbuffers
		editBuffer[1] = L'\0';
		editBuffer[2] = L'\0';

		// Setzen des Fokus auf das Fenster, um Tastatureingaben zu empfangen
		SetFocus(hwnd);
		InvalidateRect(hwnd, NULL, TRUE);		// Neuzeichnen des Fensters
		UpdateStatusBar();						// Aktualisieren der Statusleiste
	}
}

// Methode zur Handhabung von Zeichen-Eingaben (WM_CHAR)
void Window::OnChar(HWND hwnd, TCHAR ch, int cRepeat)
{
	// Überprüfung, ob der Bearbeitungsmodus aktiv ist und der gewählte Index gültig ist
	if (isEditing && selectedIndex < hexEditor.GetSize())
	{
		if (ch == VK_BACK)
		{
			// Bearbeitung der Rücktaste (Löschen des letzten eingegebenen Zeichens)
			if (editPos > 0)
			{
				editPos--;
				editBuffer[editPos] = L'\0';		// Entfernen des letzten Zeichens
				InvalidateRect(hwnd, NULL, TRUE);	// Neuzeichnen des Fensters
			}
		}
		else if (isxdigit(ch) && editPos < 2)
		{
			// Überprüfung, ob das eingegebene Zeichen ein hexadezimaler Wert ist und der Bearbeitungsbuffer noch Platz hat
			editBuffer[editPos++] = towupper(ch);		// Speichern des eingegebenen Zeichens in Großbuchstaben
			editBuffer[editPos] = L'\0';				// Null-terminieren des Buffers


			if (editPos == 2)
			{
				// Wenn zwei hexadezimale Zeichen eingegeben wurden, konvertiere sie in einen Byte-Wert
				wchar_t temp[3] = { editBuffer[0], editBuffer[1], L'\0' };
				unsigned int byte;
				swscanf_s(temp, L"%x", &byte); // Lesen des hexadezimalen Werts

				// Setzen des Byte-Werts im HexEditor
				hexEditor.SetByte(selectedIndex, static_cast<unsigned char>(byte));

				// Beenden des Bearbeitungsmodus
				isEditing = false;
				selectedIndex = SIZE_MAX;
			}

			// Neuzeichnen des Fensters und Aktualisieren der Statusleiste
			InvalidateRect(hwnd, NULL, TRUE);
			UpdateStatusBar();
		}
	}
}

// Methode zur Aktualisierung der Statusleiste mit Informationen zur aktuellen Auswahl
void Window::UpdateStatusBar()
{
	if (hStatus)
	{
		wstring status;
		if (selectedIndex < hexEditor.GetSize())
		{
			// Formatieren des Status-Texts mit der Adresse un dem Byte-Wert
			wchar_t buffer[100];
			swprintf_s(buffer, 100, L"Adresse: 0x%08llX  Byte: %02X", selectedIndex, hexEditor.GetByte(selectedIndex));
			status = buffer;
		}
		else
		{
			status = L"Keine Auswahl"; // Kein ausgewähltes Byte
		}

		// Setzen des Textes in der Statusleiste
		SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)status.c_str());
	}
}




