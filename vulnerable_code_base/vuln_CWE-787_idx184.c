parse_paragraph(tree_t *t,	 
        	float  left,	 
        	float  right,	 
        	float  bottom,	 
        	float  top,	 
        	float  *x,	 
        	float  *y,	 
        	int    *page,	 
        	int    needspace) 
{
  int		whitespace;	 
  tree_t	*flat,
		*start,
		*end,
		*prev,
		*temp;
  float		width,
		height,
		offset,
		spacing,
		borderspace,
		temp_y,
		temp_width,
		temp_height;
  float		format_width, image_y, image_left, image_right;
  int		image_page = *page;
  float		char_spacing;
  int		num_chars;
  render_t	*r;
  uchar		*align,
		*hspace,
		*vspace,
		*link,
		*border;
  float		rgb[3];
  uchar		line[10240],
		*lineptr,
		*dataptr;
  tree_t	*linetype;
  float		linex,
		linewidth;
  int		firstline;
  DEBUG_printf(("parse_paragraph(t=%p, left=%.1f, right=%.1f, bottom=%.1f, top=%.1f, x=%.1f, y=%.1f, page=%d, needspace=%d\n",
                (void *)t, left, right, bottom, top, *x, *y, *page, needspace));
  flat        = flatten_tree(t->child);
  image_left  = left;
  image_right = right;
  image_y     = 0;
  if (flat == NULL)
    DEBUG_puts("parse_paragraph: flat == NULL!");
  if (*y < top && needspace)
    *y -= _htmlSpacings[SIZE_P];
  for (temp = flat, prev = NULL; temp != NULL;)
  {
    if (temp->markup == MARKUP_IMG)
      update_image_size(temp);
    if (temp->markup == MARKUP_IMG &&
        (align = htmlGetVariable(temp, (uchar *)"ALIGN")))
    {
      if ((border = htmlGetVariable(temp, (uchar *)"BORDER")) != NULL)
	borderspace = (float)atof((char *)border);
      else if (temp->link)
	borderspace = 1;
      else
	borderspace = 0;
      borderspace *= PagePrintWidth / _htmlBrowserWidth;
      if (strcasecmp((char *)align, "LEFT") == 0)
      {
        if ((vspace = htmlGetVariable(temp, (uchar *)"VSPACE")) != NULL)
	  *y -= atoi((char *)vspace);
        if (*y < (bottom + temp->height + 2 * borderspace))
        {
	  (*page) ++;
	  *y = top;
	  if (Verbosity)
	    progress_show("Formatting page %d", *page);
        }
        if (borderspace > 0.0f)
	{
	  if (temp->link && PSLevel == 0)
	    memcpy(rgb, link_color, sizeof(rgb));
	  else
	  {
	    rgb[0] = temp->red / 255.0f;
	    rgb[1] = temp->green / 255.0f;
	    rgb[2] = temp->blue / 255.0f;
	  }
          new_render(*page, RENDER_BOX, image_left, *y - borderspace,
		     temp->width + 2 * borderspace, borderspace, rgb);
          new_render(*page, RENDER_BOX, image_left,
	             *y - temp->height - 2 * borderspace,
                     borderspace, temp->height + 2 * borderspace, rgb);
          new_render(*page, RENDER_BOX, image_left + temp->width + borderspace,
	             *y - temp->height - 2 * borderspace,
                     borderspace, temp->height + 2 * borderspace, rgb);
          new_render(*page, RENDER_BOX, image_left,
	             *y - temp->height - 2 * borderspace,
                     temp->width + 2 * borderspace, borderspace, rgb);
	}
        *y -= borderspace;
        new_render(*page, RENDER_IMAGE, image_left + borderspace,
	           *y - temp->height, temp->width, temp->height,
		   image_find((char *)htmlGetVariable(temp, (uchar *)"REALSRC")));
        if (temp->link &&
	    (link = htmlGetVariable(temp->link, (uchar *)"_HD_FULL_HREF")) != NULL)
        {
	  new_render(*page, RENDER_LINK, image_left + borderspace, *y - temp->height, temp->width, temp->height, link);
        }
        *y -= borderspace;
        if (vspace != NULL)
	  *y -= atoi((char *)vspace);
        image_left += temp->width + 2 * borderspace;
	temp_y     = *y - temp->height;
	image_page = *page;
	if (temp_y < image_y || image_y == 0)
	  image_y = temp_y;
        if ((hspace = htmlGetVariable(temp, (uchar *)"HSPACE")) != NULL)
	  image_left += atoi((char *)hspace);
        if (prev != NULL)
          prev->next = temp->next;
        else
          flat = temp->next;
        free(temp);
        temp = prev;
      }
      else if (strcasecmp((char *)align, "RIGHT") == 0)
      {
        if ((vspace = htmlGetVariable(temp, (uchar *)"VSPACE")) != NULL)
	  *y -= atoi((char *)vspace);
        if (*y < (bottom + temp->height + 2 * borderspace))
        {
	  (*page) ++;
	  *y = top;
	  if (Verbosity)
	    progress_show("Formatting page %d", *page);
        }
        image_right -= temp->width + 2 * borderspace;
	image_page = *page;
        if (borderspace > 0.0f)
	{
	  if (temp->link && PSLevel == 0)
	    memcpy(rgb, link_color, sizeof(rgb));
	  else
	  {
	    rgb[0] = temp->red / 255.0f;
	    rgb[1] = temp->green / 255.0f;
	    rgb[2] = temp->blue / 255.0f;
	  }
          new_render(*page, RENDER_BOX, image_right, *y - borderspace,
		     temp->width + 2 * borderspace, borderspace, rgb);
          new_render(*page, RENDER_BOX, image_right,
	             *y - temp->height - 2 * borderspace,
                     borderspace, temp->height + 2 * borderspace, rgb);
          new_render(*page, RENDER_BOX, image_right + temp->width + borderspace,
	             *y - temp->height - 2 * borderspace,
                     borderspace, temp->height + 2 * borderspace, rgb);
          new_render(*page, RENDER_BOX, image_right, *y - temp->height - 2 * borderspace,
                     temp->width + 2 * borderspace, borderspace, rgb);
	}
        *y -= borderspace;
        new_render(*page, RENDER_IMAGE, image_right + borderspace,
	           *y - temp->height, temp->width, temp->height,
		   image_find((char *)htmlGetVariable(temp, (uchar *)"REALSRC")));
        if (temp->link &&
	    (link = htmlGetVariable(temp->link, (uchar *)"_HD_FULL_HREF")) != NULL)
        {
	  new_render(*page, RENDER_LINK, image_right + borderspace, *y - temp->height, temp->width, temp->height, link);
        }
        *y -= borderspace;
        if (vspace != NULL)
	  *y -= atoi((char *)vspace);
	temp_y = *y - temp->height;
	if (temp_y < image_y || image_y == 0)
	  image_y = temp_y;
        if ((hspace = htmlGetVariable(temp, (uchar *)"HSPACE")) != NULL)
	  image_right -= atoi((char *)hspace);
        if (prev != NULL)
          prev->next = temp->next;
        else
          flat = temp->next;
        free(temp);
        temp = prev;
      }
    }
    if (temp != NULL)
    {
      prev = temp;
      temp = temp->next;
    }
    else
      temp = flat;
  }
  format_width = image_right - image_left;
  firstline    = 1;
  DEBUG_printf(("format_width = %.1f\n", format_width));
  offset      = 0.0f;
  temp_width  = 0.0f;
  temp_height = 0.0f;
  lineptr     = NULL;
  linex       = 0.0f;
  linewidth   = 0.0f;
  while (flat != NULL)
  {
    start = flat;
    end   = flat;
    width = 0.0;
    while (flat != NULL)
    {
      temp_width = 0.0;
      temp       = flat;
      whitespace = 0;
      while (temp != NULL && !whitespace)
      {
        if (temp->markup == MARKUP_NONE && temp->data[0] == ' ')
	{
          if (temp == start)
            temp_width -= _htmlWidths[temp->typeface][temp->style][' '] *
                          _htmlSizes[temp->size] * 0.001f;
          else if (temp_width > 0.0f)
	    whitespace = 1;
	}
        else
          whitespace = 0;
        if (whitespace)
	  break;
        if (temp->markup == MARKUP_IMG)
	{
	  if ((border = htmlGetVariable(temp, (uchar *)"BORDER")) != NULL)
	    borderspace = (float)atof((char *)border);
	  else if (temp->link)
	    borderspace = 1;
	  else
	    borderspace = 0;
          borderspace *= PagePrintWidth / _htmlBrowserWidth;
          temp_width += 2 * borderspace;
	}
        prev       = temp;
        temp       = temp->next;
        temp_width += prev->width;
        if ((temp_width >= format_width && prev->markup == MARKUP_IMG) ||
	    prev->markup == MARKUP_BR)
	{
	  break;
	}
	else if (prev->markup == MARKUP_NONE)
	{
	  int	ch = prev->data[strlen((char *)prev->data) - 1];
	  if (_htmlUTF8)
	    ch = _htmlUnicode[ch];
          if (ch == 173)
            break;
	}
      }
      if ((width + temp_width) <= format_width)
      {
        width += temp_width;
        end  = temp;
        flat = temp;
        if (prev->markup == MARKUP_BR)
          break;
      }
      else if (width == 0.0)
      {
        width += temp_width;
        end  = temp;
        flat = temp;
        break;
      }
      else
        break;
    }
    if (start == end)
    {
      end   = start->next;
      flat  = start->next;
      width = start->width;
    }
    for (height = 0.0, num_chars = 0, temp = prev = start;
         temp != end;
	 temp = temp->next)
    {
      prev = temp;
      if (temp->markup == MARKUP_NONE)
        num_chars += strlen((char *)temp->data);
      if (temp->height > height)
        height = temp->height;
    }
    for (spacing = 0.0, temp = prev = start;
         temp != end;
	 temp = temp->next)
    {
      prev = temp;
      if (temp->markup != MARKUP_IMG)
        temp_height = (float)(temp->height * _htmlSpacings[0] / _htmlSizes[0]);
      else
      {
	if ((border = htmlGetVariable(temp, (uchar *)"BORDER")) != NULL)
	  borderspace = (float)atof((char *)border);
	else if (temp->link)
	  borderspace = 1;
	else
	  borderspace = 0;
        borderspace *= PagePrintWidth / _htmlBrowserWidth;
        temp_height = temp->height + 2 * borderspace;
      }
      if (temp_height > spacing)
        spacing = temp_height;
    }
    if (firstline && end != NULL && *y < (bottom + height + _htmlSpacings[t->size]))
    {
      (*page) ++;
      *y = top;
      if (Verbosity)
        progress_show("Formatting page %d", *page);
    }
    firstline = 0;
    if (height == 0.0f)
      height = spacing;
    for (temp = start; temp != end; temp = temp->next)
      if (temp->markup != MARKUP_A)
        break;
    if (temp != NULL && temp->markup == MARKUP_NONE && temp->data[0] == ' ')
    {
      for (dataptr = temp->data; *dataptr; dataptr ++)
        *dataptr = dataptr[1];
      *dataptr = '\0';
      temp_width = _htmlWidths[temp->typeface][temp->style][' '] * _htmlSizes[temp->size] * 0.001f;
      temp->width -= temp_width;
      num_chars --;
    }
    if (end != NULL)
      temp = end->prev;
    else
      temp = NULL;
    DEBUG_printf(("    BEFORE page=%d, y=%.1f, height=%.1f, spacing=%.1f, bottom=%.1f\n", *page, *y, height, spacing, bottom));
    if (*y < (spacing + bottom))
    {
      (*page) ++;
      *y = top;
      if (Verbosity)
        progress_show("Formatting page %d", *page);
    }
    *y -= height;
    DEBUG_printf(("    page=%d, y=%.1f, width=%.1f, height=%.1f\n", *page, *y, width, height));
    if (Verbosity)
      progress_update(100 - (int)(100 * (*y) / PagePrintLength));
    char_spacing = 0.0f;
    whitespace   = 0;
    temp         = start;
    linetype     = NULL;
    rgb[0] = temp->red / 255.0f;
    rgb[1] = temp->green / 255.0f;
    rgb[2] = temp->blue / 255.0f;
    switch (t->halignment)
    {
      case ALIGN_LEFT :
          linex = image_left;
	  break;
      case ALIGN_CENTER :
          linex = image_left + 0.5f * (format_width - width);
	  break;
      case ALIGN_RIGHT :
          linex = image_right - width;
	  break;
      case ALIGN_JUSTIFY :
          linex = image_left;
	  if (flat != NULL && flat->prev->markup != MARKUP_BR && num_chars > 1)
	    char_spacing = (format_width - width) / (num_chars - 1);
	  break;
    }
    while (temp != end)
    {
      if (temp->link != NULL && PSLevel == 0 && Links &&
          temp->markup == MARKUP_NONE)
      {
	temp->red   = (uchar)(link_color[0] * 255.0);
	temp->green = (uchar)(link_color[1] * 255.0);
	temp->blue  = (uchar)(link_color[2] * 255.0);
      }
      if (linetype != NULL &&
	  (temp->markup != MARKUP_NONE ||
	   temp->typeface != linetype->typeface ||
	   temp->style != linetype->style ||
	   temp->size != linetype->size ||
	   temp->superscript != linetype->superscript ||
	   temp->subscript != linetype->subscript ||
	   temp->red != linetype->red ||
	   temp->green != linetype->green ||
	   temp->blue != linetype->blue))
      {
        r = new_render(*page, RENDER_TEXT, linex - linewidth, *y,
	               linewidth, linetype->height, line);
	r->data.text.typeface = linetype->typeface;
	r->data.text.style    = linetype->style;
	r->data.text.size     = (float)_htmlSizes[linetype->size];
	r->data.text.spacing  = char_spacing;
        memcpy(r->data.text.rgb, rgb, sizeof(rgb));
	if (linetype->superscript)
          r->y += height - linetype->height;
        else if (linetype->subscript)
          r->y -= height - linetype->height;
        free(linetype);
        linetype = NULL;
      }
      if ((link = htmlGetVariable(temp, (uchar *)"ID")) != NULL)
      {
	add_link(link, *page, (int)(*y + height));
      }
      switch (temp->markup)
      {
        case MARKUP_A :
            if ((link = htmlGetVariable(temp, (uchar *)"NAME")) != NULL)
            {
              add_link(link, *page, (int)(*y + height));
            }
	default :
	    temp_width = temp->width;
            break;
        case MARKUP_NONE :
            if (temp->data == NULL)
              break;
	    if (((temp->width - right + left) > 0.001 ||
	         (temp->height - top + bottom) > 0.001)  && OverflowErrors)
	      progress_error(HD_ERROR_CONTENT_TOO_LARGE,
	                     "Text on page %d too large - "
			     "truncation or overlapping may occur!", *page + 1);
            if (linetype == NULL)
            {
	      linetype  = temp;
	      lineptr   = line;
	      linewidth = 0.0;
	      rgb[0] = temp->red / 255.0f;
	      rgb[1] = temp->green / 255.0f;
	      rgb[2] = temp->blue / 255.0f;
	    }
            strlcpy((char *)lineptr, (char *)temp->data, sizeof(line) - (size_t)(lineptr - line));
            temp_width = temp->width + char_spacing * strlen((char *)lineptr);
	    if (temp->underline || (temp->link && LinkStyle && PSLevel == 0))
	      new_render(*page, RENDER_BOX, linex, *y - 1, temp_width, 0, rgb);
	    if (temp->strikethrough)
	      new_render(*page, RENDER_BOX, linex, *y + temp->height * 0.25f,
	                 temp_width, 0, rgb);
            linewidth  += temp_width;
            lineptr    += strlen((char *)lineptr);
            if (lineptr > line && lineptr[-1] == ' ')
              whitespace = 1;
            else
              whitespace = 0;
	    break;
	case MARKUP_IMG :
	    if (((temp->width - right + left) > 0.001 ||
	         (temp->height - top + bottom) > 0.001) && OverflowErrors)
	    {
	      DEBUG_printf(("IMAGE: %.3fx%.3f > %.3fx%.3f\n",
	                    temp->width, temp->height,
			    right - left, top - bottom));
	      progress_error(HD_ERROR_CONTENT_TOO_LARGE,
	                     "Image on page %d too large - "
			     "truncation or overlapping may occur!", *page + 1);
            }
	    if ((border = htmlGetVariable(temp, (uchar *)"BORDER")) != NULL)
	      borderspace = (float)atof((char *)border);
	    else if (temp->link)
	      borderspace = 1;
	    else
	      borderspace = 0;
            borderspace *= PagePrintWidth / _htmlBrowserWidth;
            temp_width += 2 * borderspace;
	    switch (temp->valignment)
	    {
	      case ALIGN_TOP :
		  offset = height - temp->height - 2 * borderspace;
		  break;
	      case ALIGN_MIDDLE :
		  offset = 0.5f * (height - temp->height) - borderspace;
		  break;
	      case ALIGN_BOTTOM :
		  offset = 0.0f;
	    }
            if (borderspace > 0.0f)
	    {
              new_render(*page, RENDER_BOX, linex,
	                 *y + offset + temp->height + borderspace,
			 temp->width + 2 * borderspace, borderspace, rgb);
              new_render(*page, RENDER_BOX, linex, *y + offset,
                	 borderspace, temp->height + 2 * borderspace, rgb);
              new_render(*page, RENDER_BOX, linex + temp->width + borderspace,
	                 *y + offset, borderspace,
			 temp->height + 2 * borderspace, rgb);
              new_render(*page, RENDER_BOX, linex, *y + offset,
                	 temp->width + 2 * borderspace, borderspace, rgb);
	    }
	    new_render(*page, RENDER_IMAGE, linex + borderspace,
	               *y + offset + borderspace, temp->width, temp->height,
		       image_find((char *)htmlGetVariable(temp, (uchar *)"REALSRC")));
            whitespace = 0;
	    temp_width = temp->width + 2 * borderspace;
	    break;
      }
      if (temp->link != NULL &&
          (link = htmlGetVariable(temp->link, (uchar *)"_HD_FULL_HREF")) != NULL)
      {
	new_render(*page, RENDER_LINK, linex, *y + offset, temp->width, temp->height, link);
      }
      linex += temp_width;
      prev = temp;
      temp = temp->next;
      if (prev != linetype)
        free(prev);
    }
    if (linetype != NULL)
    {
      r = new_render(*page, RENDER_TEXT, linex - linewidth, *y,
                     linewidth, linetype->height, line);
      r->data.text.typeface = linetype->typeface;
      r->data.text.style    = linetype->style;
      r->data.text.spacing  = char_spacing;
      r->data.text.size     = (float)_htmlSizes[linetype->size];
      memcpy(r->data.text.rgb, rgb, sizeof(rgb));
      if (linetype->superscript)
        r->y += height - linetype->height;
      else if (linetype->subscript)
        r->y -= height - linetype->height;
      free(linetype);
    }
    *y -= spacing - height;
    DEBUG_printf(("    AFTER y=%.1f, bottom=%.1f\n", *y, bottom));
    if (*y < bottom)
    {
      (*page) ++;
      *y = top;
      if (Verbosity)
        progress_show("Formatting page %d", *page);
    }
    if (*y < image_y || *page > image_page)
    {
      image_y      = 0.0f;
      image_left   = left;
      image_right  = right;
      format_width = image_right - image_left;
    }
  }
  *x = left;
  if (*y > image_y && image_y > 0.0f && image_page == *page)
    *y = image_y;
  DEBUG_printf(("LEAVING parse_paragraph(), x = %.1f, y = %.1f, page = %d, image_y = %.1f\n", *x, *y, *page, image_y));
}