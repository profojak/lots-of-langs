#ifndef _INPUT_H_
#define _INPUT_H_

#include <stddef.h>

/*! @brief Line. */
typedef struct line_t {
    /*! @brief Size. */
    int size;
    /*! @brief Render size. */
    int rsize;
    /*! @brief Characters. */
    char *chars;
    /*! @brief Rendered characters. */
    char *render;
} line_t;

/**
 * @brief Convert cursor X coordinate to rendered X coordinate.
 * @param[in] line Line.
 * @param[in] cx Cursor X coordinate.
 * @return Rendered X coordinate.
 */
int LineCxToRx(line_t *line, int cx);

/**
 * @brief Update line after modification.
 * @param[in] line Line.
 */
void LineUpdate(line_t *line);

/**
 * @brief Insert character into line.
 * @param[in] line Line.
 * @param[in] at Position.
 * @param[in] c Character.
 */
void LineInsertChar(line_t *line, int at, int c);

/**
 * @brief Append string to line.
 * @param[in] line Line.
 * @param[in] s String.
 * @param[in] len Length.
 */
void LineAppendString(line_t *line, char *s, size_t len);

/**
 * @brief Delete character from line.
 * @param[in] line Line.
 * @param[in] at Position.
 */
void LineDeleteChar(line_t *line, int at);

/**
 * @brief Insert line.
 * @param[in] at Position.
 * @param[in] s String.
 * @param[in] len Length.
 */
void InsertLine(int at, char *s, size_t len);

/**
 * @brief Free line.
 * @param[in] line Line.
 */
void FreeLine(line_t *line);

/**
 * @brief Delete line.
 * @param[in] at Position.
 */
void DeleteLine(int at);

/**
 * @brief Insert character.
 * @param[in] c Character.
 */
void InsertChar(int c);

/*! @brief Insert new line. */
void InsertNewLine(void);

/*! @brief Delete character. */
void DeleteChar(void);

/**
 * @brief Ask user for text input.
 * @param[in] prompt Prompt.
 * @return Text.
 */
char *Prompt(char *prompt);

/**
 * @brief Move cursor.
 * @param[in] key Key code.
 */
void MoveCursor(int key);

/*! @brief Process key press. */
void ProcessKeypress(void);

#endif // _INPUT_H_
