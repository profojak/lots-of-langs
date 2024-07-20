#ifndef _RENDER_H_
#define _RENDER_H_

/*! @brief Append buffer. */
struct abuf_t {
    char *b;
    int len;
};

/**
 * @brief Append string to append buffer.
 * @param[in] ab Append buffer.
 * @param[in] s String.
 * @param[in] len Length.
 */
void BufferAppend(struct abuf_t *ab, const char *s, int len);

/**
 * @brief Free append buffer.
 * @param[in] ab Append buffer.
 */
void BufferFree(struct abuf_t *ab);

/*! @brief Scroll editor view. */
void Scroll(void);

/**
 * @brief Draw lines.
 * @param[in] ab Append buffer.
 */
void DrawLines(struct abuf_t *ab);

/**
 * @brief Draw status bar.
 * @param[in] ab Append buffer.
 */
void DrawStatusBar(struct abuf_t *ab);

/**
 * @brief Draw message bar.
 * @param[in] ab Append buffer.
 */
void DrawMessageBar(struct abuf_t *ab);

/*! @brief Refresh screen. */
void RefreshScreen(void);

/**
 * @brief Convert lines to string.
 * @param[out] buflen Buffer length.
 * @return Buffer.
 */
char *LinesToString(int *buflen);

/**
 * @brief Open file.
 * @param[in] filename Filename.
 */
void OpenFile(char *filename);

/*! @brief Save file. */
void SaveFile(void);

/**
 * @brief Set status message.
 * @param[in] fmt Format string.
 * @param[in] ... Arguments.
 */
void SetStatusMessage(const char *fmt, ...);

#endif // _RENDER_H_
