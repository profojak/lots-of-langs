module;

#include <iostream>
#include <ranges>
#include <string>

import Terminal;

module Output;

namespace editor {

void DrawLines(std::string& buffer) {
    for([[maybe_unused]] const auto& y :
        std::ranges::views::iota(0, editor.screen_rows)) {
        buffer.append("~");
        buffer.append("\x1b[K\r\n");
    }
}

void DrawStatusBar(std::string& buffer) {
    buffer.append("\x1b[K\r\n");
}

void DrawMessageBar(std::string& buffer) {
    buffer.append("\x1b[K");
    buffer.append(editor.status_message);
}

void RefreshScreen() {
    std::string buffer;
    buffer.append("\x1b[?25l\x1b[H");

    DrawLines(buffer);
    DrawStatusBar(buffer);
    DrawMessageBar(buffer);

    buffer.append("\x1b[?25h");
    std::cout << buffer << std::flush;
}

} // namespace editor
