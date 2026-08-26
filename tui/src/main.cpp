#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

//Windows
#ifdef _WIN32
#include <windows.h>
#undef min
#undef max

// Linux / Unix
#else 
#include <csignal>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif

struct TerminalSize {
    int width;
    int height;
};

enum class Key {
    None,
    Up,
    Down,
    Enter,
    Resize,
    Quit
};

#ifdef _WIN32

HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

DWORD oldInputMode;
DWORD oldOutputMode;

void beginTUI() {
    SetConsoleOutputCP(CP_UTF8);

    GetConsoleMode(hIn, &oldInputMode);
    GetConsoleMode(hOut, &oldOutputMode);

    SetConsoleMode(
        hIn,
        ENABLE_WINDOW_INPUT |
        ENABLE_EXTENDED_FLAGS
    );

    SetConsoleMode(
        hOut,
        oldOutputMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING
    );

    std::cout << "\x1b[?1049h"  // alternate screen
                "\x1b[?25l"    // hide cursor
                "\x1b[?7l";    // disable line wrapping
}

void endTUI() {
    SetConsoleMode(hIn, oldInputMode);
    SetConsoleMode(hOut, oldOutputMode);

    std::cout << "\x1b[?7h"     // enable line wrapping
             "\x1b[?25h"    // show cursor
             "\x1b[?1049l"; // leave alternate screen
}

TerminalSize getTerminalSize() {
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(hOut, &info);

    return {
        info.srWindow.Right - info.srWindow.Left + 1,
        info.srWindow.Bottom - info.srWindow.Top + 1
    };
}

Key readKey() {
    INPUT_RECORD record;
    DWORD count;

    while (true) {
        ReadConsoleInputW(hIn, &record, 1, &count);

        if (record.EventType == WINDOW_BUFFER_SIZE_EVENT)
            return Key::Resize;

        if (record.EventType != KEY_EVENT)
            continue;

        const auto& event = record.Event.KeyEvent;

        if (!event.bKeyDown)
            continue;

        switch (event.wVirtualKeyCode) {
            case VK_UP:     return Key::Up;
            case VK_DOWN:   return Key::Down;
            case VK_RETURN: return Key::Enter;
        }

        switch (event.uChar.UnicodeChar) {
            case L'j': return Key::Down;
            case L'k': return Key::Up;
            case L'q': return Key::Quit;
        }
    }
}

#else

termios oldTermios;

volatile std::sig_atomic_t resized = 0;

void handleResize(int) {
    resized = 1;
}

void beginTUI() {
    tcgetattr(STDIN_FILENO, &oldTermios);

    termios raw = oldTermios;

    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    
    std::cout << "\x1b[?1049h"  // alternate screen
                "\x1b[?25l"    // hide cursor
                "\x1b[?7l";    // disable line wrapping
}

void endTUI() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldTermios);

       std::cout << "\x1b[?7h"     // enable line wrapping
             "\x1b[?25h"    // show cursor
             "\x1b[?1049l"; // leave alternate screen
}

TerminalSize getTerminalSize() {
    winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);

    return {
        static_cast<int>(ws.ws_col),
        static_cast<int>(ws.ws_row)
    };
}

Key readKey() {
    char c;

    while (true) {
        if (resized) {
            resized = 0;
            return Key::Resize;
        }

        const ssize_t result = read(STDIN_FILENO, &c, 1);

        if (result < 0) {
            if (resized) {
                resized = 0;
                return Key::Resize;
            }

            continue;
        }

        break;
    }

    if (c == 'q')
        return Key::Quit;

    if (c == 'j')
        return Key::Down;

    if (c == 'k')
        return Key::Up;

    if (c == '\n' || c == '\r')
        return Key::Enter;

    if (c == '\x1b') {
        char sequence[2];

        if (read(STDIN_FILENO, &sequence[0], 1) != 1)
            return Key::None;

        if (read(STDIN_FILENO, &sequence[1], 1) != 1)
            return Key::None;

        if (sequence[0] == '[') {
            if (sequence[1] == 'A')
                return Key::Up;

            if (sequence[1] == 'B')
                return Key::Down;
        }
    }

    return Key::None;
}

#endif

void setTitle(std::string title) {
    std::cout << "\x1b]0;" << title << "\x07";
}


void render(
    const std::vector<std::string>& items,
    int cursor,
    const std::string& selected
) {
    const TerminalSize terminal = getTerminalSize();

    constexpr int displayHeight = 4;

    const int menuHeight =
        std::max(0, terminal.height - displayHeight);

    std::string output;

    output += "\x1b[H";

    // Menu
    for (int row = 0; row < menuHeight; ++row) {
        if (row < static_cast<int>(items.size())) {
            if (row == cursor)
                output += "> ";
            else
                output += "  ";

            output += items[row];
        }

        output += "\x1b[K";

        if (row + 1 < terminal.height)
            output += '\n';
    }

    // Separator
    for (int i = 0; i < terminal.width; ++i)
        output += "─";

    output += "\x1b[K\n";

    // Reserved display area
    output += selected;
    output += "\x1b[K\n";
    output += "\x1b[K\n";


    output += "↑↓ / jk move   Enter select   q quit";
    output += "\x1b[K";

    // Don't leave terminal's logical cursor on the bottom row
    output += "\x1b[H";

    std::cout << output << std::flush;
}

int main() {
    const std::vector<std::string> items = {
        "First item",
        "Second item",
        "Third item",
        "Fourth item",
        "Fifth item"
    };

    int cursor = 0;
    std::string selected = "Nothing selected";

    setTitle("TUI Program");

    beginTUI();

    try {
        bool running = true;

        while (running) {
            render(items, cursor, selected);

            switch (readKey()) {
                case Key::Up:
                    cursor = std::max(0, cursor - 1);
                    break;

                case Key::Down:
                    cursor = std::min(
                        static_cast<int>(items.size()) - 1,
                        cursor + 1
                    );
                    break;

                case Key::Enter:
                    selected = items[cursor];
                    break;

                case Key::Quit:
                    running = false;
                    break;

                case Key::Resize:
                    break;

                default:
                    break;
            }
        }
    }
    catch (...) {
        endTUI();
        throw;
    }

    endTUI();
}