module;

#include <iostream>
#include <optional>

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

module Terminal;

namespace editor {

class Editor editor;

Editor::Editor() : cursor_x{0}, cursor_y{0}, rendered_x{0}, screen_rows{0},
    screen_cols{0}, row_offset{0}, col_offset{0}, lines_count{0}, dirty{0},
    status_time{0} {
    if(tcgetattr(STDIN_FILENO, &original_termios) == -1) {
        std::cerr << "Error getting terminal attributes" << std::endl;
        std::terminate();
    }

    struct termios new_termios = original_termios;
    new_termios.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    new_termios.c_oflag &= ~(OPOST);
    new_termios.c_cflag |= (CS8);
    new_termios.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    new_termios.c_cc[VMIN] = 0;
    new_termios.c_cc[VTIME] = 1;

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_termios) == -1) {
        std::cerr << "Error setting terminal attributes" << std::endl;
        std::terminate();
    }

    if(auto result = GetWindowSize()) {
        screen_rows = result->first - 2;
        screen_cols = result->second;
    } else {
        std::cerr << "Error getting window size" << std::endl;
        std::terminate();
    }
}

Editor::~Editor() {
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1) {
        std::cerr << "Error setting terminal attributes" << std::endl;
        std::terminate();
    }
}

std::optional<std::pair<int, int>> GetCursorPosition() noexcept {
    std::string buffer;
    char ch;

    std::cout << "\x1b[6n" << std::flush;
    while(std::cin.get(ch) && ch != 'R')
        buffer.push_back(ch);

    if(buffer.size() < 2 || buffer[0] != '\x1b' || buffer[1] != '[')
        return std::nullopt;

    int rows, cols;
    if (sscanf(buffer.c_str() + 2, "%d;%d", &rows, &cols) != 2)
        return std::nullopt;

    return std::make_pair(rows, cols);
}

std::optional<std::pair<int, int>> GetWindowSize() noexcept {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        std::cout << "\x1b[999C\x1b[999B" << std::flush;
        return GetCursorPosition();
    } else
        return std::make_pair(ws.ws_row, ws.ws_col);
}

} // namespace editor
