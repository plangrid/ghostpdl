/* Copyright (C) 1996, 1997 Aladdin Enterprises.  All rights reserved.
   Unauthorized use, copying, and/or distribution prohibited.
 */

/* pxfont.c */
/* PCL XL font operators */

#include "math_.h"
#include "stdio_.h"
#include "string_.h"
#include "gdebug.h"
#include "plvalue.h"
#include "pxoper.h"
#include "pxstate.h"
#include "pxfont.h"
#include "gserrors.h"
#include "gsstruct.h"
#include "gschar.h"
#include "gspaint.h"
#include "gspath.h"
#include "gsstate.h"
#include "gscoord.h"
#include "gsimage.h"
#include "gsutil.h"			/* for string_match */
#include "gxfont.h"
#include "gxfont42.h"
#include "gxfixed.h"
#include "gxchar.h"
#include "gxpath.h"
#include "gzstate.h"

/*
 * There appears to be a combination of undocumented behavior and
 * firmware bugs in the H-P printers that make it difficult to determine
 * the proper interaction of character and page transformations.
 * The following two conditionals probably should not be enabled,
 * but we leave them here just in case this turns out to be wrong.
 */
/*#define TRANSFORM_TEXT_AS_OBJECTS*/
/*#define POST_TRANSFORM_CHARS*/

/* ---------------- Initialization ---------------- */

int
pxfont_init(px_state_t *pxs)
{	/* Allocate the font directory now. */
	pxs->font_dir = gs_font_dir_alloc(pxs->memory);
	if ( pxs->font_dir == 0 )
	  return_error(errorInsufficientMemory);
	return 0;
}

/* ---------------- Operator utilities ---------------- */

/* Compute the symbol map from the font and symbol set. */
private void
set_symbol_map(px_state_t *pxs)
{	px_gstate_t *pxgs = pxs->pxgs;
	px_font_t *pxfont = pxgs->base_font;
	uint symbol_set = pxgs->symbol_set;

	if ( symbol_set == pxfont->params.symbol_set )
	  { /* Exact match, no mapping required. */
	    pxgs->symbol_map = 0;
	  }
	else if ( pl_font_is_bound(pxfont) )
	  { /****** CAN'T REMAP YET ******/
	    pxgs->symbol_map = 0;
	  }
	else
	  { /* See if we know about the requested symbol set. */
	    /****** ASSUME UNICODE ******/
	    const pl_symbol_map_t **ppsm = pl_built_in_symbol_maps;

	    while ( *ppsm != 0 && pl_get_uint16((*ppsm)->id) != symbol_set )
	      ++ppsm;
	    /* If we didn't find it, default to Roman-8. */
	    pxgs->symbol_map = (*ppsm ? *ppsm : pl_built_in_symbol_maps[0]);
	  }
}

