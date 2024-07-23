#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "input.h"
#include "output.h"
#include "terminal.h"

/*! @brief Editor version. */
#define VERSION "0.0.1"

void BufferAppend(struct abuf_t *ab, const char *s, const int len) {
    char *new = realloc(ab->b, ab->len + len);

    if (new == NULL)
        return;
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

void BufferFree(struct abuf_t *ab) {
    free(ab->b);
}

void Scroll(void) {
    cfg.r_x = 0;
    if (cfg.c_y < cfg.num_lines)
        cfg.r_x = LineCxToRx(&cfg.line[cfg.c_y], cfg.c_x);

    if (cfg.c_y < cfg.row_offset)
        cfg.row_offset = cfg.c_y;
    if (cfg.c_y >= cfg.row_offset + cfg.scr_rows)
        cfg.row_offset = cfg.c_y - cfg.scr_rows + 1;
    if (cfg.r_x < cfg.col_offset)
        cfg.col_offset = cfg.r_x;
    if (cfg.r_x >= cfg.col_offset + cfg.scr_cols)
        cfg.col_offset = cfg.r_x - cfg.scr_cols + 1;
}

void DrawLines(struct abuf_t *ab) {
    for (int y = 0; y < cfg.scr_rows; y++) {
        const int fline = y + cfg.row_offset;
        if (fline >= cfg.num_lines) {
            if (cfg.num_lines == 0 && y == cfg.scr_rows / 3) {
                char welcome[80];
                int wlen = snprintf(welcome, sizeof(welcome),
                    "Text editor - version %s", VERSION);
                if (wlen > cfg.scr_cols)
                    wlen = cfg.scr_cols;
                int padding = (cfg.scr_cols - wlen) / 2;
                if (padding) {
                    BufferAppend(ab, "~", 1);
                    padding--;
                }
                while (padding--)
                    BufferAppend(ab, " ", 1);
                BufferAppend(ab, welcome, wlen);
            } else
                BufferAppend(ab, "~", 1);
        } else {
            int len = cfg.line[fline].rsize - cfg.col_offset;
            if (len < 0)
                len = 0;
            if (len > cfg.scr_cols)
                len = cfg.scr_cols;
            BufferAppend(ab, &cfg.line[fline].render[cfg.col_offset], len);
        }

        BufferAppend(ab, "\x1b[K", 3);
        BufferAppend(ab, "\r\n", 2);
    }
}

void DrawStatusBar(struct abuf_t *ab) {
    BufferAppend(ab, "\x1b[7m", 4);

    char status[80], rstatus[80];
    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
        cfg.filename ? cfg.filename : "[No Name]", cfg.num_lines,
        cfg.dirty ? "(modified)" : "");
    const int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d",
        cfg.c_y + 1, cfg.num_lines);

    if (len > cfg.scr_cols)
        len = cfg.scr_cols;
    BufferAppend(ab, status, len);
    while (len < cfg.scr_cols) {
        if (cfg.scr_cols - len == rlen) {
            BufferAppend(ab, rstatus, rlen);
            break;
        } else {
            BufferAppend(ab, " ", 1);
            len++;
        }
    }

    BufferAppend(ab, "\x1b[m", 3);
    BufferAppend(ab, "\r\n", 2);
}

void DrawMessageBar(struct abuf_t *ab) {
    BufferAppend(ab, "\x1b[K", 3);
    int msglen = strlen(cfg.statusmsg);
    if (msglen > cfg.scr_cols)
        msglen = cfg.scr_cols;
    if (msglen && time(NULL) - cfg.statusmsg_time < 5)
        BufferAppend(ab, cfg.statusmsg, msglen);
}

void RefreshScreen(void) {
    Scroll();

    struct abuf_t ab = {NULL, 0};
    BufferAppend(&ab, "\x1b[?25l", 6);
    BufferAppend(&ab, "\x1b[H", 3);

    DrawLines(&ab);
    DrawStatusBar(&ab);
    DrawMessageBar(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (cfg.c_y - cfg.row_offset) + 1,
        (cfg.r_x - cfg.col_offset) + 1);
    BufferAppend(&ab, buf, strlen(buf));

    BufferAppend(&ab, "\x1b[?25h", 6);

    write(STDOUT_FILENO, ab.b, ab.len);
    BufferFree(&ab);
}

char *LinesToString(int *buflen) {
    int totlen = 0;
    for (int j = 0; j < cfg.num_lines; j++)
        totlen += cfg.line[j].size + 1;
    *buflen = totlen;

    char *buf = malloc(totlen);
    char *p = buf;
    for (int j = 0; j < cfg.num_lines; j++) {
        memcpy(p, cfg.line[j].chars, cfg.line[j].size);
        p += cfg.line[j].size;
        *p = '\n';
        p++;
    }

    return buf;
}

void OpenFile(const char *filename) {
    free(cfg.filename);
    cfg.filename = strdup(filename);

    FILE *fp = fopen(filename, "r");
    if (!fp)
        Die("fopen");

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, fp)) != -1) {
        while (linelen > 0 && (line[linelen - 1] == '\n' ||
                               line[linelen - 1] == '\r'))
            linelen--;
        InsertLine(cfg.num_lines, line, linelen);
    }
    free(line);
    fclose(fp);
    cfg.dirty = 0;
}

void SaveFile(void) {
    if (cfg.filename == NULL) {
        cfg.filename = Prompt("Save as: %s");
        if (cfg.filename == NULL) {
            SetStatusMessage("Save aborted");
            return;
        }
    }

    int len;
    char *buf = LinesToString(&len);

    int fd = open(cfg.filename, O_RDWR | O_CREAT, 0644);
    if (fd != -1) {
        if (ftruncate(fd, len) != -1) {
            if (write(fd, buf, len) == len) {
                close(fd);
                free(buf);
                cfg.dirty = 0;
                SetStatusMessage("%d bytes written to disk", len);
                return;
            }
        }
        close(fd);
    }
    free(buf);
    SetStatusMessage("Can't save! I/O error: %s", strerror(errno));
}

void SetStatusMessage(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(cfg.statusmsg, sizeof(cfg.statusmsg), fmt, ap);
    va_end(ap);
    cfg.statusmsg_time = time(NULL);
}
