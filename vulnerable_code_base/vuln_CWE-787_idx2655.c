parse_table(tree_t *t,			 
            float  left,		 
            float  right,		 
            float  bottom,		 
            float  top,			 
            float  *x,			 
            float  *y,			 
            int    *page,		 
            int    needspace)		 
{
  int		col,
		row,
                header_row = -1,
		tcol,
		colspan,
		rowspan,
		alloc_rows,
		regular_cols;
  hdtable_t     table;
  float		col_width,
		col_min,
		col_pref,
		col_height,
		cellspacing,
		width,
		pref_width,
		span_width,
		regular_width,
		actual_width,
		table_width,
		min_width,
		temp_width,
                header_height = 0.0,
		table_y,
                temp_bottom,
		temp_top;
  int		temp_page, table_page;
  uchar		*var,
		*height_var,		 
                *header_height_var = NULL;
  tree_t	*temprow,
		*tempcol,
		*tempnext,
		***cells,
		*caption;		 
  float		temp_height;		 
  uchar		*bgcolor;
  float		bgrgb[3];
  const char	*htmldoc_debug;		 
  DEBUG_puts("\n\nTABLE");
  DEBUG_printf(("parse_table(t=%p, left=%.1f, right=%.1f, x=%.1f, y=%.1f, page=%d\n",
                (void *)t, left, right, *x, *y, *page));
  if (t->child == NULL)
    return;    
  memset(&table, 0, sizeof(table));
  if ((htmldoc_debug = getenv("HTMLDOC_DEBUG")) != NULL &&
      (strstr(htmldoc_debug, "table") || strstr(htmldoc_debug, "all")))
    table.debug = 1;
  else
    table.debug = 0;
  cells = NULL;
  if ((var = htmlGetVariable(t, (uchar *)"WIDTH")) != NULL)
  {
    if (var[strlen((char *)var) - 1] == '%')
      table_width = (float)(atof((char *)var) * (right - left) / 100.0f);
    else
      table_width = (float)(atoi((char *)var) * PagePrintWidth / _htmlBrowserWidth);
    if (table_width < 0.0f || table_width > PagePrintWidth)
      table_width = right - left;
  }
  else
    table_width = right - left;
  if ((var = htmlGetVariable(t, (uchar *)"HEIGHT")) != NULL)
  {
    if (var[strlen((char *)var) - 1] == '%')
      table.height = (float)(atof((char *)var) * (top - bottom) / 100.0f);
    else
      table.height = (float)(atoi((char *)var) * PagePrintWidth / _htmlBrowserWidth);
  }
  else
    table.height = -1.0f;
  DEBUG_printf(("table_width = %.1f\n", table_width));
  if ((var = htmlGetVariable(t, (uchar *)"CELLPADDING")) != NULL)
  {
    if ((table.cellpadding = atoi((char *)var)) < 0.0f)
      table.cellpadding = 0.0f;
    else if (table.cellpadding > 20.0f)
      table.cellpadding = 20.0f;
  }
  else
    table.cellpadding = 1.0f;
  if ((var = htmlGetVariable(t, (uchar *)"CELLSPACING")) != NULL)
  {
    if ((cellspacing = atoi((char *)var)) < 0.0f)
      cellspacing = 0.0f;
    else if (cellspacing > 20.0f)
      cellspacing = 20.0f;
  }
  else
    cellspacing = 0.0f;
  if ((var = htmlGetVariable(t, (uchar *)"BORDER")) != NULL)
  {
    if ((table.border = (float)atof((char *)var)) <= 0.0 && var[0] != '0')
      table.border = 1.0f;
    else if (table.border > 20.0f)
      table.border = 20.0f;
    table.cellpadding += table.border;
  }
  else
    table.border = 0.0f;
  if (table.debug && table.border == 0.0f)
    table.border = 0.01f;
  table.border_rgb[0] = t->red / 255.0f;
  table.border_rgb[1] = t->green / 255.0f;
  table.border_rgb[2] = t->blue / 255.0f;
  if ((var = htmlGetVariable(t, (uchar *)"BORDERCOLOR")) != NULL)
    get_color(var, table.border_rgb, 0);
  if (table.border == 0.0f && table.cellpadding > 0.0f)
  {
    table.cellpadding += 1.0f;
  }
  table.border_size = table.border - 1.0f;
  cellspacing       *= PagePrintWidth / _htmlBrowserWidth;
  table.cellpadding *= PagePrintWidth / _htmlBrowserWidth;
  table.border      *= PagePrintWidth / _htmlBrowserWidth;
  table.border_size *= PagePrintWidth / _htmlBrowserWidth;
  DEBUG_printf(("border = %.1f, cellpadding = %.1f\n", table.border, table.cellpadding));
  temp_bottom = bottom - table.cellpadding;
  temp_top    = top + table.cellpadding;
  for (temprow = t->child, table.num_cols = 0, table.num_rows = 0, alloc_rows = 0, caption = NULL;
       temprow != NULL;
       temprow = tempnext)
  {
    tempnext = temprow->next;
    if (temprow->markup == MARKUP_CAPTION)
    {
      if ((var = htmlGetVariable(temprow, (uchar *)"ALIGN")) == NULL ||
          strcasecmp((char *)var, "bottom"))
      {
        parse_paragraph(temprow, left, right, bottom, top, x, y, page, needspace);
        needspace = 1;
      }
      else
      {
        caption = temprow;
      }
    }
    else if (temprow->markup == MARKUP_TR ||
             ((temprow->markup == MARKUP_TBODY || temprow->markup == MARKUP_THEAD ||
               temprow->markup == MARKUP_TFOOT) && temprow->child != NULL))
    {
      if (temprow->markup == MARKUP_THEAD)
        header_row = table.num_rows;
      if (temprow->markup == MARKUP_TBODY || temprow->markup == MARKUP_THEAD ||
          temprow->markup == MARKUP_TFOOT)
        temprow = temprow->child;
      if ((tempnext = temprow->next) == NULL)
        if (temprow->parent->markup == MARKUP_TBODY ||
            temprow->parent->markup == MARKUP_THEAD ||
            temprow->parent->markup == MARKUP_TFOOT)
          tempnext = temprow->parent->next;
      if (table.num_rows >= alloc_rows)
      {
        alloc_rows += ALLOC_ROWS;
        if (alloc_rows == ALLOC_ROWS)
	  cells = (tree_t ***)malloc(sizeof(tree_t **) * (size_t)alloc_rows);
	else
	  cells = (tree_t ***)realloc(cells, sizeof(tree_t **) * (size_t)alloc_rows);
        if (cells == (tree_t ***)0)
	{
	  progress_error(HD_ERROR_OUT_OF_MEMORY,
                         "Unable to allocate memory for table!");
	  return;
	}
      }
      if ((cells[table.num_rows] = (tree_t **)calloc(sizeof(tree_t *), MAX_COLUMNS)) == NULL)
      {
	progress_error(HD_ERROR_OUT_OF_MEMORY,
                       "Unable to allocate memory for table!");
        free(cells);
	return;
      }
#ifdef DEBUG
      printf("BEFORE row %d: num_cols = %d\n", table.num_rows, table.num_cols);
      if (table.num_rows)
        for (col = 0; col < table.num_cols; col ++)
	  printf("    col %d: row_spans[] = %d\n", col, table.row_spans[col]);
#endif  
      if (table.num_rows)
      {
	for (col = 0, rowspan = 9999; col < table.num_cols; col ++)
	  if (table.row_spans[col] < rowspan)
	    rowspan = table.row_spans[col];
	for (col = 0; col < table.num_cols; col ++)
	  table.row_spans[col] -= rowspan;
	for (col = 0; table.row_spans[col] && col < table.num_cols; col ++)
          cells[table.num_rows][col] = cells[table.num_rows - 1][col];
      }
      else
        col = 0;
      for (tempcol = temprow->child;
           tempcol != NULL && col < MAX_COLUMNS;
           tempcol = tempcol->next)
      {
        if (tempcol->markup == MARKUP_TH && table.num_rows == 0)
          header_row = table.num_rows;
        if (tempcol->markup == MARKUP_TD || tempcol->markup == MARKUP_TH)
        {
          if ((var = htmlGetVariable(tempcol, (uchar *)"COLSPAN")) != NULL)
            colspan = atoi((char *)var);
          else
            colspan = 1;
          if ((var = htmlGetVariable(tempcol, (uchar *)"ROWSPAN")) != NULL)
	  {
            table.row_spans[col] = atoi((char *)var);
	    if (table.row_spans[col] == 1)
	      table.row_spans[col] = 0;
	    for (tcol = 1; tcol < colspan; tcol ++)
              table.row_spans[col + tcol] = table.row_spans[col];
          }
          col_width = get_cell_size(tempcol, 0.0f, table_width, &col_min, &col_pref, &col_height);
          if ((var = htmlGetVariable(tempcol, (uchar *)"WIDTH")) != NULL)
	  {
	    if (var[strlen((char *)var) - 1] == '%')
	    {
              col_width -= 2.0 * table.cellpadding - cellspacing;
	      if (colspan <= 1)
	        table.col_percent[col] = 1;
	    }
	    else
	    {
              col_width -= 2.0 * table.cellpadding;
	    }
	  }
	  else
	    col_width = 0.0f;
          tempcol->height = col_height;
	  DEBUG_printf(("%d,%d: colsp=%d, rowsp=%d, width=%.1f, minw=%.1f, prefw=%.1f, minh=%.1f\n", col, table.num_rows, colspan, table.row_spans[col], col_width, col_min, col_pref, col_height));
          if (colspan > 1)
          {
	    if (colspan > table.col_spans[col])
	      table.col_spans[col] = colspan;
	    if (col_width > table.col_swidths[col])
	      table.col_swidths[col] = col_width;
	    if (col_min > table.col_smins[col])
	      table.col_smins[col] = col_min;
	    temp_width = col_width / colspan;
	    for (int i = 0; i < colspan; i ++)
	    {
	      if (temp_width > table.col_widths[col + i])
	        table.col_widths[col + i] = temp_width;
	    }
          }
	  else
	  {
	    if (col_width > 0.0f)
	      table.col_fixed[col] = 1;
	    if (col_width > table.col_widths[col])
	      table.col_widths[col] = col_width;
	    if (col_pref > table.col_prefs[col])
	      table.col_prefs[col] = col_pref;
	    if (col_min > table.col_mins[col])
	      table.col_mins[col] = col_min;
          }
	  while (colspan > 0 && col < MAX_COLUMNS)
	  {
            cells[table.num_rows][col] = tempcol;
            col ++;
            colspan --;
          }
          while (table.row_spans[col] && col < table.num_cols)
	  {
            cells[table.num_rows][col] = cells[table.num_rows - 1][col];
	    col ++;
	  }
        }
      }
      DEBUG_printf(("header_row=%d\n", header_row));
      if (col > table.num_cols)
        table.num_cols = col;
#ifdef DEBUG
      printf("AFTER row %d: num_cols = %d\n", table.num_rows, table.num_cols);
      for (col = 0; col < table.num_cols; col ++)
        printf("    col %d: row_spans[] = %d\n", col, table.row_spans[col]);
#endif  
      table.num_rows ++;
      for (col = 0; col < table.num_cols; col ++)
        if (table.row_spans[col])
	  table.row_spans[col] --;
    }
  }
  if (table.num_cols == 0)
    return;
  if ((var = htmlGetVariable(t, (uchar *)"WIDTH")) != NULL)
  {
    if (var[strlen((char *)var) - 1] == '%')
      width = (float)(atof((char *)var) * (right - left) / 100.0f);
    else
      width = (float)(atoi((char *)var) * PagePrintWidth / _htmlBrowserWidth);
  }
  else
  {
    for (col = 0, width = 0.0; col < table.num_cols; col ++)
      width += table.col_prefs[col];
    width += (2 * table.cellpadding + cellspacing) * table.num_cols - cellspacing;
    if (width > (right - left))
      width = right - left;
  }
  DEBUG_printf(("\nTABLE: %dx%d\n\n", table.num_cols, table.num_rows));
  actual_width  = (2 * table.cellpadding + cellspacing) * table.num_cols -
                  cellspacing;
  regular_width = (width - actual_width) / table.num_cols;
  DEBUG_printf(("    width = %.1f, actual_width = %.1f, regular_width = %.1f\n\n",
                width, actual_width, regular_width));
  DEBUG_puts("    Col  Width   Min     Pref    Fixed?  Percent?");
  DEBUG_puts("    ---  ------  ------  ------  ------  --------");
#ifdef DEBUG
  for (col = 0; col < table.num_cols; col ++)
    printf("    %-3d  %-6.1f  %-6.1f  %-6.1f  %-6s  %s\n", col, table.col_widths[col], table.col_mins[col], table.col_prefs[col], table.col_fixed[col] ? "YES" : "NO", table.col_percent[col] ? "YES" : "NO");
  puts("");
#endif  
  DEBUG_puts("PASS 1: fixed width handling\n");
  for (col = 0, regular_cols = 0; col < table.num_cols; col ++)
    if (table.col_widths[col] > 0.0f)
    {
      if (table.col_mins[col] > table.col_widths[col])
      {
        DEBUG_printf(("    updating column %d to width=%.1f\n", col, table.col_mins[col]));
        table.col_widths[col] = table.col_mins[col];
      }
      actual_width += table.col_widths[col];
    }
    else
    {
      regular_cols ++;
      actual_width += table.col_mins[col];
    }
  DEBUG_printf(("    actual_width = %.1f, regular_cols = %d\n\n", actual_width,regular_cols));
  DEBUG_puts("PASS 2: preferred width handling\n");
  for (col = 0, pref_width = 0.0f; col < table.num_cols; col ++)
    if (table.col_widths[col] == 0.0f)
      pref_width += table.col_prefs[col] - table.col_mins[col];
  DEBUG_printf(("    pref_width = %.1f\n", pref_width));
  if (pref_width > 0.0f)
  {
    if ((regular_width = (width - actual_width) / pref_width) < 0.0f)
      regular_width = 0.0f;
    else if (regular_width > 1.0f)
      regular_width = 1.0f;
    DEBUG_printf(("    regular_width = %.1f\n", regular_width));
    for (col = 0; col < table.num_cols; col ++)
      if (table.col_widths[col] == 0.0f)
      {
	pref_width = (table.col_prefs[col] - table.col_mins[col]) * regular_width;
	if ((actual_width + pref_width) > width)
	{
          if (col == (table.num_cols - 1) && (width - actual_width) >= table.col_mins[col])
	    table.col_widths[col] = width - actual_width;
	  else
	    table.col_widths[col] = table.col_mins[col];
	}
	else
          table.col_widths[col] = pref_width + table.col_mins[col];
        DEBUG_printf(("    col_widths[%d] = %.1f\n", col, table.col_widths[col]));
	actual_width += table.col_widths[col] - table.col_mins[col];
      }
  }
  else
  {
    for (col = 0; col < table.num_cols; col ++)
      if (table.col_widths[col] == 0.0f)
        table.col_widths[col] = table.col_mins[col];
  }
  DEBUG_printf(("    actual_width = %.1f\n\n", actual_width));
  DEBUG_puts("PASS 3: colspan handling\n\n");
  for (col = 0; col < table.num_cols; col ++)
  {
    DEBUG_printf(("    col %d, colspan %d\n", col, table.col_spans[col]));
    if (table.col_spans[col] > 1)
    {
      for (colspan = 0, span_width = 0.0f; colspan < table.col_spans[col]; colspan ++)
        span_width += table.col_widths[col + colspan];
      pref_width = 0.0f;
      if (span_width < table.col_swidths[col])
        pref_width = table.col_swidths[col];
      if (span_width < table.col_smins[col] && pref_width < table.col_smins[col])
        pref_width = table.col_smins[col];
      for (colspan = 0; colspan < table.col_spans[col]; colspan ++)
        if (table.col_fixed[col + colspan])
	{
          span_width -= table.col_widths[col + colspan];
	  pref_width -= table.col_widths[col + colspan];
	}
      DEBUG_printf(("    col_swidths=%.1f, col_smins=%.1f, span_width=%.1f, pref_width=%.1f\n", table.col_swidths[col], table.col_smins[col], span_width, pref_width));
      if (pref_width > 0.0f && pref_width > span_width)
      {
        if (span_width >= 1.0f)
	{
	  regular_width = pref_width / span_width;
	  for (colspan = 0; colspan < table.col_spans[col]; colspan ++)
	    if (!table.col_fixed[col + colspan])
	    {
	      actual_width -= table.col_widths[col + colspan];
	      table.col_widths[col + colspan] *= regular_width;
	      actual_width += table.col_widths[col + colspan];
              DEBUG_printf(("    col_widths[%d] = %.1f\n", col + colspan, table.col_widths[col + colspan]));
	    }
        }
	else
	{
	  regular_width = pref_width / table.col_spans[col];
	  for (colspan = 0; colspan < table.col_spans[col]; colspan ++)
	  {
	    actual_width += regular_width;
	    table.col_widths[col + colspan] += regular_width;
            DEBUG_printf(("    col_widths[%d] = %.1f\n", col, table.col_widths[col]));
	  }
	}
      }
    }
  }
  DEBUG_printf(("    actual_width = %.1f\n\n", actual_width));
  DEBUG_puts("PASS 4: divide remaining space, if any...\n");
  if (width > actual_width)
  {
    for (col = 0, colspan = 0; col < table.num_cols; col ++)
      if (!table.col_fixed[col] || table.col_percent[col])
        colspan ++;
    if (colspan > 0)
    {
      regular_width = (width - actual_width) / table.num_cols;
      for (col = 0; col < table.num_cols; col ++)
        if (!table.col_fixed[col])
	{
	  table.col_widths[col] += regular_width;
	  DEBUG_printf(("    col_widths[%d] = %.1f\n", col, table.col_widths[col]));
	}
    }
  }
  else
    width = actual_width;
  DEBUG_puts("");
  DEBUG_puts("PASS 5: Squeeze table as needed...");
  if (width > table_width)
  {
    for (col = 0, min_width = -cellspacing; col < table.num_cols; col ++)
      min_width += table.col_mins[col] + 2 * table.cellpadding + cellspacing;
    DEBUG_printf(("    table_width = %.1f, width = %.1f, min_width = %.1f\n", table_width, width, min_width));
    temp_width = table_width - min_width;
    if (temp_width < 0.0f)
      temp_width = 0.0f;
    width -= min_width;
    if (width < 1.0f)
      width = 1.0f;
    for (col = 0; col < table.num_cols; col ++)
    {
      table.col_widths[col] = table.col_mins[col] + temp_width * (table.col_widths[col] - table.col_mins[col]) / width;
      DEBUG_printf(("    col_widths[%d] = %.1f\n", col, table.col_widths[col]));
    }
    for (col = 0, width = -cellspacing; col < table.num_cols; col ++)
      width += table.col_widths[col] + 2 * table.cellpadding + cellspacing;
    DEBUG_printf(("    new width = %.1f, max width = %.1f\n", width, right - left));
  }
  if ((width - right + left) > 0.001f && OverflowErrors)
    progress_error(HD_ERROR_CONTENT_TOO_LARGE, "Table on page %d too wide - truncation or overlapping may occur!", *page + 1);
  DEBUG_puts("");
  DEBUG_printf(("Final table width = %.1f, alignment = %d\n", width, t->halignment));
  switch (t->halignment)
  {
    case ALIGN_LEFT :
        *x = left + table.cellpadding;
        break;
    case ALIGN_CENTER :
        *x = left + 0.5f * (right - left - width) + table.cellpadding;
        break;
    case ALIGN_RIGHT :
        *x = right - width + table.cellpadding;
        break;
  }
  for (col = 0; col < table.num_cols; col ++)
  {
    table.col_lefts[col]  = *x;
    table.col_rights[col] = *x + table.col_widths[col];
    *x = table.col_rights[col] + 2 * table.cellpadding + cellspacing;
    DEBUG_printf(("left[%d] = %.1f, right[%d] = %.1f\n", col, table.col_lefts[col], col, table.col_rights[col]));
  }
  if (*y < top && needspace)
    *y -= _htmlSpacings[SIZE_P];
  if (table.debug)
  {
    check_pages(*page);
    render_t *r;
    char table_text[255];
    snprintf(table_text, sizeof(table_text), "t=%p", (void *)t);
    r = new_render(*page, RENDER_TEXT, left, *y,
                   get_width((uchar *)table_text, TYPE_COURIER, STYLE_NORMAL, 3),
		   _htmlSizes[3], table_text);
    r->data.text.typeface = TYPE_COURIER;
    r->data.text.style    = STYLE_NORMAL;
    r->data.text.size     = (float)_htmlSizes[3];
  }
  table_page = *page;
  table_y    = *y;
  for (row = 0; row < table.num_rows; row ++)
  {
    height_var = NULL;
    if (cells[row][0] != NULL)
    {
      if (cells[row][0]->parent->prev != NULL &&
          cells[row][0]->parent->prev->markup == MARKUP_COMMENT)
        parse_comment(cells[row][0]->parent->prev, &left, &right, &temp_bottom, &temp_top, x, y, page, NULL, 0);
      if ((height_var = htmlGetVariable(cells[row][0]->parent, (uchar *)"HEIGHT")) == NULL)
	for (col = 0; col < table.num_cols; col ++)
	  if (htmlGetVariable(cells[row][col], (uchar *)"ROWSPAN") == NULL)
	    if ((height_var = htmlGetVariable(cells[row][col], (uchar *)"HEIGHT")) != NULL)
	      break;
    }
    if (height_var != NULL && row == header_row)
      header_height_var = height_var;
    if (cells[row][0] != NULL && height_var != NULL)
    {
      if (height_var[strlen((char *)height_var) - 1] == '%')
	temp_height = (float)(atof((char *)height_var) * 0.01f * (PagePrintLength - 2 * table.cellpadding));
      else
        temp_height = (float)(atof((char *)height_var) * PagePrintWidth / _htmlBrowserWidth);
      if (table.height > 0.0f && temp_height > table.height)
        temp_height = table.height;
      temp_height -= 2 * table.cellpadding;
    }
    else
    {
      for (col = 0, temp_height = (float)_htmlSpacings[SIZE_P];
           col < table.num_cols;
	   col ++)
        if (cells[row][col] != NULL &&
	    cells[row][col]->height > temp_height &&
	    !htmlGetVariable(cells[row][col], (uchar *)"ROWSPAN"))
	  temp_height = cells[row][col]->height;
      if (table.height > 0.0)
      {
	if (temp_height > table.height)
          temp_height = table.height;
	temp_height -= 2 * table.cellpadding;
      }
      else if (temp_height > (PageLength / 8.0) && height_var == NULL)
	temp_height = PageLength / 8.0;
    }
    DEBUG_printf(("BEFORE row = %d, temp_height = %.1f, *y = %.1f, *page = %d\n",
                  row, temp_height, *y, *page));
    if (*y < (bottom + 2 * table.cellpadding + temp_height) &&
        temp_height <= (top - bottom - 2 * table.cellpadding))
    {
      DEBUG_puts("NEW PAGE");
      *y = top - header_height;
      (*page) ++;
      if (Verbosity)
        progress_show("Formatting page %d", *page);
      if (row > 0 && header_row >= 0)
      {
        render_table_row(table, cells, header_row, header_height_var, left, right, bottom, top, x, y, page);
      }
    }
    float start_y = *y;
    temp_page = *page;
    render_table_row(table, cells, row, height_var, left, right, bottom, top, x, y, page);
    if (header_row >= 0 && row == header_row)
    {
      header_height = *y - start_y;
      top += header_height;
    }
    else if (temp_page != *page && header_row >= 0)
    {
      do
      {
        float temp_y = top - header_height;
        temp_page ++;
        render_table_row(table, cells, header_row, header_height_var, left, right, bottom, top, x, &temp_y, &temp_page);
      }
      while (temp_page < *page);
    }
    if (row < (table.num_rows - 1))
      (*y) -= cellspacing;
    DEBUG_printf(("END row = %d, *y = %.1f, *page = %d\n", row, *y, *page));
  }
  top -= header_height;
  if ((bgcolor = htmlGetVariable(t, (uchar *)"BGCOLOR")) != NULL)
  {
    memcpy(bgrgb, background_color, sizeof(bgrgb));
    get_color(bgcolor, bgrgb, 0);
    table.border_left = table.col_lefts[0] - table.cellpadding;
    width       = table.col_rights[table.num_cols - 1] - table.col_lefts[0] + 2 * table.cellpadding;
    if (table_page != *page)
    {
      new_render(table_page, RENDER_BOX, table.border_left, bottom,
	         width, table_y - bottom, bgrgb,
		 pages[table_page].start);
      for (temp_page = table_page + 1; temp_page < *page; temp_page ++)
      {
        new_render(temp_page, RENDER_BOX, table.border_left, bottom,
                   width, top - bottom, bgrgb, pages[temp_page].start);
      }
      check_pages(*page);
      new_render(*page, RENDER_BOX, table.border_left, *y,
	         width, top - *y, bgrgb, pages[*page].start);
    }
    else
    {
      new_render(table_page, RENDER_BOX, table.border_left, *y,
	         width, table_y - *y, bgrgb, pages[table_page].start);
    }
  }
  *x = left;
  if (caption)
  {
    parse_paragraph(caption, left, right, bottom, top, x, y, page, needspace);
    needspace = 1;
  }
  if (table.num_rows > 0)
  {
    for (row = 0; row < table.num_rows; row ++)
      free(cells[row]);
    free(cells);
  }
}