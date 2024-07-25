#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdint.h>
#include <stdio.h>

/*! @brief TGA image header. */
#pragma pack(push, 1)
typedef struct {
    uint8_t id_len;
    uint8_t cmap_type;
    uint8_t img_type;
    uint16_t cmap_orig;
    uint16_t cmap_len;
    uint8_t cmap_depth;
    uint16_t xorig;
    uint16_t yorig;
    uint16_t w;
    uint16_t h;
    uint8_t pixel_depth;
    uint8_t img_desc;
} header_t;
#pragma pack(pop)

/*! @brief TGA image pixel. */
typedef struct {
    /*! @brief Data. */
    uint8_t data[4];
} pixel_t;

/*! @brief Image. */
typedef struct {
    /*! @brief Data. */
    uint8_t *data;
    /*! @brief Z-buffer. */
    float* zbuf;
    /*! @brief Width. */
	int w;
    /*! @brief Height. */
	int h;
    /*! @brief Bytes per pixel. */
	uint8_t bpp;
} image_t;

/*! @brief Format. */
enum {
    GRAYSCALE = 1,
    RGB = 3,
    RGBA = 4,
};

/**
 * @brief Create image.
 * @param[in] w Width.
 * @param[in] h Height.
 * @param[in] bpp Bytes per pixel.
 */
image_t ImageCreate(const int w, const int h, const uint8_t bpp);

/**
 * @brief Destroy image.
 * @param[in,out] image Image.
 */
void ImageDestroy(image_t *image);

/**
 * @brief Read TGA file.
 * @param[in] filename Filename.
 * @param[out] image Image.
 * @return Zero on success, non-zero on failure.
 */
int ImageReadTgaFile(const char *filename, image_t *image);

/**
 * @brief Write TGA file.
 * @param[in] filename Filename.
 * @param[in] rle Run-length encoding.
 * @param[in] image Image.
 * @return Zero on success, non-zero on failure.
 */
int ImageWriteTgaFile(const char *filename, const int rle,
    const image_t *image);

/**
 * @brief Load run-length encoded data.
 * @param[in] fp File pointer.
 * @param[out] image Image.
 * @return Zero on success, non-zero on failure.
 */
int ImageLoadRleData(FILE *fp, image_t *image);

/**
 * @brief Unload run-length encoded data.
 * @param[in] fp File pointer.
 * @param[in] image Image.
 * @return Zero on success, non-zero on failure.
 */
int ImageUnloadRleData(FILE *fp, const image_t *image);

/**
 * @brief Set pixel.
 * @param[in] x X coordinate.
 * @param[in] y Y coordinate.
 * @param[in] z Z-buffer value.
 * @param[in] pixel Pixel.
*/
void ImageSetPixel(const int x, const int y, const float z, const pixel_t pixel,
    image_t *image);

/**
 * @brief Get pixel.
 * @param[in] x X coordinate.
 * @param[in] y Y coordinate.
 * @param[in] image Image.
 */
pixel_t ImageGetPixel(const int x, const int y, const image_t *image);

/**
 * @brief Flip horizontally.
 * @param[in,out] image Image.
 */
void ImageFlipHorizontally(image_t *image);

/**
 * @brief Flip vertically.
 * @param[in,out] image Image.
 */
void ImageFlipVertically(image_t *image);

#endif // _IMAGE_H_