/* Recompute the combined character matrix from character and font */
/* parameters.  Note that this depends on the current font. */
private double
adjust_scale(double scale)
{	double int_scale = floor(scale + 0.5);
	double diff = scale - int_scale;

	/* If the scale is very close to an integer, make it integral, */
	/* so we won't have to scale the bitmap (or can scale it fast). */
	return (diff < 0.001 && diff > -0.001 ? int_scale : scale);
}
private int
px_set_char_matrix(px_state_t *pxs)
{	px_gstate_t *pxgs = pxs->pxgs;
	px_font_t *pxfont = pxgs->base_font;
	gs_matrix mat;

	if ( pxfont == 0 )
	  return_error(errorNoCurrentFont);
	if ( pxfont->scaling_technology == plfst_bitmap )
	  { /*
	     * Bitmaps don't scale, shear, or rotate; however, we have to
	     * scale them to make the resolution match that of the device.
	     * Note that we disregard the character size, and, in px_text,
	     * all but the translation and orientation components of the
	     * CTM.
	     */
	    if ( pxgs->char_angle != 0 ||
		 pxgs->char_shear.x != 0 || pxgs->char_shear.y != 0 ||
		 pxgs->char_scale.x != 1 || pxgs->char_scale.y != 1
	       )
	      return_error(errorIllegalFontData);
	    gs_defaultmatrix(pxs->pgs, &mat);
	    gs_make_scaling(
	      adjust_scale(fabs(mat.xx + mat.yx) * 72 / pxfont->resolution.x),
	      adjust_scale(fabs(mat.yy + mat.xy) * 72 / pxfont->resolution.y),
	      &mat);
	    /*
	     * Rotate the bitmap to undo the effect of its built-in
	     * orientation.
	     */
	    gs_matrix_rotate(&mat, pxfont->header[1] * 90, &mat);
	  }
	else
	  { float char_size = pxgs->char_size;
	    int i;

	    gs_make_identity(&mat);
	    /* H-P and Microsoft have Y coordinates running opposite ways. */
	    gs_matrix_scale(&mat, char_size, -char_size, &mat);
	    /* Apply the character transformations in the reverse order. */
	    for ( i = 0; i < 3; ++i )
	      switch ( pxgs->char_transforms[i] )
		{
		case pxct_rotate:
		  if ( pxgs->char_angle != 0 )
		    gs_matrix_rotate(&mat, pxgs->char_angle, &mat);
		  break;
		case pxct_shear:
		  if ( pxgs->char_shear.x != 0 || pxgs->char_shear.y != 0 )
		    { gs_matrix smat;
		      gs_make_identity(&smat);
		      smat.yx = pxgs->char_shear.x;
		      smat.xy = pxgs->char_shear.y;
		      gs_matrix_multiply(&smat, &mat, &mat);
		    }
		  break;
		case pxct_scale:
		  if ( pxgs->char_scale.x != 1 || pxgs->char_scale.y != 1 )
		    gs_matrix_scale(&mat, pxgs->char_scale.x,
				    pxgs->char_scale.y, &mat);
		  break;
		}
	  }
	pxgs->char_matrix = mat;
	pxgs->char_matrix_set = true;
	return 0;
}

/* ---------------- Operator implementations ---------------- */

/* Define a font.  The caller must fill in pxfont->storage and ->font_type. */
/* This procedure implements FontHeader loading; it is exported for */
/* initializing the error page font. */
int
px_define_font(px_font_t *pxfont, const byte *header, ulong size, gs_id id,
  px_state_t *pxs)
{	gs_memory_t *mem = pxs->memory;
	uint num_chars;

	/* Check for a valid font. */
	if ( size < 8 /* header */ + 6 /* 1 required segment */ +
	       6 /* NULL segment */
	   )
	  return_error(errorIllegalFontData);
	if ( header[0] != 0 /* format */ ||
	     header[5] != 0 /* variety */
	   )
	  return_error(errorIllegalFontHeaderFields);
	pxfont->header = header;
	pxfont->header_size = size;
	{ static const pl_font_offset_errors_t errors = {
	    errorIllegalFontData,
	    errorIllegalFontSegment,
	    errorIllegalFontHeaderFields,
	    errorIllegalNullSegmentSize,
	    errorMissingRequiredSegment,
	    errorIllegalGlobalTrueTypeSegment,
	    errorIllegalGalleyCharacterSegment,
	    errorIllegalVerticalTxSegment,
	    errorIllegalBitmapResolutionSegment
	  };
	  int code = pl_font_scan_segments(pxfont, 4, 8, size, true, &errors);

	  if ( code < 0 )
	    return code;
	}
	num_chars = pl_get_uint16(header + 6);
	/* Allocate the character table. */
	{ /* Some fonts ask for unreasonably large tables.... */
	  int code = pl_font_alloc_glyph_table(pxfont, min(num_chars, 300),
					       mem, "px_define_font(glyphs)");
	  if ( code < 0 )
	    return code;
	}
	/* Now construct a gs_font. */
	if ( pxfont->scaling_technology == plfst_bitmap )
	  { /* Bitmap font. */
	    gs_font_base *pfont =
	      gs_alloc_struct(mem, gs_font_base, &st_gs_font_base,
			      "px_define_font(gs_font_base)");
	    int code;

	    if ( pfont == 0 )
	      return_error(errorInsufficientMemory);
	    code = px_fill_in_font((gs_font *)pfont, pxfont, pxs);
	    if ( code < 0 )
	      return code;
	    pl_fill_in_bitmap_font(pfont, id);
	  }
	else
	  { /* TrueType font. */
	    gs_font_type42 *pfont =
	      gs_alloc_struct(mem, gs_font_type42, &st_gs_font_type42,
			      "px_define_font(gs_font_type42)");
	    int code;

	    if ( pfont == 0 )
	      return_error(errorInsufficientMemory);
	    /* Some fonts ask for unreasonably large tables.... */
	    code = pl_tt_alloc_char_glyphs(pxfont, min(num_chars, 300), mem,
					   "px_define_font(char_glyphs)");
	    if ( code < 0 )
	      return code;
	    code = px_fill_in_font((gs_font *)pfont, pxfont, pxs);
	    if ( code < 0 )
	      return code;
	    pl_fill_in_tt_font(pfont, NULL, id);
	  }
	pxfont->params.symbol_set = pl_get_uint16(header + 2);
	return gs_definefont(pxs->font_dir, pxfont->pfont);
}

