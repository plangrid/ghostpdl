/* Portions Copyright (C) 2001 artofcode LLC.
   Portions Copyright (C) 1996, 2001 Artifex Software Inc.
   Portions Copyright (C) 1988, 2000 Aladdin Enterprises.
   This software is based in part on the work of the Independent JPEG Group.
   All Rights Reserved.

   This software is distributed under license and may not be copied, modified
   or distributed except as expressly authorized under the terms of that
   license.  Refer to licensing information at http://www.artifex.com/ or
   contact Artifex Software, Inc., 101 Lucas Valley Road #110,
   San Rafael, CA  94903, (415)492-9861, for further information. */

/*$RCSfile$ $Revision$ */
/* JPEG output driver */
#include "stdio_.h"		/* for jpeglib.h */
#include "jpeglib_.h"
#include "gdevprn.h"
#include "stream.h"
#include "strimpl.h"
#include "sdct.h"
#include "sjpeg.h"

/* Structure for the JPEG-writing device. */
typedef struct gx_device_jpeg_s {
    gx_device_common;
    gx_prn_device_common;
    /* Additional parameters */
    int JPEGQ;			/* quality on IJG scale */
    float QFactor;		/* quality per DCTEncode conventions */
    /* JPEGQ overrides QFactor if both are specified. */    

    /** 1.0 default 2.0 is twice a big 
     */
    gs_point ViewScale;

    /** translation needs to have scalefactor multiplied in.
     */
    gs_point ViewTrans;

    int TrayOrientation;        /* 0 (default), 90, 180, 270 */
} gx_device_jpeg;

/* The device descriptor */
private dev_proc_get_params(jpeg_get_params);
private dev_proc_get_initial_matrix(jpeg_get_initial_matrix);
private dev_proc_put_params(jpeg_put_params);
private dev_proc_print_page(jpeg_print_page);

/* ------ The device descriptors ------ */

/* Default X and Y resolution. */
#ifndef X_DPI
#  define X_DPI 72
#endif
#ifndef Y_DPI
#  define Y_DPI 72
#endif

/* 24-bit color */

private const gx_device_procs jpeg_procs =
{
    gdev_prn_open,
    jpeg_get_initial_matrix,	/* get_initial_matrix */
    NULL,			/* sync_output */
    gdev_prn_output_page,
    gdev_prn_close,
    gx_default_rgb_map_rgb_color,/* map_rgb_color */
    gx_default_rgb_map_color_rgb,
    NULL,			/* fill_rectangle */
    NULL,			/* tile_rectangle */
    NULL,			/* copy_mono */
    NULL,			/* copy_color */
    NULL,			/* draw_line */
    NULL,			/* get_bits */
    jpeg_get_params,
    jpeg_put_params,
    NULL,
    NULL,			/* get_xfont_procs */
    NULL,			/* get_xfont_device */
    NULL,			/* map_rgb_alpha_color */
    gx_page_device_get_page_device
};

const gx_device_jpeg gs_jpeg_device =
{prn_device_std_body(gx_device_jpeg, jpeg_procs, "jpeg",
		     DEFAULT_WIDTH_10THS, DEFAULT_HEIGHT_10THS,
		     X_DPI, Y_DPI, 0, 0, 0, 0, 24, jpeg_print_page),
 0,				/* JPEGQ: 0 indicates not specified */
 0.0,				/* QFactor: 0 indicates not specified */
 { 1.0, 1.0 },                  /* ViewScale 1 to 1 */ 
 { 0.0, 0.0 }                   /* translation 0 */ 
};

/* 8-bit gray */

private const gx_device_procs jpeggray_procs =
{
    gdev_prn_open,
    jpeg_get_initial_matrix,	/* get_initial_matrix */
    NULL,			/* sync_output */
    gdev_prn_output_page,
    gdev_prn_close,
    gx_default_gray_map_rgb_color,/* map_rgb_color */
    gx_default_gray_map_color_rgb,
    NULL,			/* fill_rectangle */
    NULL,			/* tile_rectangle */
    NULL,			/* copy_mono */
    NULL,			/* copy_color */
    NULL,			/* draw_line */
    NULL,			/* get_bits */
    jpeg_get_params,
    jpeg_put_params,
    NULL,
    NULL,			/* get_xfont_procs */
    NULL,			/* get_xfont_device */
    NULL,			/* map_rgb_alpha_color */
    gx_page_device_get_page_device
};

