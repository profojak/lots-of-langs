import Input;
import Output;
import Terminal;

using namespace editor;

/*! @brief Main function. */
int main() {
    SetStatusMessage("Ctrl-S = save | Ctrl-Q = quit");

    while(true) {
        RefreshScreen();
        ProcessKeypress();
    }

    return 0;
}
