#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "input.h"
#include "output.h"
#include "terminal.h"

/*! @brief Tab spaces. */
#define TAB_SPACES 8

/*! @brief Number of confirmations to quit. */
#define QUIT_TIMES 3

/*! @brief Control key. */
#define CTRL_KEY(k) ((k) & 0x1f)

int LineCxToRx(line_t *line, int cx) {
    int r_x = 0;
    for (int j = 0; j < cx; j++) {
        if (line->chars[j] == '\t')
            r_x += (TAB_SPACES - 1) - (r_x % TAB_SPACES);
        r_x++;
    }
    return r_x;
}

void LineUpdate(line_t *line) {
    int tabs = 0;
    for (int j = 0; j < line->size; j++)
        if (line->chars[j] == '\t')
            tabs++;
    free(line->render);
    line->render = malloc(line->size + tabs * (TAB_SPACES - 1) + 1);

    int index = 0;
    for (int j = 0; j < line->size; j++) {
        if (line->chars[j] == '\t') {
            line->render[index++] = ' ';
            while (index % TAB_SPACES != 0)
                line->render[index++] = ' ';
        } else
            line->render[index++] = line->chars[j];
    }
    line->render[index] = '\0';
    line->rsize = index;
}

void LineInsertChar(line_t *line, int at, int c) {
    if (at < 0 || at > line->size)
        at = line->size;
    line->chars = realloc(line->chars, line->size + 2);
    memmove(&line->chars[at + 1], &line->chars[at], line->size - at + 1);
    line->size++;
    line->chars[at] = c;
    LineUpdate(line);
    cfg.dirty++;
}

void LineAppendString(line_t *line, char *s, size_t len) {
    line->chars = realloc(line->chars, line->size + len + 1);
    memcpy(&line->chars[line->size], s, len);
    line->size += len;
    line->chars[line->size] = '\0';
    LineUpdate(line);
    cfg.dirty++;
}

void LineDeleteChar(line_t *line, int at) {
    if (at < 0 || at >= line->size)
        return;
    memmove(&line->chars[at], &line->chars[at + 1], line->size - at);
    line->size--;
    LineUpdate(line);
    cfg.dirty++;
}

void InsertLine(int at, char *s, size_t len) {
    if (at < 0 || at > cfg.num_lines)
        return;
    cfg.line = realloc(cfg.line, sizeof(line_t) * (cfg.num_lines + 1));
    memmove(&cfg.line[at + 1], &cfg.line[at],
        sizeof(line_t) * (cfg.num_lines - at));

    cfg.line[at].size = len;
    cfg.line[at].chars = malloc(len + 1);
    memcpy(cfg.line[at].chars, s, len);
    cfg.line[at].chars[len] = '\0';
    cfg.line[at].rsize = 0;
    cfg.line[at].render = NULL;
    LineUpdate(&cfg.line[at]);

    cfg.num_lines++;
    cfg.dirty++;
}

void FreeLine(line_t *line) {
    free(line->render);
    free(line->chars);
}

void DeleteLine(int at) {
    if (at < 0 || at >= cfg.num_lines)
        return;
    FreeLine(&cfg.line[at]);
    memmove(&cfg.line[at], &cfg.line[at + 1],
        sizeof(line_t) * (cfg.num_lines - at - 1));
    cfg.num_lines--;
    cfg.dirty++;
}

void InsertChar(int c) {
    if (cfg.c_y == cfg.num_lines)
        InsertLine(cfg.num_lines, "", 0);
    LineInsertChar(&cfg.line[cfg.c_y], cfg.c_x, c);
    cfg.c_x++;
}

void InsertNewLine(void) {
    if (cfg.c_x == 0)
        InsertLine(cfg.c_y, "", 0);
    else {
        line_t *line = &cfg.line[cfg.c_y];
        InsertLine(cfg.c_y + 1, &line->chars[cfg.c_x], line->size - cfg.c_x);
        line = &cfg.line[cfg.c_y];
        line->size = cfg.c_x;
        line->chars[line->size] = '\0';
        LineUpdate(line);
    }
    cfg.c_y++;
    cfg.c_x = 0;
}