const gx_device_jpeg gs_jpeggray_device =
{prn_device_body(gx_device_jpeg, jpeggray_procs, "jpeggray",
		 DEFAULT_WIDTH_10THS, DEFAULT_HEIGHT_10THS,
		 X_DPI, Y_DPI, 0, 0, 0, 0,
		 1, 8, 255, 0, 256, 0,
		 jpeg_print_page),
 0,				/* JPEGQ: 0 indicates not specified */
 0.0,				/* QFactor: 0 indicates not specified */
 { 1.0, 1.0 },                  /* ViewScale 1 to 1 */ 
 { 0.0, 0.0 }                   /* translation 0 */ 
};

/* Get parameters. */
private int
jpeg_get_params(gx_device * dev, gs_param_list * plist)
{
    gx_device_jpeg *jdev = (gx_device_jpeg *) dev;
    int code = gdev_prn_get_params(dev, plist);
    int ecode;

    if (code < 0)
	return code;

    if ((ecode = param_write_int(plist, "JPEGQ", &jdev->JPEGQ)) < 0)
	code = ecode;
    if ((ecode = param_write_float(plist, "QFactor", &jdev->QFactor)) < 0)
	code = ecode;
    if ((ecode = param_write_float(plist, "ViewScaleX", &jdev->ViewScale.x)) < 0)
	code = ecode;
    if ((ecode = param_write_float(plist, "ViewScaleY", &jdev->ViewScale.y)) < 0)
	code = ecode;
    if ((ecode = param_write_float(plist, "ViewTransX", &jdev->ViewTrans.x)) < 0)
	code = ecode;
    if ((ecode = param_write_float(plist, "ViewTransY", &jdev->ViewTrans.y)) < 0)
	code = ecode;
    if ((ecode = param_write_int(plist, "TrayOrientation", &jdev->TrayOrientation)) < 0)
        code = ecode;


    return code;
}

/* Put parameters. */
private int
jpeg_put_params(gx_device * dev, gs_param_list * plist)
{
    gx_device_jpeg *jdev = (gx_device_jpeg *) dev;
    int ecode = 0;
    int code;
    gs_param_name param_name;
    int jq = jdev->JPEGQ;
    float qf = jdev->QFactor;
    float fparam;
    int t;

    switch (code = param_read_int(plist, (param_name = "JPEGQ"), &jq)) {
	case 0:
	    if (jq < 0 || jq > 100)
		ecode = gs_error_limitcheck;
	    else
		break;
	    goto jqe;
	default:
	    ecode = code;
	  jqe:param_signal_error(plist, param_name, ecode);
	case 1:
	    break;
    }

    switch (code = param_read_float(plist, (param_name = "QFactor"), &qf)) {
	case 0:
	    if (qf < 0.0 || qf > 1.0e6)
		ecode = gs_error_limitcheck;
	    else
		break;
	    goto qfe;
	default:
	    ecode = code;
	  qfe:param_signal_error(plist, param_name, ecode);
	case 1:
	    break;
    }


    code = param_read_float(plist, (param_name = "ViewScaleX"), &fparam);
    if ( code == 0 ) {
	if (fparam < 1.0)
	    param_signal_error(plist, param_name, gs_error_limitcheck);
	else
	    jdev->ViewScale.x = fparam;
    }
    else if ( code < 1 ) {
	ecode = code;
	param_signal_error(plist, param_name, code);
    }

    code = param_read_float(plist, (param_name = "ViewScaleY"), &fparam);
    if ( code == 0 ) {
	if (fparam < 1.0)
	    param_signal_error(plist, param_name, gs_error_limitcheck);
	else
	    jdev->ViewScale.y = fparam;
    }
    else if ( code < 1 ) {
	ecode = code;
	param_signal_error(plist, param_name, code);
    }

    /* pixels in desired dpi, auto negative ( moves up and left ) */ 
    code = param_read_float(plist, (param_name = "ViewTransX"), &fparam);
    if ( code == 0 ) {
	jdev->ViewTrans.x = fparam;
    }
    else if ( code < 1 ) {
	ecode = code;
	param_signal_error(plist, param_name, code);
    }

    code = param_read_float(plist, (param_name = "ViewTransY"), &fparam);
    if ( code == 0 ) {
	jdev->ViewTrans.y = fparam;
    }
    else if ( code < 1 ) {
	ecode = code;
	param_signal_error(plist, param_name, code);
    }  

    /* set up resolution and page size before TrayOrientation */
    code = gdev_prn_put_params(dev, plist);
    if (code < 0)
	return code;

    if ((code = param_read_int(plist, "TrayOrientation", &t)) != 1 ) {
        if (code < 0)
            ecode = code;
        else if (t != 0 && t != 90 && t != 180 && t != 270)
            param_signal_error(plist, "TrayOrientation",
                               ecode = gs_error_rangecheck);
        else {
            if ( t != jdev->TrayOrientation) {
                if ( t == 90 || t == 270 ) {
		    /* page sizes don't rotate, height and width do rotate 
		     * HWResolution, HWSize, and MediaSize parameters interact, 
		     * and must be set before TrayOrientation
		     */
                    floatp tmp = jdev->height;
                    jdev->height = jdev->width;
                    jdev->width = tmp;
                }
                jdev->TrayOrientation = t;
            }
        }
    }
    if (ecode < 0)
	return ecode;

    jdev->JPEGQ = jq;
    jdev->QFactor = qf;
    return 0;
}

