/* Copyright (C) 1997, 2000 Aladdin Enterprises.  All rights reserved.
  
  This software is provided AS-IS with no warranty, either express or
  implied.
  
  This software is distributed under license and may not be copied,
  modified or distributed except as expressly authorized under the terms
  of the license contained in the file LICENSE in this distribution.
  
  For more information about licensing, please refer to
  http://www.ghostscript.com/licensing/. For information on
  commercial licensing, go to http://www.artifex.com/licensing/ or
  contact Artifex Software, Inc., 101 Lucas Valley Road #110,
  San Rafael, CA  94903, U.S.A., +1(415)492-9861.
*/

/*$RCSfile$ $Revision$ */
/* DevicePixel color space and operation definition */
#include "gx.h"
#include "gserrors.h"
#include "gsrefct.h"
#include "gxcspace.h"
#include "gscpixel.h"
#include "gxdevice.h"

/* Define the DevicePixel color space type. */
private cs_proc_equal(gx_equal_DevicePixel);
private cs_proc_restrict_color(gx_restrict_DevicePixel);
private cs_proc_remap_concrete_color(gx_remap_concrete_DevicePixel);
private cs_proc_concretize_color(gx_concretize_DevicePixel);
private const gs_color_space_type gs_color_space_type_DevicePixel = {
    gs_color_space_index_DevicePixel, true, false,
    &st_base_color_space, gx_num_components_1,
    gx_no_base_space, gx_equal_DevicePixel,
    gx_init_paint_1, gx_restrict_DevicePixel,
    gx_same_concrete_space,
    gx_concretize_DevicePixel, gx_remap_concrete_DevicePixel,
    gx_default_remap_color, gx_no_install_cspace,
    gx_no_adjust_cspace_count, gx_no_adjust_color_count
};

/* Test whether one DevicePixel color space equals another. */
private bool
gx_equal_DevicePixel(const gs_color_space *pcs1, const gs_color_space *pcs2)
{
    return pcs1->params.pixel.depth == pcs2->params.pixel.depth;
}

/* Initialize a DevicePixel color space. */
int
gs_cspace_init_DevicePixel(gs_color_space * pcs, int depth)
{
    switch (depth) {
	case 1:
	case 2:
	case 4:
	case 8:
	case 16:
	case 24:
	case 32:
	    break;
	default:
	    return_error(gs_error_rangecheck);
    }
    gs_cspace_init(pcs, &gs_color_space_type_DevicePixel, NULL);
    pcs->params.pixel.depth = depth;
    return 0;
}

/* ------ Internal routines ------ */

/* Force a DevicePixel color into legal range. */
private void
gx_restrict_DevicePixel(gs_client_color * pcc, const gs_color_space * pcs)
{
    /****** NOT ENOUGH BITS IN float OR frac ******/
    floatp pixel = pcc->paint.values[0];
    ulong max_value = (1L << pcs->params.pixel.depth) - 1;

    pcc->paint.values[0] = (pixel < 0 ? 0 : min(pixel, max_value));
}


/* Remap a DevicePixel color. */

private int
gx_concretize_DevicePixel(const gs_client_color * pc, const gs_color_space * pcs,
			  frac * pconc, const gs_imager_state * pis)
{
    /****** NOT ENOUGH BITS IN float OR frac ******/
    pconc[0] = (frac) (ulong) pc->paint.values[0];
    return 0;
}

private int
gx_remap_concrete_DevicePixel(const frac * pconc,
	gx_device_color * pdc, const gs_imager_state * pis, gx_device * dev,
			      gs_color_select_t select)
{
    color_set_pure(pdc, pconc[0] & ((1 << dev->color_info.depth) - 1));
    return 0;
}
