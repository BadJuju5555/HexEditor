#include "Window.h"
#include <CommCtrl.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    // Initialisiere Common Controls
    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_BAR_CLASSES };
    InitCommonControlsEx(&icex);

    // Erstelle das Fenster
    Window win(hInstance, nCmdShow);
    if (!win.Create())
    {
        return 0;
    }

    // Starte die Nachrichten-Schleife
    win.Run();

    return 0;
}
