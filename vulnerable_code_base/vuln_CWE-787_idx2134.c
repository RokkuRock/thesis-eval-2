render_table_row(hdtable_t &table,
                 tree_t    ***cells,
                 int       row,
                 uchar     *height_var,
                 float     left,		 
                 float     right,		 
                 float     bottom,		 
                 float     top,			 
                 float     *x,
                 float     *y,
                 int       *page)
{
  int		col,
		tcol,
		colspan,
		rowspan,
		tempspace;
  float		width,
		temp_y;
  int		temp_page;
  uchar		*var;
  int		do_valign;		 
  int           row_page;
  float		row_y,
                row_starty,
                row_height,		 
		temp_height;		 
  uchar		*bgcolor;
  float		bgrgb[3];
  do_valign  = 1;
  row_height = 0.0f;
  row_page   = *page;
  row_y      = *y - table.cellpadding;
  row_starty = row_y;
  DEBUG_printf(("BEFORE row_y = %.1f, *y = %.1f, row_page = %d\n",
                row_y, *y, row_page));
  for (col = 0, rowspan = 9999; col < table.num_cols; col += colspan)
  {
    if (table.row_spans[col] == 0)
    {
      if ((var = htmlGetVariable(cells[row][col], (uchar *)"ROWSPAN")) != NULL)
        table.row_spans[col] = atoi((char *)var);
      if (table.row_spans[col] == 1)
        table.row_spans[col] = 0;
      if (table.row_spans[col] > (table.num_rows - row))
        table.row_spans[col] = table.num_rows - row;
      table.span_heights[col] = 0.0f;
    }
    if (table.row_spans[col] < rowspan)
      rowspan = table.row_spans[col];
    for (colspan = 1; (col + colspan) < table.num_cols; colspan ++)
      if (cells[row][col] != cells[row][col + colspan])
        break;
  }
  if (!rowspan)
    rowspan = 1;
  for (col = 0; col < table.num_cols;)
  {
    for (colspan = 1; (col + colspan) < table.num_cols; colspan ++)
      if (cells[row][col] != cells[row][col + colspan])
        break;
    colspan --;
    DEBUG_printf(("    col = %d, colspan = %d, left = %.1f, right = %.1f, cell = %p\n", col, colspan, table.col_lefts[col], table.col_rights[col + colspan], (void *)cells[row][col]));
    *x        = table.col_lefts[col];
    temp_y    = *y - table.cellpadding;
    temp_page = *page;
    tempspace = 0;
    if (row == 0 || cells[row][col] != cells[row - 1][col])
    {
      check_pages(*page);
      if (cells[row][col] == NULL)
        bgcolor = NULL;
      else if ((bgcolor = htmlGetVariable(cells[row][col], (uchar *)"BGCOLOR")) != NULL)
      {
        memcpy(bgrgb, background_color, sizeof(bgrgb));
        get_color(bgcolor, bgrgb, 0);
        width       = table.col_rights[col + colspan] - table.col_lefts[col] + 2 * table.cellpadding;
        table.border_left = table.col_lefts[col] - table.cellpadding;
        table.cell_bg[col] = new_render(*page, RENDER_BOX, table.border_left, row_y, width + table.border, 0.0, bgrgb);
      }
      else
      {
        table.cell_bg[col] = NULL;
        new_render(*page, RENDER_TEXT, -1.0f, -1.0f, 0.0, 0.0, (void *)"");
      }
      DEBUG_printf(("cell_bg[%d] = %p, pages[%d].end = %p\n", col, (void *)table.cell_bg[col], *page, (void *)pages[*page].end));
      table.cell_start[col] = pages[*page].end;
      table.cell_page[col]  = temp_page;
      table.cell_y[col]     = temp_y;
      if (table.debug)
      {
        check_pages(*page);
        render_t *r;
        char table_text[255];
        snprintf(table_text, sizeof(table_text), "cell=%p [%d,%d]",
                 (void *)cells[row][col], row, col);
        r = new_render(temp_page, RENDER_TEXT, *x, temp_y,
                       get_width((uchar *)table_text, TYPE_COURIER, STYLE_NORMAL, 1),
                       _htmlSizes[1], table_text);
        r->data.text.typeface = TYPE_COURIER;
        r->data.text.style    = STYLE_NORMAL;
        r->data.text.size     = (float)_htmlSizes[1];
      }
      if (cells[row][col] != NULL && cells[row][col]->child != NULL)
      {
        DEBUG_printf(("    parsing cell %d,%d; width = %.1f\n", row, col, table.col_rights[col + colspan] - table.col_lefts[col]));
        bottom += table.cellpadding;
        top    -= table.cellpadding;
        parse_doc(cells[row][col]->child, table.col_lefts + col, table.col_rights + col + colspan, &bottom, &top, x, &temp_y, &temp_page, NULL, &tempspace);
        bottom -= table.cellpadding;
        top    += table.cellpadding;
      }
      table.cell_endpage[col] = temp_page;
      table.cell_endy[col]    = temp_y;
      table.cell_height[col]  = *y - table.cellpadding - temp_y;
      table.cell_end[col]     = pages[*page].end;
      if (table.cell_start[col] == NULL)
        table.cell_start[col] = pages[*page].start;
      DEBUG_printf(("row = %d, col = %d, y = %.1f, cell_y = %.1f, cell_height = %.1f\n", row, col, *y - table.cellpadding, temp_y, table.cell_height[col]));
      DEBUG_printf(("cell_start[%d] = %p, cell_end[%d] = %p\n", col, (void *)table.cell_start[col], col, (void *)table.cell_end[col]));
    }
    if (table.row_spans[col] == 0 &&
        table.cell_page[col] == table.cell_endpage[col] &&
        table.cell_height[col] > row_height)
      row_height = table.cell_height[col];
    if (table.row_spans[col] <= rowspan)
    {
      if (table.cell_page[col] != table.cell_endpage[col])
        do_valign = 0;
      if (table.cell_endpage[col] > row_page)
      {
        row_page = table.cell_endpage[col];
        row_y    = table.cell_endy[col];
      }
      else if (table.cell_endy[col] < row_y && table.cell_endpage[col] == row_page)
        row_y = table.cell_endy[col];
    }
    DEBUG_printf(("**** col = %d, row = %d, row_y = %.1f, row_page = %d\n", col, row, row_y, row_page));
    for (col ++; colspan > 0; colspan --, col ++)
    {
      table.cell_start[col]   = NULL;
      table.cell_page[col]    = table.cell_page[col - 1];
      table.cell_y[col]       = table.cell_y[col - 1];
      table.cell_end[col]     = NULL;
      table.cell_endpage[col] = table.cell_endpage[col - 1];
      table.cell_endy[col]    = table.cell_endy[col - 1];
      table.cell_height[col]  = table.cell_height[col - 1];
    }
  }
  DEBUG_printf(("row = %d, row_y = %.1f, row_height = %.1f\n", row, row_y, row_height));
  for (col = 0; col < table.num_cols; col += colspan)
  {
    for (colspan = 1; (col + colspan) < table.num_cols; colspan ++)
      if (cells[row][col] != cells[row][col + colspan])
        break;
    if (table.row_spans[col])
      table.span_heights[col] += row_height;
    DEBUG_printf(("col = %d, cell_y = %.1f, cell_page = %d, cell_endpage = %d, row_spans = %d, span_heights = %.1f, cell_height = %.1f\n", col, table.cell_y[col], table.cell_page[col], table.cell_endpage[col], table.row_spans[col], table.span_heights[col], table.cell_height[col]));
  }
  for (col = 0; col < table.num_cols; col += colspan)
  {
    for (colspan = 1; (col + colspan) < table.num_cols; colspan ++)
      if (cells[row][col] != cells[row][col + colspan])
        break;
    if (table.row_spans[col] == rowspan &&
        table.cell_page[col] == table.cell_endpage[col] &&
        table.cell_height[col] > table.span_heights[col])
    {
      temp_height = table.cell_height[col] - table.span_heights[col];
      row_height  += temp_height;
      DEBUG_printf(("Adjusting row-span height by %.1f, new row_height = %.1f\n", temp_height, row_height));
      for (tcol = 0; tcol < table.num_cols; tcol ++)
        if (table.row_spans[tcol])
        {
          table.span_heights[tcol] += temp_height;
          DEBUG_printf(("col = %d, span_heights = %.1f\n", tcol, table.span_heights[tcol]));
        }
    }
  }
  DEBUG_printf(("AFTER row = %d, row_page = %d, row_y = %.1f, row_height = %.1f, *y = %.1f, do_valign = %d\n", row, row_page, row_y, row_height, *y, do_valign));
  if (do_valign)
  {
    height_var = NULL;
    if (cells[row][0] != NULL)
    {
      if ((height_var = htmlGetVariable(cells[row][0]->parent, (uchar *)"HEIGHT")) == NULL)
	for (col = 0; col < table.num_cols; col ++)
	  if (htmlGetVariable(cells[row][col], (uchar *)"ROWSPAN") == NULL)
	    if ((height_var = htmlGetVariable(cells[row][col], (uchar *)"HEIGHT")) != NULL)
	      break;
    }
    if (height_var != NULL)
    {
      if (height_var[strlen((char *)height_var) - 1] == '%')
        temp_height = (float)(atof((char *)height_var) * 0.01f * PagePrintLength);
      else
        temp_height = (float)(atof((char *)height_var) * PagePrintWidth / _htmlBrowserWidth);
      if (table.height > 0 && temp_height > table.height)
        temp_height = table.height;
      temp_height -= 2 * table.cellpadding;
      if (temp_height > row_height)
      {
        row_height = temp_height;
        row_y      = *y - temp_height;
      }
    }
    for (col = 0; col < table.num_cols; col += colspan + 1)
    {
      render_t	*p;
      float	delta_y;
      for (colspan = 1; (col + colspan) < table.num_cols; colspan ++)
        if (cells[row][col] != cells[row][col + colspan])
          break;
      colspan --;
      if (table.cell_start[col] == NULL || table.row_spans[col] > rowspan ||
          cells[row][col] == NULL || cells[row][col]->child == NULL)
        continue;
      if (table.row_spans[col] == 1)
      {
        int tcol;
        float span_height = 0.0f;
        for (tcol = 0; tcol < table.num_cols; tcol ++)
        {
          if (table.row_spans[col] == 1 && table.span_heights[col] > span_height)
            span_height = table.span_heights[col];
        }
        switch (cells[row][col]->valignment)
        {
          case ALIGN_MIDDLE :
              delta_y = (span_height - table.cell_height[col]) * 0.5f;
              break;
          case ALIGN_BOTTOM :
              delta_y = span_height - table.cell_height[col];
              break;
          default :
              delta_y = 0.0f;
              break;
        }
      }
      else if (table.row_spans[col])
      {
        delta_y = 0.0f;
      }
      else
      {
        switch (cells[row][col]->valignment)
        {
          case ALIGN_MIDDLE :
              delta_y = (row_height - table.cell_height[col]) * 0.5f;
              break;
          case ALIGN_BOTTOM :
              delta_y = row_height - table.cell_height[col];
              break;
          default :
              delta_y = 0.0f;
              break;
        }
      }
      DEBUG_printf(("row = %d, col = %d, valign = %d, rowspans = %d, cell_height = %.1f, span_heights = %.1f, delta_y = %.1f\n", row, col, cells[row][col]->valignment, table.row_spans[col], table.cell_height[col], table.span_heights[col], delta_y));
      if (delta_y > 0.0f)
      {
        if (table.cell_start[col] == table.cell_end[col])
          p = table.cell_start[col];
        else
          p = table.cell_start[col]->next;
        for (; p != NULL; p = p->next)
        {
          DEBUG_printf(("aligning %p (%s), y was %.1f, now %.1f\n",
                        (void *)p, p->data.text.buffer, p->y, p->y - delta_y));
          p->y -= delta_y;
          if (p == table.cell_end[col])
            break;
        }
      }
#ifdef DEBUG
      else
      {
        if (table.cell_start[col] == table.cell_end[col])
          p = table.cell_start[col];
        else
          p = table.cell_start[col]->next;
        for (; p != NULL; p = p->next)
        {
          printf("NOT aligning %p (%s)\n", (void *)p, p->data.text.buffer);
          if (p == table.cell_end[col])
            break;
        }
      }
#endif  
    }
  }
  for (col = 0, temp_page = -1, temp_y = 99999999; col < table.num_cols; col ++)
    if (table.row_spans[col] <= rowspan &&
        cells[row][col] != NULL && cells[row][col]->child != NULL)
    {
      if (table.cell_endpage[col] > temp_page)
      {
        temp_page = table.cell_endpage[col];
        temp_y    = table.cell_endy[col];
      }
      else if (table.cell_endpage[col] == temp_page && table.cell_endy[col] < temp_y)
        temp_y = table.cell_endy[col];
    }
  for (col = 0; col < table.num_cols; col ++)
    if (table.row_spans[col] <= rowspan &&
        cells[row][col] != NULL && cells[row][col]->child != NULL)
    {
      table.cell_endpage[col] = temp_page;
      table.cell_endy[col]    = temp_y;
    }
  row_y -= table.cellpadding;
  table.border_left = table.col_lefts[0] - table.cellpadding;
  width       = table.col_rights[table.num_cols - 1] - table.col_lefts[0] + 2 * table.cellpadding;
  for (bgcolor = NULL, col = 0; col < table.num_cols; col ++)
    if (table.row_spans[col] <= rowspan &&
        cells[row][col] &&
        !htmlGetVariable(cells[row][col], (uchar *)"ROWSPAN") &&
        (bgcolor = htmlGetVariable(cells[row][col]->parent,
                                   (uchar *)"BGCOLOR")) != NULL)
      break;
  if (bgcolor)
  {
    memcpy(bgrgb, background_color, sizeof(bgrgb));
    get_color(bgcolor, bgrgb, 0);
    if (row_page > *page)
    {
      new_render(*page, RENDER_BOX, table.border_left, bottom,
                 width, row_starty - bottom + table.cellpadding, bgrgb,
                 pages[*page].start);
      for (temp_page = *page + 1; temp_page < row_page; temp_page ++)
      {
        new_render(temp_page, RENDER_BOX, table.border_left, bottom,
                   width, top - bottom, bgrgb, pages[temp_page].start);
      }
      check_pages(*page);
      new_render(row_page, RENDER_BOX, table.border_left, row_y,
                 width, top - row_y, bgrgb,
                 pages[row_page].start);
    }
    else
    {
      new_render(row_page, RENDER_BOX, table.border_left, row_y,
                 width, row_height + 2 * table.cellpadding, bgrgb,
                 pages[row_page].start);
    }
  }
  for (col = 0; col < table.num_cols; col += colspan + 1)
  {
    for (colspan = 0; (col + colspan) < table.num_cols; colspan ++)
      if (cells[row][col] != cells[row][col + colspan])
        break;
      else if (table.row_spans[col + colspan] > 0)
      {
        DEBUG_printf(("row = %d, col = %d, decrementing row_spans (%d) to %d...\n", row,
                      col, table.row_spans[col + colspan],
                      table.row_spans[col + colspan] - rowspan));
        table.row_spans[col + colspan] -= rowspan;
      }
    colspan --;
    width = table.col_rights[col + colspan] - table.col_lefts[col] +
            2 * table.cellpadding;
    if (cells[row][col] == NULL || cells[row][col]->child == NULL ||
        table.row_spans[col] > 0)
      continue;
    DEBUG_printf(("DRAWING BORDER+BACKGROUND: col=%d, row=%d, cell_page=%d, cell_y=%.1f\n"
                  "                           cell_endpage=%d, cell_endy=%.1f\n",
                  col, row, table.cell_page[col], table.cell_y[col],
                  table.cell_endpage[col], table.cell_endy[col]));
    if ((bgcolor = htmlGetVariable(cells[row][col],
                                   (uchar *)"BGCOLOR")) != NULL)
    {
      memcpy(bgrgb, background_color, sizeof(bgrgb));
      get_color(bgcolor, bgrgb, 0);
    }
    table.border_left = table.col_lefts[col] - table.cellpadding;
    if (table.cell_page[col] != table.cell_endpage[col])
    {
      if (table.border > 0)
      {
        new_render(table.cell_page[col], RENDER_BOX, table.border_left,
                   table.cell_y[col] + table.cellpadding,
                   width + table.border, table.border, table.border_rgb);
        new_render(table.cell_page[col], RENDER_BOX, table.border_left, bottom,
                   table.border, table.cell_y[col] - bottom + table.cellpadding + table.border, table.border_rgb);
        new_render(table.cell_page[col], RENDER_BOX,
                   table.border_left + width, bottom,
                   table.border, table.cell_y[col] - bottom + table.cellpadding + table.border, table.border_rgb);
      }
      if (bgcolor != NULL)
      {
        table.cell_bg[col]->y      = bottom;
        table.cell_bg[col]->height = table.cell_y[col] - bottom + table.cellpadding + table.border;
      }
      for (temp_page = table.cell_page[col] + 1; temp_page < table.cell_endpage[col]; temp_page ++)
      {
        if (table.border > 0.0f)
        {
          new_render(temp_page, RENDER_BOX, table.border_left, bottom,
                     table.border, top - bottom, table.border_rgb);
          new_render(temp_page, RENDER_BOX,
                     table.border_left + width, bottom,
                     table.border, top - bottom, table.border_rgb);
        }
        if (bgcolor != NULL)
          new_render(temp_page, RENDER_BOX, table.border_left, bottom,
                     width + table.border, top - bottom, bgrgb,
                     pages[temp_page].start);
      }
      if (table.border > 0.0f)
      {
        new_render(table.cell_endpage[col], RENDER_BOX, table.border_left, row_y,
                   table.border, top - row_y, table.border_rgb);
        new_render(table.cell_endpage[col], RENDER_BOX,
                   table.border_left + width, row_y,
                   table.border, top - row_y, table.border_rgb);
        new_render(table.cell_endpage[col], RENDER_BOX, table.border_left, row_y,
                   width + table.border, table.border, table.border_rgb);
      }
      if (bgcolor != NULL)
      {
        check_pages(table.cell_endpage[col]);
        new_render(table.cell_endpage[col], RENDER_BOX, table.border_left, row_y,
                   width + table.border, top - row_y, bgrgb,
                   pages[table.cell_endpage[col]].start);
      }
    }
    else
    {
      if (table.border > 0.0f)
      {
        new_render(table.cell_page[col], RENDER_BOX, table.border_left,
                   table.cell_y[col] + table.cellpadding,
                   width + table.border, table.border, table.border_rgb);
        new_render(table.cell_page[col], RENDER_BOX, table.border_left, row_y,
                   table.border, table.cell_y[col] - row_y + table.cellpadding + table.border, table.border_rgb);
        new_render(table.cell_page[col], RENDER_BOX,
                   table.border_left + width, row_y,
                   table.border, table.cell_y[col] - row_y + table.cellpadding + table.border, table.border_rgb);
        new_render(table.cell_page[col], RENDER_BOX, table.border_left, row_y,
                   width + table.border, table.border, table.border_rgb);
      }
      if (bgcolor != NULL)
      {
        table.cell_bg[col]->y      = row_y;
        table.cell_bg[col]->height = table.cell_y[col] - row_y + table.cellpadding + table.border;
      }
    }
  }
  *page = row_page;
  *y    = row_y;
}