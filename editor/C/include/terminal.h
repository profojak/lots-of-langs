#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <termios.h>
#include <time.h>

/*! @brief Editor. */
extern struct editor_t {
    /*! @brief Cursor X coordinate. */
    int c_x;
    /*! @brief Cursor Y coordinate. */
    int c_y;
    /*! @brief Rendered cursor X coordinate. */
    int r_x;
    /*! @brief Screen rows. */
    int scr_rows;
    /*! @brief Screen columns. */
    int scr_cols;
    /*! @brief Row offset. */
    int row_offset;
    /*! @brief Column offset. */
    int col_offset;
    /*! @brief Number of lines in file. */
    int num_lines;
    /*! @brief Lines. */
    struct line_t *line;
    /*! @brief File dirty flag. */
    int dirty;
    /*! @brief Filename. */
    char *filename;
    /*! @brief Status message. */
    char statusmsg[80];
    /*! @brief Status message time. */
    time_t statusmsg_time;
    /*! @brief Original termios configuration. */
    struct termios orig_termios;
} cfg;

/*! @brief Key codes. */
enum key_t {
    BACKSPACE = 127,
    ARROW_UP = 1000,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
    PAGE_UP,
    PAGE_DOWN,
    HOME,
    END,
    DELETE
};

/*! @brief Initialize editor. */
void InitEditor(void);

/**
 * @brief Die with error message.
 * @param[in] s Error message.
 */
void Die(const char *s);

/*! @brief Disable terminal raw mode. */
void DisableRawMode(void);

/*! @brief Enable terminal raw mode. */
void EnableRawMode(void);

/**
 * @brief Read key.
 * @return Key code.
 */
int ReadKey(void);

/**
 * @brief Get cursor position.
 * @param[out] rows Rows.
 * @param[out] cols Columns.
 * @return Zero on success, non-zero on failure.
 */
int GetCursorPosition(int *rows, int *cols);

/**
 * @brief Get window size.
 * @param[out] rows Rows.
 * @param[out] cols Columns.
 * @return Zero on success, non-zero on failure.
 */
int GetWindowSize(int *rows, int *cols);

#endif // _TERMINAL_H_
