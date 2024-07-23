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

auto Line::CursorXToRenderedX(const int cursor_x) noexcept -> int {
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

void Line::InsertChar(const int at, const int c) noexcept {
    auto cx = (at < 0 || static_cast<unsigned long>(at) > characters.size()) ?
        characters.size() : at;
    characters.insert(cx, 1, c);
    Update();
    editor.dirty++;
}

void Line::AppendString(const std::string& str) noexcept {
    characters.append(str);
    Update();
    editor.dirty++;
}

void Line::DeleteChar(const int at) noexcept {
    if(at < 0 || static_cast<unsigned long>(at) >= characters.size())
        return;
    characters.erase(at, 1);
    Update();
    editor.dirty++;
}

void InsertLine(const int at, const std::string& str) noexcept {
    if(at < 0 || static_cast<unsigned long>(at) > editor.lines.size())
        return;
    editor.lines.insert(editor.lines.begin() + at, Line{});
    editor.lines[at].characters = str;
    editor.lines[at].Update();
    editor.dirty++;
}

void DeleteLine(const int at) noexcept {
    if(at < 0 || static_cast<unsigned long>(at) >= editor.lines.size())
        return;
    editor.lines.erase(editor.lines.begin() + at);
    editor.dirty++;
}

void InsertChar(const int c) noexcept {
    if(static_cast<unsigned long>(editor.cursor_y) == editor.lines.size())
        InsertLine(editor.lines.size(), "");
    editor.lines[editor.cursor_y].InsertChar(editor.cursor_x, c);
    editor.cursor_x++;
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

void DeleteChar() noexcept {
    if(static_cast<unsigned long>(editor.cursor_y) >= editor.lines.size())
        return;
    if(editor.cursor_x == 0 && editor.cursor_y == 0)
        return;

    auto& line = editor.lines[editor.cursor_y];
    if(editor.cursor_x > 0) {
        line.DeleteChar(editor.cursor_x - 1);
        editor.cursor_x--;
    } else {
        editor.cursor_x = editor.lines[editor.cursor_y - 1].characters.size();
        editor.lines[editor.cursor_y - 1].AppendString(line.characters);
        DeleteLine(editor.cursor_y);
        editor.cursor_y--;
    }
}

auto Prompt(const std::string& prompt) -> std::string {
    std::string buffer;
    while(true) {
        SetStatusMessage(prompt, buffer);
        RefreshScreen();

        const auto c = ReadKey();
        if(c == DELETE || c == CONTROL_KEY('h') || c == BACKSPACE) {
            if(!buffer.empty())
                buffer.pop_back();
        } else if(c == '\x1b') {
            SetStatusMessage("");
            return {};
        } else if(c == '\r') {
            if(!buffer.empty()) {
                SetStatusMessage("");
                return buffer;
            }
        } else if(!iscntrl(c) && c < 128)
            buffer.push_back(c);
    }
}

void MoveCursor(const int key) noexcept {
    auto line = (static_cast<unsigned long>(editor.cursor_y) >=
        editor.lines.size()) ? nullptr : &editor.lines[editor.cursor_y];
    
    switch(key) {
        case ARROW_LEFT:
            if(editor.cursor_x != 0)
                editor.cursor_x--;
            else if(editor.cursor_y > 0) {
                editor.cursor_y--;
                editor.cursor_x =
                    editor.lines[editor.cursor_y].characters.size();
            }
            break;
        case ARROW_RIGHT:
            if(line && static_cast<unsigned long>(editor.cursor_x) <
                line->characters.size())
                editor.cursor_x++;
            else if(line && static_cast<unsigned long>(editor.cursor_x) ==
                line->characters.size()) {
                editor.cursor_y++;
                editor.cursor_x = 0;
            }
            break;
        case ARROW_UP:
            if(editor.cursor_y != 0)
                editor.cursor_y--;
            break;
        case ARROW_DOWN:
            if(static_cast<unsigned long>(editor.cursor_y) <
                editor.lines.size())
                editor.cursor_y++;
            break;
    }

    line = (static_cast<unsigned long>(editor.cursor_y) >=
        editor.lines.size()) ? nullptr : &editor.lines[editor.cursor_y];
    const int line_length = line ? line->characters.size() : 0;
    if(editor.cursor_x > line_length)
        editor.cursor_x = line_length;
}

void ProcessKeypress() noexcept {
    static auto quit_times = QUIT_TIMES;
    const auto c = ReadKey();
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

        case CONTROL_KEY('s'):
            SaveFile();
            break;
        
        case HOME:
            editor.cursor_x = 0;
            break;
        case END:
            if(static_cast<unsigned long>(editor.cursor_y) <
                editor.lines.size())
                editor.cursor_x =
                    editor.lines[editor.cursor_y].characters.size();
            break;
        
        case BACKSPACE:
        case CONTROL_KEY('h'):
        case DELETE:
            if(c == DELETE)
                MoveCursor(ARROW_RIGHT);
            DeleteChar();
            break;
        
        case PAGE_UP:
        case PAGE_DOWN:
            {
                if(c == PAGE_UP)
                    editor.cursor_y = editor.row_offset;
                else if(c == PAGE_DOWN) {
                    editor.cursor_y = editor.row_offset +
                        editor.screen_rows - 1;
                    if(static_cast<unsigned long>(editor.cursor_y) >
                        editor.lines.size())
                        editor.cursor_y = editor.lines.size();
                }
                auto times = editor.screen_rows;
                while(times--)
                    MoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
            break;
        
        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            MoveCursor(c);
            break;
        
        case CONTROL_KEY('l'):
        case '\x1b':
            break;
        
        default:
            InsertChar(c);
            break;
    }

    quit_times = QUIT_TIMES;
}

} // namespace editor
