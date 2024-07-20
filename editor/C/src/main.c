#include "input.h"
#include "output.h"
#include "terminal.h"

/*! @brief Main function. */
int main(int argc, char *argv[]) {
    EnableRawMode();
    InitEditor();
    if (argc >= 2)
        OpenFile(argv[1]);

    SetStatusMessage("Ctrl-S = save | Ctrl-Q = quit");

    while (1) {
        RefreshScreen();
        ProcessKeypress();
    }

    return 0;
}
