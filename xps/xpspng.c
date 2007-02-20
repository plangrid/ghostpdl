#include "ghostxps.h"

#include "stream.h"
#include "strimpl.h"
#include "gsstate.h"
#include "png_.h"

/*
 * PNG using libpng directly (no gs wrappers)
 */

struct xps_png_io_s
{
    byte *ptr;
    byte *lim;
};

private void xps_png_read(png_structp png, png_bytep data, png_size_t length)
{
    struct xps_png_io_s *io = png_get_io_ptr(png);
    if (io->ptr + length > io->lim)
        png_error(png, "Read Error");
    memcpy(data, io->ptr, length);
    io->ptr += length;
}

private png_voidp xps_png_malloc(png_structp png, png_size_t size)
{
    gs_memory_t *mem = png_get_mem_ptr(png);
    return gs_alloc_bytes(mem, size, "libpng");
}

private void xps_png_free(png_structp png, png_voidp ptr)
{
    gs_memory_t *mem = png_get_mem_ptr(png);
    gs_free_object(mem, ptr, "libpng");
}

int xps_decode_png(gs_memory_t *mem, byte *rbuf, int rlen, xps_image_t *image)
{
    png_structp png;
    png_infop info;
    struct xps_png_io_s io;
    int npasses;
    int pass;
    int y;

    /*
     * Set up PNG structs and input source
     */

    io.ptr = rbuf;
    io.lim = rbuf + rlen;

    png = png_create_read_struct_2(PNG_LIBPNG_VER_STRING,
            NULL, NULL, NULL,
            mem, xps_png_malloc, xps_png_free);
    if (!png)
        return gs_throw(-1, "png_create_read_struct");

    info = png_create_info_struct(png);
    if (!info)
        return gs_throw(-1, "png_create_info_struct");

    png_set_read_fn(png, &io, xps_png_read);

    /*
     * Jump to here on errors.
     */

    if (setjmp(png_jmpbuf(png)))
    {
        png_destroy_read_struct(&png, &info, NULL);
        return gs_throw(-1, "png reading failed");
    }

    /*
     * Read PNG header
     */

    png_read_info(png, info);

    image->width = png_get_image_width(png, info);
    image->height = png_get_image_height(png, info);
    image->bits = png_get_bit_depth(png, info);

    if (png_get_interlace_type(png, info) == PNG_INTERLACE_ADAM7)
    {
        npasses = png_set_interlace_handling(png);
    }
    else
    {
        npasses = 1;
    }

    if (image->bits == 16)
    {
        png_set_strip_16(png);
        image->bits = 8;
    }

    switch (png_get_color_type(png, info))
    {
    case PNG_COLOR_TYPE_GRAY:
        image->comps = 1;
        image->colorspace = XPS_GRAY;
        break;

    case PNG_COLOR_TYPE_PALETTE:
        /* ask libpng to expand palettes to rgb triplets */
        png_set_palette_to_rgb(png);
        image->bits = 8;

        /* libpng will expand to rgba if there is a tRNS chunk */
        if (png_get_valid(png, info, PNG_INFO_tRNS))
        {
            image->comps = 4;
            image->colorspace = XPS_RGB_A;
        }
        else
        {
            image->comps = 3;
            image->colorspace = XPS_RGB;
        }
        break;

    case PNG_COLOR_TYPE_RGB:
        image->comps = 3;
        image->colorspace = XPS_RGB;
        break;

    case PNG_COLOR_TYPE_GRAY_ALPHA:
        image->comps = 2;
        image->colorspace = XPS_GRAY_A;
        break;

    case PNG_COLOR_TYPE_RGB_ALPHA:
        image->comps = 4;
        image->colorspace = XPS_RGB_A;
        break;

    default:
        return gs_throw(-1, "cannot handle this png color type");
    }

    image->stride = (image->width * image->comps * image->bits + 7) / 8;

    /*
     * Extract DPI, default to 96 dpi
     */

    image->xres = 96;
    image->yres = 96;

    if (info->valid & PNG_INFO_pHYs)
    {
        png_uint_32 xres, yres;
        int unit;
        png_get_pHYs(png, info, &xres, &yres, &unit);
        if (unit == PNG_RESOLUTION_METER)
        {
            image->xres = xres * 0.0254 + 0.5;
            image->yres = yres * 0.0254 + 0.5;
        }
    }

    /*
     * Read rows, filling transformed output into image buffer.
     */

    image->samples = gs_alloc_bytes(mem, image->stride * image->height, "decodepng");

    for (pass = 0; pass < npasses; pass++)
    {
        for (y = 0; y < image->height; y++)
        {
            png_read_row(png, image->samples + (y * image->stride), NULL);
        }
    }

    /*
     * Clean up memory.
     */

    png_destroy_read_struct(&png, &info, NULL);

    return gs_okay;
}

