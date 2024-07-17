module;

#include <iostream>
#include <ranges>

import Output;
import Terminal;

module Input;

namespace editor {

/*! @brief Tab spaces. */
constexpr const auto TAB_SPACES = 8;

/*! @brief Number of confirmations to quit. */
constexpr const auto QUIT_TIMES = 3;

/*! @brief Control key. */
const auto CONTROL_KEY = [](const auto key) { return key & 0x1f; };

auto Line::CursorXToRenderedX(int cursor_x) noexcept -> int {
    auto rendered_x = 0;
    for(const auto& j : std::ranges::views::iota(0, cursor_x)) {
        if(characters[j] == '\t')
            rendered_x += (TAB_SPACES - 1) - (rendered_x % TAB_SPACES);
        rendered_x++;
    }
    return rendered_x;
}

void Line::Update() noexcept {
    auto tabs = 0;
    for(const auto& c : characters)
        if(c == '\t')
            tabs++;
    rendered_characters.resize(characters.size() + tabs * (TAB_SPACES - 1));

    auto index = 0;
    for(const auto& c : characters) {
        if(c == '\t') {
            rendered_characters[index++] = ' ';
            while(index % TAB_SPACES != 0)
                rendered_characters[index++] = ' ';
        } else
            rendered_characters[index++] = c;
    }
}

void InsertLine(int at, const std::string& str) noexcept {
    if(at < 0 || static_cast<unsigned long>(at) > editor.lines.size())
        return;
    editor.lines.insert(editor.lines.begin() + at, Line{});
    editor.lines[at].characters = str;
    editor.lines[at].Update();
    editor.dirty++;
}

void InsertNewLine() noexcept {
    if(editor.cursor_x == 0)
        InsertLine(editor.cursor_y, "");
    else {
        auto& line = editor.lines[editor.cursor_y];
        InsertLine(editor.cursor_y + 1,
            line.characters.substr(editor.cursor_x));
        line.characters = line.characters.substr(0, editor.cursor_x);
        line.Update();
    }
    editor.cursor_y++;
    editor.cursor_x = 0;
}

void ProcessKeypress() noexcept {
    static auto quit_times = QUIT_TIMES;
    auto c = ReadKey();
    switch(c) {
        case '\r':
            InsertNewLine();
            break;

        case CONTROL_KEY('q'):
            if(editor.dirty && quit_times > 0) {
                SetStatusMessage(
                    "File has unsaved changes! Press Ctrl-Q {} more times to quit.",
                    quit_times);
                quit_times--;
                return;
            }
            std::cout << "\x1b[2J\x1b[H";
            exit(0);
            break;
    }
}

} // namespace editor
