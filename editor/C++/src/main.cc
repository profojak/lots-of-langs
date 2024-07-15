import Input;
import Output;
import Terminal;

using namespace editor;

/*! @brief Main function. */
int main(int argc, char *argv[]) {
    if(argc >= 2)
        OpenFile(argv[1]);

    SetStatusMessage("Ctrl-S = save | Ctrl-Q = quit");

    while(true) {
        RefreshScreen();
        ProcessKeypress();
    }

    return 0;
}
