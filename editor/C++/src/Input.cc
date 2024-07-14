module;

#include <iostream>

import Terminal;

module Input;

namespace editor {

/*! @brief Number of confirmations to quit. */
//constexpr const auto QUIT_TIMES = 3;

/*! @brief Control key. */
const auto CONTROL_KEY = [](const auto key) { return key & 0x1f; };

void ProcessKeypress() {
    auto c = ReadKey();
    switch(c) {
        case CONTROL_KEY('q'):
            std::cout << "\x1b[2J\x1b[H";
            exit(0);
            break;
    }
}

} // namespace editor