/******************************************************************
 This device supports translation and scaling.

0123456  

0PPPPPPP  0 is origin
PPPPPPPP  1 is x1,y1 (2,2)
PP1vvvPP  2 is x2,y2 (6,6)
PPvvvvPP  v is viewport, P is original page
PPvvvvPP
PPPPPP2P
PPPPPPPP
  
Given a view port in pixels starting at x1,y1   
where x1 < width, y1 < height in pixels

ViewScaleX = desired Resolution / HWResolution ; 1.0 default 
ViewScaleY = desired Resolution / HWResolution

HWResolutionX = desired dpi at 1:1 scaling     ; 72dpi default  
HWResolutionY = desired dpi at 1:1 scaling

ViewTransX = x1 * ViewScaleX                   ; 0.0 default 
ViewTransY = y1 * ViewScaleY

if initial matrix multiplies ViewScaleX in then translation is limited to
multiples of the HWResolution.

***************************************************************************/

private void
jpeg_get_initial_matrix(gx_device *dev, gs_matrix *pmat)
{
    gx_device_jpeg *pdev = (gx_device_jpeg *)dev;
    floatp fs_res = (dev->HWResolution[0] / 72.0) * pdev->ViewScale.x; 
    floatp ss_res = (dev->HWResolution[1] / 72.0) * pdev->ViewScale.y; 

    /* NB this device has no paper margins */

    switch(pdev->TrayOrientation) {
    case 0:
        pmat->xx = fs_res;
        pmat->xy = 0;
        pmat->yx = 0;
        pmat->yy = -ss_res;
        pmat->tx = -pdev->ViewTrans.x;
        pmat->ty = (pdev->height * pdev->ViewScale.y) - pdev->ViewTrans.y;
        break;
    case 90:
        pmat->xx = 0;
        pmat->xy = -ss_res;
        pmat->yx = -fs_res;
        pmat->yy = 0;
        pmat->tx = (pdev->width * pdev->ViewScale.x) - pdev->ViewTrans.x;
        pmat->ty = (pdev->height * pdev->ViewScale.y) - pdev->ViewTrans.y;
        break;
    case 180:
        pmat->xx = -fs_res;
        pmat->xy = 0;
        pmat->yx = 0;
        pmat->yy = ss_res;
        pmat->tx = (pdev->width * pdev->ViewScale.x) - pdev->ViewTrans.x;
        pmat->ty = -pdev->ViewTrans.x;
        break;
    case 270:
        pmat->xx = 0;
        pmat->xy = ss_res;
        pmat->yx = fs_res;
        pmat->yy = 0;
        pmat->tx = -pdev->ViewTrans.x;
        pmat->ty = -pdev->ViewTrans.y;
        break;
    default:
        /* not reached */
    }

}