void DeleteChar(void) {
    if (cfg.c_y == cfg.num_lines)
        return;
    if (cfg.c_x == 0 && cfg.c_y == 0)
        return;
    
    line_t *line = &cfg.line[cfg.c_y];
    if (cfg.c_x > 0) {
        LineDeleteChar(line, cfg.c_x - 1);
        cfg.c_x--;
    } else {
        cfg.c_x = cfg.line[cfg.c_y - 1].size;
        LineAppendString(&cfg.line[cfg.c_y - 1], line->chars, line->size);
        DeleteLine(cfg.c_y);
        cfg.c_y--;
    }
}

char *Prompt(char *prompt) {
    size_t bufsize = 128;
    char *buf = malloc(bufsize);
    size_t buflen = 0;
    buf[0] = '\0';

    while (1) {
        SetStatusMessage(prompt, buf);
        RefreshScreen();

        int c = ReadKey();
        if (c == DELETE || c == CTRL_KEY('h') || c == BACKSPACE) {
            if (buflen != 0)
                buf[--buflen] = '\0';
        } else if (c == '\x1b') {
            SetStatusMessage("");
            free(buf);
            return NULL;
        } else if (c == '\r') {
            if (buflen != 0) {
                SetStatusMessage("");
                return buf;
            }
        } else if (!iscntrl(c) && c < 128) {
            if (buflen == bufsize - 1) {
                bufsize *= 2;
                buf = realloc(buf, bufsize);
            }
            buf[buflen++] = c;
            buf[buflen] = '\0';
        }
    }
}

void MoveCursor(int key) {
    line_t *line = (cfg.c_y >= cfg.num_lines) ? NULL : &cfg.line[cfg.c_y];

    switch (key) {
        case ARROW_LEFT:
            if (cfg.c_x != 0)
                cfg.c_x--;
            else if (cfg.c_y > 0) {
                cfg.c_y--;
                cfg.c_x = cfg.line[cfg.c_y].size;
            }
            break;
        case ARROW_RIGHT:
            if (line && cfg.c_x < line->size)
                cfg.c_x++;
            else if (line && cfg.c_x == line->size) {
                cfg.c_y++;
                cfg.c_x = 0;
            }
            break;
        case ARROW_UP:
            if (cfg.c_y != 0)
                cfg.c_y--;
            break;
        case ARROW_DOWN:
            if (cfg.c_y < cfg.num_lines)
                cfg.c_y++;
            break;
    }

    line = (cfg.c_y >= cfg.num_lines) ? NULL : &cfg.line[cfg.c_y];
    int line_len = line ? line->size : 0;
    if (cfg.c_x > line_len)
        cfg.c_x = line_len;
}

void ProcessKeypress(void) {
    static int quit_times = QUIT_TIMES;
    int c = ReadKey();
    switch (c) {
        case '\r':
            InsertNewLine();
            break;

        case CTRL_KEY('q'):
            if (cfg.dirty && quit_times > 0) {
                SetStatusMessage("File has unsaved changes! Press Ctrl-Q %d more times to quit.", quit_times);
                quit_times--;
                return;
            }
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;

        case CTRL_KEY('s'):
            SaveFile();
            break;

        case HOME:
            cfg.c_x = 0;
            break;
        case END:
            if (cfg.c_y < cfg.num_lines)
                cfg.c_x = cfg.line[cfg.c_y].size;
            break;

        case BACKSPACE:
        case CTRL_KEY('h'):
        case DELETE:
            if (c == DELETE)
                MoveCursor(ARROW_RIGHT);
            DeleteChar();
            break;

        case PAGE_UP:
        case PAGE_DOWN:
            {
                if (c == PAGE_UP)
                    cfg.c_y = cfg.row_offset;
                else if (c == PAGE_DOWN) {
                    cfg.c_y = cfg.row_offset + cfg.scr_rows - 1;
                    if (cfg.c_y > cfg.num_lines)
                        cfg.c_y = cfg.num_lines;
                }

                int times = cfg.scr_rows;
                while (times--)
                    MoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
            break;

        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            MoveCursor(c);
            break;

        case CTRL_KEY('l'):
        case '\x1b':
            break;

        default:
            InsertChar(c);
            break;
    }

    quit_times = QUIT_TIMES;
}