/* Concatenate a widened (16-bit) font name onto an error message string. */
void
px_concat_font_name(char *message, uint max_message, const px_value_t *pfnv)
{	char *mptr = message + strlen(message);
	uint fnsize = pfnv->value.array.size;
	uint i;

	/*
	 **** We truncate 16-bit font name chars to 8 bits
	 **** for the message.
	 */
	for ( i = 0; i < fnsize && mptr - message < max_message; ++mptr, ++i )
	  if ( (*mptr = (byte)integer_elt(pfnv, i)) < 32 )
	    *mptr = '?';
	*mptr = 0;
}

/* Paint text or add it to the path. */
/* This procedure implements the Text and TextPath operators. */
/* Attributes: pxaTextData, pxaXSpacingData, pxaYSpacingData. */
private font_proc_next_char(px_next_char_8);
private font_proc_next_char(px_next_char_16big);
private font_proc_next_char(px_next_char_16little);
int
px_text(px_args_t *par, px_state_t *pxs, bool to_path)
{	gs_memory_t *mem = pxs->memory;
	gs_state *pgs = pxs->pgs;
	px_gstate_t *pxgs = pxs->pxgs;
	gs_show_enum *penum;
	const px_value_t *pstr = par->pv[0];
	int index_shift = (pstr->type & pxd_ubyte ? 0 : 1);
	const char *str = (const char *)pstr->value.array.data;
	uint len = pstr->value.array.size;
	const px_value_t *pxdata = par->pv[1];
	const px_value_t *pydata = par->pv[2];
       	gs_matrix save_ctm;
	gs_font *pfont = gs_currentfont(pgs);
	font_proc_next_char((*save_next_char)) = pfont->procs.next_char;
	int code = 0;

	if ( (pxdata != 0 && pxdata->value.array.size != len) ||
	     (pydata != 0 && pydata->value.array.size != len)
	   )
	  return_error(errorIllegalArraySize);
	if ( !pxgs->base_font )
	  return_error(errorNoCurrentFont);
	if ( !pxgs->char_matrix_set )
	  { code = px_set_char_matrix(pxs);
	    if ( code < 0 )
	      return code;
	  }
	penum = gs_show_enum_alloc(mem, pgs, "px_text");
	if ( penum == 0 )
	  return_error(errorInsufficientMemory);
	gs_currentmatrix(pgs, &save_ctm);
#if 0
	/*
	 * I don't know why this code was here (briefly); if it turns out
	 * to be important, we can put it back in, but it breaks Genoa FTS
	 * t202 images 3-5.
	 */
	if ( pxgs->base_font->scaling_technology == plfst_bitmap )
	  { /*
	     * Bitmap fonts don't scale or rotate: the char_matrix is the
	     * actual matrix to use, aside from translation.
	     */
	    gs_matrix mat;
	    mat = pxgs->char_matrix;
	    mat.tx = save_ctm.tx;
	    mat.ty = save_ctm.ty;
	    gs_setmatrix(pgs, &mat);
	  }
	else
#endif
#ifdef TRANSFORM_TEXT_AS_OBJECTS
	/* Keep the current translation, but post-scale and rotate. */
	{ gs_matrix tmat, rmat;
	  tmat = pxs->initial_matrix;
	  gs_matrix_rotate(&tmat, 90 * (int)pxs->orientation, &tmat);
	  gs_matrix_multiply(&tmat, &pxgs->text_ctm, &tmat);
	  gs_make_rotation(-90 * (int)pxs->orientation, &rmat);
	  gs_matrix_multiply(&tmat, &rmat, &tmat);
	  tmat.tx = save_ctm.tx;
	  tmat.ty = save_ctm.ty;
	  gs_setmatrix(pgs, &tmat);
	}
#endif
#ifdef POST_TRANSFORM_CHARS
	{ gs_matrix cmat, mat;
	  gs_currentmatrix(pgs, &cmat);
	  gs_matrix_multiply(&cmat, &pxgs->char_matrix, &mat);
	  gs_setmatrix(pgs, &mat);
	}
#else
	gs_concat(pgs, &pxgs->char_matrix);
#endif
	pfont->procs.next_char =
	  (!index_shift ? px_next_char_8 :
	   pstr->type & pxd_big_endian ? px_next_char_16big :
	   px_next_char_16little);
	pfont->WMode =
	  (pxgs->char_sub_mode == eVerticalSubstitution ? 1 : 0) +
	  ((int)(pxgs->char_bold_value * 127) * 2);
	if ( to_path )
	  { /* TextPath */
	    /*
	     **** We really want a combination of charpath & kshow,
	     **** maybe by adding flags to show*_init?
	     * Lacking this, we have to add each character individually.
	     */
	    uint i;
	    for ( i = 0; i < len; ++i )
	      { gs_fixed_point origin, dist;
		code = gx_path_current_point(pgs->path, &origin);
		if ( code < 0 )
		  break;
		code = gs_charpath_n_init(penum, pgs,
					  str + (i << index_shift), 1, false);
		if ( code < 0 )
		  break;
		code = gs_show_next(penum);
		if ( code != 0 )
		  { if ( code > 0 )
		      code = gs_note_error(errorBadFontData);	/* shouldn't happen! */
		    break;
		  }
		/* We cheat here, knowing that gs_d_t2f doesn't actually */
		/* require a gs_matrix_fixed but only a gs_matrix. */
		gs_distance_transform2fixed((const gs_matrix_fixed *)&save_ctm,
					(pxdata ? real_elt(pxdata, i) : 0.0),
					(pydata ? real_elt(pydata, i) : 0.0),
					&dist);
		code = gx_path_add_point(pgs->path,
					 origin.x + dist.x, origin.y + dist.y);
		if ( code < 0 )
		  break;
	      }
	  }
	else
	  { /* Text */
	    uint i;
	    code = gs_xyshow_n_init(penum, pgs, str, len);
	    if ( code < 0 )
	      goto x;
	    for ( i = 0; i < len; ++i )
	      { gs_fixed_point dist;
		code = gs_show_next(penum);
		if ( code != gs_show_move )
		  { if ( code >= 0 )
		      code = gs_note_error(errorBadFontData);	/* shouldn't happen! */
		    break;
		  }
		/* See above re the cast in the next line. */
		gs_distance_transform2fixed((const gs_matrix_fixed *)&save_ctm,
					(pxdata ? real_elt(pxdata, i) : 0.0),
					(pydata ? real_elt(pydata, i) : 0.0),
					&dist);
		code = gx_path_add_relative_point(pgs->path, dist.x, dist.y);
		if ( code < 0 )
		  break;
	      }
	    if ( code >= 0 )
	      { code = gs_show_next(penum);
	        if ( code != 0 )
		  { if ( code >= 0 )
		      code = gs_note_error(errorBadFontData);	/* shouldn't happen! */
		  }
	      }
	  }
x:	if ( code < 0 )
	  gs_show_enum_release(penum, mem);
	else
	  gs_free_object(mem, penum, "px_text");
	gs_setmatrix(pgs, &save_ctm);
	pfont->WMode = 0;
	pfont->procs.next_char = save_next_char;
	return (code == gs_error_invalidfont ?
		gs_note_error(errorBadFontData) : code);
}
/* Next-character procedures, with symbol mapping. */
private gs_char
map_symbol(uint chr, const gs_show_enum *penum)
{	px_gstate_t *pxgs = gs_state_client_data(penum->pgs);
	const pl_symbol_map_t *psm = pxgs->symbol_map;
	uint first_code;

	if ( psm == 0 )
	  return chr;
	first_code = pl_get_uint16(psm->first_code);
	if ( chr < first_code || chr > pl_get_uint16(psm->last_code) )
	  return 0xffff;
	return psm->codes[chr - first_code];
}
private int
px_next_char_8(gs_show_enum *penum, gs_char *pchr)
{	if ( penum->index == penum->text.size )
	  return 2;
	*pchr = map_symbol(penum->text.data.bytes[penum->index++], penum);
	return 0;
}
private int
px_next_char_16big(gs_show_enum *penum, gs_char *pchr)
{	if ( penum->index == penum->text.size )
	  return 2;
	*pchr = map_symbol(uint16at(&penum->text.data.bytes[penum->index++ << 1], true), penum);
	return 0;
}
private int
px_next_char_16little(gs_show_enum *penum, gs_char *pchr)
{	if ( penum->index == penum->text.size )
	  return 2;
	*pchr = map_symbol(uint16at(&penum->text.data.bytes[penum->index++ << 1], false), penum);
	return 0;
}

