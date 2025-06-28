pspdf_prepare_outpages()
{
  int		c, i, j;	 
  int		nup;		 
  page_t	*page;		 
  outpage_t	*outpage;	 
  outpages = (outpage_t *)malloc(sizeof(outpage_t) * num_pages);
  memset(outpages, -1, sizeof(outpage_t) * num_pages);
  num_outpages = 0;
  outpage      = outpages;
  if (TitlePage)
  {
    for (i = 0, j = 0, nup = -1, page = pages;
         i < chapter_starts[1];
	 i ++, page ++)
    {
      if (nup != page->nup)
      {
        if (j)
	{
	  outpage ++;
	  num_outpages ++;
	}
	nup = page->nup;
	j   = 0;
      }
      if (!j)
	outpage->nup = nup;
      pspdf_transform_page(num_outpages, j, i);
      j ++;
      if (j >= nup)
      {
        j = 0;
	outpage ++;
	num_outpages ++;
      }
    }
    if (j)
    {
      outpage ++;
      num_outpages ++;
    }
  }
  if (OutputType == OUTPUT_BOOK && TocLevels > 0)
    c = 0;
  else
    c = 1;
  for (; c <= TocDocCount; c ++)
  {
    if (chapter_starts[c] < 0)
      continue;
    chapter_outstarts[c] = num_outpages;
    for (i = chapter_starts[c], j = 0, nup = -1, page = pages + i;
         i <= chapter_ends[c];
	 i ++, page ++)
    {
      if (nup != page->nup)
      {
        if (j)
	{
	  outpage ++;
	  num_outpages ++;
	}
	nup = page->nup;
	j   = 0;
      }
      if (!j)
	outpage->nup = nup;
      pspdf_transform_page(num_outpages, j, i);
      j ++;
      if (j >= nup)
      {
        j = 0;
	outpage ++;
	num_outpages ++;
      }
    }
    if (j)
    {
      outpage ++;
      num_outpages ++;
    }
    chapter_outends[c] = num_outpages;
  }
#ifdef DEBUG
  for (c = 0; c <= TocDocCount; c ++)
    printf("chapter_outstarts[%d] = %d, chapter_outends[%d] = %d\n",
           c, chapter_outstarts[c], c, chapter_outends[c]);
  printf("num_outpages = %d\n", (int)num_outpages);
  for (i = 0, outpage = outpages; i < (int)num_outpages; i ++, outpage ++)
  {
    printf("outpage[%d]:\tnup=%d, pages=[", i, outpage->nup);
    for (j = 0; j < outpage->nup; j ++)
      printf(" %d", outpage->pages[j]);
    puts(" ]");
    page = pages + outpage->pages[0];
    printf("\t\twidth = %d, length = %d\n", page->width, page->length);
  }
  for (c = 0; c <= TocDocCount; c ++)
    printf("chapter_starts[%d] = %d, chapter_ends[%d] = %d\n",
           c, chapter_starts[c], c, chapter_ends[c]);
  for (i = 0; i < (int)num_pages; i ++)
    printf("pages[%d]->outpage = %d\n", i, pages[i].outpage);
  for (i = 0; i < (int)num_headings; i ++)
    printf("heading_pages[%d] = %d\n", i, heading_pages[i]);
  for (i = 0; i < (int)num_links; i ++)
    printf("links[%d].name = \"%s\", page = %d\n", i,
           links[i].name, links[i].page);
#endif  
}