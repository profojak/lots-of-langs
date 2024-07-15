module;

#include <iostream>
#include <ranges>

import Terminal;

module Input;

namespace editor {

/*! @brief Tab spaces. */
constexpr const auto TAB_SPACES = 8;

/*! @brief Number of confirmations to quit. */
//constexpr const auto QUIT_TIMES = 3;

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

void ProcessKeypress() noexcept {
    auto c = ReadKey();
    switch(c) {
        case CONTROL_KEY('q'):
            std::cout << "\x1b[2J\x1b[H";
            exit(0);
            break;
    }
}

} // namespace editor