/* ---------------- Operators ---------------- */

const byte apxSetFont[] = {
  pxaFontName, pxaCharSize, pxaSymbolSet, 0, 0
};
int
pxSetFont(px_args_t *par, px_state_t *pxs)
{	px_gstate_t *pxgs = pxs->pxgs;
	px_font_t *pxfont;
	px_value_t *pfnv = par->pv[0];
	uint symbol_set = par->pv[2]->value.i;
	int code = px_find_font(pfnv, symbol_set, &pxfont, pxs);

	if ( code < 0 )
	  { switch ( code )
	    {
	    case errorFontUndefined:
	      strcpy(pxs->error_line, "FontUndefined - ");
	      goto undef;
	    case errorFontUndefinedNoSubstituteFound:
	      strcpy(pxs->error_line, "FontUndefinedNoSubstituteFound - ");
undef:	      px_concat_font_name(pxs->error_line, px_max_error_line, pfnv);
	      break;
	    case errorSymbolSetRemapUndefined:
	      strcpy(pxs->error_line, "SymbolSetRemapUndefined - ");
	      px_concat_font_name(pxs->error_line, px_max_error_line, pfnv);
	      { char setstr[26];	/* 64-bit value plus message */
	        sprintf(setstr, " : %d", symbol_set);
		strncat(pxs->error_line, setstr,
			px_max_error_line - strlen(pxs->error_line));
		pxs->error_line[px_max_error_line] = 0;
	      }
	      break;
	    }
	    return code;
	  }
	code = gs_setfont(pxs->pgs, pxfont->pfont);
	if ( code < 0 )
	  return code;
	pxgs->char_size = real_value(par->pv[1], 0);
	pxgs->symbol_set = symbol_set;
	pxgs->base_font = pxfont;
	set_symbol_map(pxs);
	pxgs->char_matrix_set = false;
	return 0;
}