/* Send the page to the file. */
private int
jpeg_print_page(gx_device_printer * pdev, FILE * prn_stream)
{
    gx_device_jpeg *jdev = (gx_device_jpeg *) pdev;
    gs_memory_t *mem = pdev->memory;
    int line_size = gdev_mem_bytes_per_scan_line((gx_device *) pdev);
    byte *in = gs_alloc_bytes(mem, line_size, "jpeg_print_page(in)");
    jpeg_compress_data *jcdp = gs_alloc_struct_immovable(mem, jpeg_compress_data,
      &st_jpeg_compress_data, "jpeg_print_page(jpeg_compress_data)");
    byte *fbuf = 0;
    uint fbuf_size;
    byte *jbuf = 0;
    uint jbuf_size;
    int lnum;
    int code;
    stream_DCT_state state;
    stream fstrm, jstrm;

    if (jcdp == 0 || in == 0) {
	code = gs_note_error(gs_error_VMerror);
	goto fail;
    }
    /* Create the DCT decoder state. */
    jcdp->template = s_DCTE_template;
    state.template = &jcdp->template;
    state.memory = 0;
    if (state.template->set_defaults)
	(*state.template->set_defaults) ((stream_state *) & state);
    state.QFactor = 1.0;	/* disable quality adjustment in zfdcte.c */
    state.ColorTransform = 1;	/* default for RGB */
    /* We insert no markers, allowing the IJG library to emit */
    /* the format it thinks best. */
    state.NoMarker = true;	/* do not insert our own Adobe marker */
    state.Markers.data = 0;
    state.Markers.size = 0;
    state.data.compress = jcdp;
    jcdp->memory = state.jpeg_memory = mem;
    if ((code = gs_jpeg_create_compress(&state)) < 0)
	goto fail;
    jcdp->cinfo.image_width = pdev->width;
    jcdp->cinfo.image_height = pdev->height;
    switch (pdev->color_info.depth) {
	case 24:
	    jcdp->cinfo.input_components = 3;
	    jcdp->cinfo.in_color_space = JCS_RGB;
	    break;
	case 8:
	    jcdp->cinfo.input_components = 1;
	    jcdp->cinfo.in_color_space = JCS_GRAYSCALE;
	    break;
    }
    /* Set compression parameters. */
    if ((code = gs_jpeg_set_defaults(&state)) < 0)
	goto done;
    if (jdev->JPEGQ > 0) {
	code = gs_jpeg_set_quality(&state, jdev->JPEGQ, TRUE);
	if (code < 0)
	    goto done;
    } else if (jdev->QFactor > 0.0) {
	code = gs_jpeg_set_linear_quality(&state,
					  (int)(min(jdev->QFactor, 100.0)
						* 100.0 + 0.5),
					  TRUE);
	if (code < 0)
	    goto done;
    }
    jcdp->cinfo.restart_interval = 0;
    jcdp->cinfo.density_unit = 1;	/* dots/inch (no #define or enum) */
    jcdp->cinfo.X_density = pdev->HWResolution[0];
    jcdp->cinfo.Y_density = pdev->HWResolution[1];
    /* Create the filter. */
    /* Make sure we get at least a full scan line of input. */
    state.scan_line_size = jcdp->cinfo.input_components *
	jcdp->cinfo.image_width;
    jcdp->template.min_in_size =
	max(s_DCTE_template.min_in_size, state.scan_line_size);
    /* Make sure we can write the user markers in a single go. */
    jcdp->template.min_out_size =
	max(s_DCTE_template.min_out_size, state.Markers.size);

    /* Set up the streams. */
    fbuf_size = max(512 /* arbitrary */ , jcdp->template.min_out_size);
    jbuf_size = jcdp->template.min_in_size;
    if ((fbuf = gs_alloc_bytes(mem, fbuf_size, "jpeg_print_page(fbuf)")) == 0 ||
	(jbuf = gs_alloc_bytes(mem, jbuf_size, "jpeg_print_page(jbuf)")) == 0
	) {
	code = gs_note_error(gs_error_VMerror);
	goto done;
    }
    swrite_file(&fstrm, prn_stream, fbuf, fbuf_size);
    s_std_init(&jstrm, jbuf, jbuf_size, &s_filter_write_procs,
	       s_mode_write);
    jstrm.memory = mem;
    jstrm.state = (stream_state *) & state;
    jstrm.procs.process = state.template->process;
    jstrm.strm = &fstrm;
    if (state.template->init)
	(*state.template->init) (jstrm.state);

    /* Copy the data to the output. */
    for (lnum = 0; lnum < pdev->height; ++lnum) {
	byte *data;
	uint ignore_used;

	gdev_prn_get_bits(pdev, lnum, in, &data);
	sputs(&jstrm, data, state.scan_line_size, &ignore_used);
    }

    /* Wrap up. */
    sclose(&jstrm);
    sflush(&fstrm);
    jcdp = 0;
  done:
    gs_free_object(mem, jbuf, "jpeg_print_page(jbuf)");
    gs_free_object(mem, fbuf, "jpeg_print_page(fbuf)");
    if (jcdp)
	gs_jpeg_destroy(&state);	/* frees *jcdp */
    gs_free_object(mem, in, "jpeg_print_page(in)");
    return code;
  fail:
    if (jcdp)
	gs_free_object(mem, jcdp, "jpeg_print_page(jpeg_compress_data)");
    gs_free_object(mem, in, "jpeg_print_page(in)");
    return code;
#undef jcdp
}
