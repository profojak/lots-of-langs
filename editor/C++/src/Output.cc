module;

#include <chrono>
#include <iostream>
#include <ranges>
#include <string>

import Terminal;

module Output;

namespace editor {

/*! @brief Editor version. */
constexpr const std::string_view VERSION{"0.0.1"};

void Scroll() {
    editor.rendered_x = 0;
    if(editor.cursor_y < editor.lines_count)
        editor.rendered_x = editor.lines[editor.cursor_y].CursorXToRenderedX(
            editor.cursor_x);
    
    if(editor.cursor_y < editor.row_offset)
        editor.row_offset = editor.cursor_y;
    if(editor.cursor_y >= editor.row_offset + editor.screen_rows)
        editor.row_offset = editor.cursor_y - editor.screen_rows + 1;
    if(editor.rendered_x < editor.col_offset)
        editor.col_offset = editor.rendered_x;
    if(editor.rendered_x >= editor.col_offset + editor.screen_cols)
        editor.col_offset = editor.rendered_x - editor.screen_cols + 1;
}

void DrawLines(std::string& buffer) {
    for(const auto& y : std::ranges::views::iota(0, editor.screen_rows)) {
        auto file_line = y + editor.row_offset;
        if(file_line >= editor.lines_count) {
            if(editor.lines_count == 0 && y == editor.screen_rows / 3) {
                std::string welcome = "Text editor - version " +
                    std::string{ VERSION };
                if(welcome.size() >
                    static_cast<unsigned long>(editor.screen_cols))
                    welcome.resize(editor.screen_cols);
                auto padding = (editor.screen_cols - welcome.size()) / 2;
                if(padding) {
                    buffer.append("~");
                    padding--;
                }
                buffer.append(std::string(padding, ' '));
                buffer.append(welcome);
            } else
                buffer.append("~");
        } else {

        }

        buffer.append("\x1b[K\r\n");
    }
}

void DrawStatusBar(std::string& buffer) {
    buffer.append("\x1b[K\r\n");
}

void DrawMessageBar(std::string& buffer) {
    buffer.append("\x1b[K");
    if(editor.status_message.size() >
        static_cast<unsigned long>(editor.screen_cols))
        editor.status_message.resize(editor.screen_cols);
    auto now = std::chrono::system_clock::now();
    if(std::chrono::system_clock::to_time_t(now) - editor.status_time < 5)
        buffer.append(editor.status_message);
}

void RefreshScreen() {
    Scroll();

    std::string buffer;
    buffer.append("\x1b[?25l\x1b[H");

    DrawLines(buffer);
    DrawStatusBar(buffer);
    DrawMessageBar(buffer);

    buffer.append("\x1b[?25h");
    std::cout << buffer << std::flush;
}

} // namespace editor