const byte apxBeginFontHeader[] = {
  pxaFontName, pxaFontFormat, 0, 0};
int
pxBeginFontHeader(px_args_t *par, px_state_t *pxs)
{	px_value_t *pfnv = par->pv[0];
	gs_memory_t *mem = pxs->memory;
	px_font_t *pxfont;
	int code = px_find_existing_font(pfnv, &pxfont, pxs);

	if ( code >= 0 )
	  { strcpy(pxs->error_line, "FontNameAlreadyExists - ");
	    px_concat_font_name(pxs->error_line, px_max_error_line, pfnv);
	    return_error(errorFontNameAlreadyExists);
	  }
	/* Make a partially filled-in dictionary entry. */
	pxfont = pl_alloc_font(mem, "pxBeginFontHeader(pxfont)");
	if ( pxfont == 0 )
	  return_error(errorInsufficientMemory);
	pxfont->storage = pxfsDownLoaded;
	pxfont->data_are_permanent = false;
	code = px_dict_put(&pxs->font_dict, par->pv[0], pxfont);
	if ( code < 0 )
	  { gs_free_object(mem, pxfont, "pxBeginFontHeader(pxfont)");
	    return code;
	  }
	pxs->download_font = pxfont;
	pxs->download_bytes.data = 0;
	pxs->download_bytes.size = 0;
	return 0;
}

