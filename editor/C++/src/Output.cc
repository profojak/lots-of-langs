module;

#include <chrono>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>

import Input;
import Terminal;

module Output;

namespace editor {

/*! @brief Editor version. */
constexpr const std::string_view VERSION{"0.0.1"};

void Scroll() noexcept {
    editor.rendered_x = 0;
    if(static_cast<unsigned long>(editor.cursor_y) < editor.lines.size())
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

void DrawLines(std::string& buffer) noexcept {
    for(const auto& y : std::ranges::views::iota(0, editor.screen_rows)) {
        auto file_line = y + editor.row_offset;
        if(static_cast<unsigned long>(file_line) >= editor.lines.size()) {
            if(editor.lines.size() == 0 && y == editor.screen_rows / 3) {
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
            int length = editor.lines[file_line].rendered_characters.size() -
                editor.col_offset;
            if(length < 0)
                length = 0;
            if(length > editor.screen_cols)
                length = editor.screen_cols;
            buffer.append(editor.lines[file_line].rendered_characters,
                editor.col_offset, length);
        }

        buffer.append("\x1b[K\r\n");
    }
}

void DrawStatusBar(std::string& buffer) noexcept {
    buffer.append("\x1b[7m");

    std::string status = editor.filename.empty() ? "[No Name]" :
        editor.filename;
    status.append(" - ");
    status.append(std::to_string(editor.lines.size()));
    status.append(" lines");
    status.append(editor.dirty ? " (modified)" : "");

    std::string right_status = std::to_string(editor.cursor_y + 1);
    right_status.append("/");
    right_status.append(std::to_string(editor.lines.size()));

    if(status.size() > static_cast<unsigned long>(editor.screen_cols))
        status.resize(editor.screen_cols);
    buffer.append(status);
    int length = status.size();
    while(length < editor.screen_cols) {
        if(editor.screen_cols - status.size() == right_status.size()) {
            buffer.append(right_status);
            break;
        } else {
            buffer.append(" ");
            length++;
        }
    }

    buffer.append("\x1b[m\r\n");
}

void DrawMessageBar(std::string& buffer) noexcept {
    buffer.append("\x1b[K");
    if(editor.status_message.size() >
        static_cast<unsigned long>(editor.screen_cols))
        editor.status_message.resize(editor.screen_cols);
    auto now = std::chrono::system_clock::now();
    if(std::chrono::system_clock::to_time_t(now) - editor.status_time < 5)
        buffer.append(editor.status_message);
}

void RefreshScreen() noexcept {
    Scroll();

    std::string buffer;
    buffer.append("\x1b[?25l\x1b[H");

    DrawLines(buffer);
    DrawStatusBar(buffer);
    DrawMessageBar(buffer);

    buffer.append("\x1b[");
    buffer.append(std::to_string(editor.cursor_y - editor.row_offset + 1));
    buffer.append(";");
    buffer.append(std::to_string(editor.rendered_x - editor.col_offset + 1));
    buffer.append("H");

    buffer.append("\x1b[?25h");
    std::cout << buffer << std::flush;
}

void OpenFile(std::string_view filename) noexcept {
    editor.filename = filename;

    std::ifstream file{editor.filename, std::ios::in};
    if(!file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        std::terminate();
    }

    std::string line;
    while(std::getline(file, line)) {
        while(!line.empty() && (line.back() == '\n' || line.back() == '\r'))
            line.pop_back();
        InsertLine(editor.lines.size(), line);
    }

    editor.dirty = 0;
}

void SaveFile(void) noexcept {
    if(editor.filename.empty()) {
        editor.filename = Prompt("Save as: {}");
        if(editor.filename.empty()) {
            SetStatusMessage("Save aborted");
            return;
        }
    }

    std::ofstream file{editor.filename, std::ios::out};
    if(file.is_open()) {
        int length = 0;
        for(const auto& line : editor.lines) {
            file << line.characters << '\n';
            length += line.characters.size() + 1;
        }
        file.close();
        editor.dirty = 0;
        SetStatusMessage("{} bytes written to disk", length);
        return;
    }
    SetStatusMessage("Can't save!");
}

} // namespace editor
