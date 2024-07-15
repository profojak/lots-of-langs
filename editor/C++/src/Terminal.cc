module;

#include <array>
#include <iostream>
#include <optional>

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

module Terminal;

namespace editor {

class Editor editor;

Editor::Editor() noexcept : cursor_x{0}, cursor_y{0}, rendered_x{0},
    screen_rows{0}, screen_cols{0}, row_offset{0}, col_offset{0},
    dirty{0}, status_time{0} {
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

Editor::~Editor() noexcept {
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1) {
        std::cerr << "Error setting terminal attributes" << std::endl;
        std::terminate();
    }
}

auto ReadKey() noexcept -> int {
    int nread;
    char c;
    while((nread = read(STDIN_FILENO, &c, 1)) != 1)
        if(nread == -1 && errno != EAGAIN) {
            std::cerr << "Error reading key" << std::endl;
            std::terminate();
        }

    if(c == '\x1b') {
        std::array<char, 3> seq;
        if(!std::cin.get(seq[0]) || !std::cin.get(seq[1]))
            return '\x1b';

        if(seq[0] == '[') {
            if(seq[1] >= '0' && seq[1] <= '9') {
                if(!std::cin.get(seq[2]))
                    return '\x1b';
                if(seq[2] == '~')
                    switch(seq[1]) {
                        case '1': return HOME;
                        case '3': return DELETE;
                        case '4': return END;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME;
                        case '8': return END;
                    }
            } else
                switch(seq[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME;
                    case 'F': return END;
                }
        } else if(seq[0] == 'O')
            switch(seq[1]) {
                case 'H': return HOME;
                case 'F': return END;
            }
        
        return '\x1b';
    } else
        return c;
}

auto GetCursorPosition() noexcept -> std::optional<std::pair<int, int>> {
    std::string buffer;
    char ch;

    std::cout << "\x1b[6n" << std::flush;
    while(std::cin.get(ch) && ch != 'R')
        buffer.push_back(ch);

    if(buffer.size() < 2 || buffer[0] != '\x1b' || buffer[1] != '[')
        return std::nullopt;

    int rows, cols;
    if(sscanf(buffer.c_str() + 2, "%d;%d", &rows, &cols) != 2)
        return std::nullopt;

    return std::make_pair(rows, cols);
}

auto GetWindowSize() noexcept -> std::optional<std::pair<int, int>> {
    struct winsize ws;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        std::cout << "\x1b[999C\x1b[999B" << std::flush;
        return GetCursorPosition();
    } else
        return std::make_pair(ws.ws_row, ws.ws_col);
}

} // namespace editor