const byte apxReadFontHeader[] = {
  pxaFontHeaderLength, 0, 0
};
int
pxReadFontHeader(px_args_t *par, px_state_t *pxs)
{	ulong len = par->pv[0]->value.i;
	ulong left = len - par->source.position;
	int code = pxNeedData;

	if ( left > 0 )
	  { ulong pos;
	    if ( par->source.position == 0 )
	      { /* (Re-)allocate the downloaded data. */
		void *new_data;

		if ( par->source.available == 0 )
		  return code;
		new_data =
		  (pxs->download_bytes.size == 0 ?
		   gs_alloc_bytes(pxs->memory, len, "pxReadFontHeader") :
		   gs_resize_object(pxs->memory, pxs->download_bytes.data,
				    pxs->download_bytes.size + len,
				    "pxReadFontHeader"));
		if ( new_data == 0 )
		  return_error(errorInsufficientMemory);
		pxs->download_bytes.data = new_data;
		pxs->download_bytes.size += len;
	      }
	    if ( left > par->source.available )
	      left = par->source.available;
	    else
	      code = 0;
	    pos = pxs->download_bytes.size - len + par->source.position;
	    memcpy(pxs->download_bytes.data + pos, par->source.data, left);
	    par->source.position += left;
	    par->source.data += left;
	    par->source.available -= left;
	    if ( pos < 8 && pos + left >= 8 )
	      { /* Check the font header fields now. */
		const byte *data = pxs->download_bytes.data;
		if ( data[0] | data[5] )
		  return_error(errorIllegalFontHeaderFields);
		switch ( data[4] )
		  {
		  case plfst_TrueType:
		    if ( data[1] )
		      return_error(errorIllegalFontHeaderFields);
		    break;
		  case plfst_bitmap:
		    if ( data[1] & ~3 )
		      return_error(errorIllegalFontHeaderFields);
		    break;
		  default:
		    return_error(errorIllegalFontHeaderFields);
		  }
	      }
	  }
	return code;
}

const byte apxEndFontHeader[] = {0, 0};
int
pxEndFontHeader(px_args_t *par, px_state_t *pxs)
{	px_font_t *pxfont = pxs->download_font;
	int code = px_define_font(pxfont, pxs->download_bytes.data,
				  (ulong)pxs->download_bytes.size,
				  gs_next_ids(1), pxs);

	/****** HOW TO DETERMINE FONT TYPE? ******/
	pxfont->font_type = plft_16bit;
	/* Clear pointers for GC */
	pxs->download_font = 0;
	pxs->download_bytes.data = 0;
	return code;
}

const byte apxBeginChar[] = {
  pxaFontName, 0, 0
};
int
pxBeginChar(px_args_t *par, px_state_t *pxs)
{	px_value_t *pfnv = par->pv[0];
	px_font_t *pxfont;
	int code = px_find_existing_font(pfnv, &pxfont, pxs);

	if ( code >= 0 && pxfont == 0 )
	  code = gs_note_error(errorFontUndefined);
	if ( code < 0 )
	  { if ( code == errorFontUndefined )
	      { strcpy(pxs->error_line, "FontUndefined - ");
	        px_concat_font_name(pxs->error_line, px_max_error_line, pfnv);
	      }
	    return code;
	  }
	if ( pxfont->storage != pxfsDownLoaded )
	  return_error(errorCannotReplaceCharacter);
	pxs->download_font = pxfont;
	return 0;
}

const byte apxReadChar[] = {
  pxaCharCode, pxaCharDataSize, 0, 0
};
int
pxReadChar(px_args_t *par, px_state_t *pxs)
{	uint char_code = par->pv[0]->value.i;
	uint size = par->pv[1]->value.i;
	uint pos = par->source.position;

	if ( pos == 0 )
	  { /* We're starting a character definition. */
	    byte *def;

	    if ( size < 2 )
	      return_error(errorIllegalCharacterData);
	    if ( par->source.available == 0 )
	      return pxNeedData;
	    def = gs_alloc_bytes(pxs->memory, size, "pxReadChar");
	    if ( def == 0 )
	      return_error(errorInsufficientMemory);
	    pxs->download_bytes.data = def;
	    pxs->download_bytes.size = size;
	  }
	while ( pos < size )
	  { uint copy = min(par->source.available, size - pos);

	    if ( copy == 0 )
	      return pxNeedData;
	    memcpy(pxs->download_bytes.data + pos, par->source.data, copy);
	    par->source.data += copy;
	    par->source.available -= copy;
	    par->source.position = pos += copy;
	  }
	/* We have the complete character. */
	/* Do error checks before installing. */
	{ const byte *header = pxs->download_font->header;
	  const byte *data = pxs->download_bytes.data;
	  int code = 0;

	  switch ( data[0] )
	    {
	    case 0:		/* bitmap */
	      if ( header[4] != plfst_bitmap )
		code = gs_note_error(errorFSTMismatch);
	      else if ( data[1] != 0 )
		code = gs_note_error(errorUnsupportedCharacterClass);
	      else if ( size < 10 ||
			size != 10 + ((pl_get_uint16(data + 6) + 7) >> 3) *
			  pl_get_uint16(data + 8)
		      )
		code = gs_note_error(errorIllegalCharacterData);
	      break;
	    case 1:		/* TrueType outline */
	      if ( header[4] != plfst_TrueType )
		code = gs_note_error(errorFSTMismatch);
	      else if ( data[1] != 0 )
		code = gs_note_error(errorUnsupportedCharacterClass);
	      else if ( size < 6 || size != 2 + pl_get_uint16(data + 2) )
		code = gs_note_error(errorIllegalCharacterData);
	      break;
	    default:
	      code = gs_note_error(errorUnsupportedCharacterFormat);
	    }
	  if ( code >= 0 )
	    { code = pl_font_add_glyph(pxs->download_font, char_code, data);
	      if ( code < 0 )
		code = gs_note_error(errorInternalOverflow);
	    }
	  if ( code < 0 )
	    gs_free_object(pxs->memory, pxs->download_bytes.data,
			   "pxReadChar");
	  pxs->download_bytes.data = 0;
	  return code;
	}
}

const byte apxEndChar[] = {0, 0};
int
pxEndChar(px_args_t *par, px_state_t *pxs)
{	return 0;
}

const byte apxRemoveFont[] = {
  pxaFontName, 0, 0
};
int
pxRemoveFont(px_args_t *par, px_state_t *pxs)
{	px_value_t *pfnv = par->pv[0];
	px_font_t *pxfont;
	int code = px_find_existing_font(pfnv, &pxfont, pxs);
	const char *error = 0;

	if ( code < 0 )
	  error = "UndefinedFontNotRemoved - ";
	else if ( pxfont == 0 )	/* built-in font, assume internal */
	  error = "InternalFontNotRemoved - ";
	else
	  switch ( pxfont->storage )
	    {
	    case pxfsInternal:
	      error = "InternalFontNotRemoved - ";
	      break;
	    case pxfsMassStorage:
	      error = "MassStorageFontNotRemoved - ";
	      break;
	    default:		/* downloaded */
	      ;
	    }
	if ( error )
	  { /* Construct a warning message including the font name. */
	    char message[px_max_error_line + 1];

	    strcpy(message, error);
	    px_concat_font_name(message, px_max_error_line, pfnv);
	    code = px_record_warning(message, false, pxs);
	  }
	/****** WHAT IF THIS IS THE CURRENT FONT? ******/
	px_dict_undef(&pxs->font_dict, par->pv[0]);
	return 0;
}
