parse_tree(tree_t *t)		 
{
  tree_t	*parent;	 
  tree_t	*target,	 
		*temp;		 
  uchar		heading[255],	 
		link[255],	 
		baselink[255],	 
		*existing;	 
  int		i, level;	 
  uchar		*var;		 
  static const char *ones[10] =
		{
		  "",	"i",	"ii",	"iii",	"iv",
		  "v",	"vi",	"vii",	"viii",	"ix"
		},
		*tens[10] =
		{
		  "",	"x",	"xx",	"xxx",	"xl",
		  "l",	"lx",	"lxx",	"lxxx",	"xc"
		},
		*hundreds[10] =
		{
		  "",	"c",	"cc",	"ccc",	"cd",
		  "d",	"dc",	"dcc",	"dccc",	"cm"
		},
		*ONES[10] =
		{
		  "",	"I",	"II",	"III",	"IV",
		  "V",	"VI",	"VII",	"VIII",	"IX"
		},
		*TENS[10] =
		{
		  "",	"X",	"XX",	"XXX",	"XL",
		  "L",	"LX",	"LXX",	"LXXX",	"XC"
		},
		*HUNDREDS[10] =
		{
		  "",	"C",	"CC",	"CCC",	"CD",
		  "D",	"DC",	"DCC",	"DCCC",	"CM"
		};
  while (t != NULL)
  {
    switch (t->markup)
    {
      case MARKUP_H1 :
      case MARKUP_H2 :
      case MARKUP_H3 :
      case MARKUP_H4 :
      case MARKUP_H5 :
      case MARKUP_H6 :
      case MARKUP_H7 :
      case MARKUP_H8 :
      case MARKUP_H9 :
      case MARKUP_H10 :
      case MARKUP_H11 :
      case MARKUP_H12 :
      case MARKUP_H13 :
      case MARKUP_H14 :
      case MARKUP_H15 :
          level = t->markup - MARKUP_H1;
	  if ((level - last_level) > 1)
	  {
	    level     = last_level + 1;
	    t->markup = (markup_t)(MARKUP_H1 + level);
	  }
          if ((var = htmlGetVariable(t, (uchar *)"VALUE")) != NULL)
            heading_numbers[level] = atoi((char *)var);
          else
            heading_numbers[level] ++;
          if (level == 0)
            TocDocCount ++;
          if ((var = htmlGetVariable(t, (uchar *)"TYPE")) != NULL)
            heading_types[level] = var[0];
          for (i = level + 1; i < 15; i ++)
            heading_numbers[i] = 0;
          heading[0]  = '\0';
	  baselink[0] = '\0';
          for (i = 0; i <= level; i ++)
          {
            uchar	*baseptr = baselink + strlen((char *)baselink);
            uchar	*headptr = heading + strlen((char *)heading);
            if (i == 0)
              snprintf((char *)baseptr, sizeof(baselink) - (size_t)(baseptr - baselink), "%d", TocDocCount);
            else
              snprintf((char *)baseptr, sizeof(baselink) - (size_t)(baseptr - baselink), "%d", heading_numbers[i]);
            switch (heading_types[i])
            {
              case '1' :
                  snprintf((char *)headptr, sizeof(heading) - (size_t)(headptr - heading), "%d", heading_numbers[i]);
                  break;
              case 'a' :
                  if (heading_numbers[i] > 26)
                    snprintf((char *)headptr, sizeof(heading) - (size_t)(headptr - heading), "%c%c", 'a' + (heading_numbers[i] / 26) - 1, 'a' + (heading_numbers[i] % 26) - 1);
                  else
                    snprintf((char *)headptr, sizeof(heading) - (size_t)(headptr - heading), "%c", 'a' + heading_numbers[i] - 1);
                  break;
              case 'A' :
                  if (heading_numbers[i] > 26)
                    snprintf((char *)headptr, sizeof(heading) - (size_t)(headptr - heading), "%c%c", 'A' + (heading_numbers[i] / 26) - 1, 'A' + (heading_numbers[i] % 26) - 1);
                  else
                    snprintf((char *)headptr, sizeof(heading) - (size_t)(headptr - heading), "%c", 'A' + heading_numbers[i] - 1);
                  break;
              case 'i' :
                  snprintf((char *)headptr, sizeof(heading) - (size_t)(headptr - heading), "%s%s%s", hundreds[heading_numbers[i] / 100], tens[(heading_numbers[i] / 10) % 10], ones[heading_numbers[i] % 10]);
                  break;
              case 'I' :
                  snprintf((char *)headptr, sizeof(heading) - (size_t)(headptr - heading), "%s%s%s", HUNDREDS[heading_numbers[i] / 100], TENS[(heading_numbers[i] / 10) % 10], ONES[heading_numbers[i] % 10]);
                  break;
            }
            if (i < level)
            {
              strlcat((char *)heading, ".", sizeof(heading));
              strlcat((char *)baselink, "_", sizeof(baselink));
            }
          }
          existing = NULL;
          if (t->parent != NULL && t->parent->markup == MARKUP_A)
          {
	    existing = htmlGetVariable(t->parent, (uchar *)"NAME");
	    if (!existing)
              existing = htmlGetVariable(t->parent, (uchar *)"ID");
          }
	  if (existing == NULL &&
              t->child != NULL && t->child->markup == MARKUP_A)
          {
	    existing = htmlGetVariable(t->child, (uchar *)"NAME");
	    if (!existing)
              existing = htmlGetVariable(t->child, (uchar *)"ID");
          }
          if (existing != NULL &&
	      strlen((char *)existing) >= 124)	 
	    existing = NULL;
          if (existing != NULL)
	    snprintf((char *)link, sizeof(link), "#%s", existing);
	  else
	    snprintf((char *)link, sizeof(link), "#%s", baselink);
          if (TocNumbers)
	  {
            strlcat((char *)heading, " ", sizeof(heading));
            htmlInsertTree(t, MARKUP_NONE, heading);
	  }
          if (level < TocLevels)
          {
            if (level > last_level)
	    {
	      if (heading_parents[last_level]->last_child && level > 1)
        	heading_parents[level] =
		    htmlAddTree(heading_parents[last_level]->last_child,
                                MARKUP_UL, NULL);
              else
        	heading_parents[level] =
		    htmlAddTree(heading_parents[last_level], MARKUP_UL, NULL);
              DEBUG_printf(("level=%d, last_level=%d, created new UL parent %p\n",
	                    level, last_level, (void *)heading_parents[level]));
	    }
            if (level == 0)
            {
              if (last_level == 0)
              {
                htmlAddTree(heading_parents[level], MARKUP_BR, NULL);
                htmlAddTree(heading_parents[level], MARKUP_BR, NULL);
              }
              parent = htmlAddTree(heading_parents[level], MARKUP_B, NULL);
            }
            else
              parent = htmlAddTree(heading_parents[level], MARKUP_LI, NULL);
            DEBUG_printf(("parent=%p\n", (void *)parent));
            if ((var = htmlGetVariable(t, (uchar *)"_HD_OMIT_TOC")) != NULL)
	      htmlSetVariable(parent, (uchar *)"_HD_OMIT_TOC", var);
            if (TocLinks)
            {
              parent = htmlAddTree(parent, MARKUP_A, NULL);
              htmlSetVariable(parent, (uchar *)"HREF", link);
              if (existing == NULL)
	      {
                if (t->parent != NULL && t->parent->markup == MARKUP_A)
	          htmlSetVariable(t->parent, (uchar *)"NAME", baselink);
		else if (t->child != NULL && t->child->markup == MARKUP_A)
	          htmlSetVariable(t->child, (uchar *)"NAME", baselink);
		else
		{
        	  target = htmlNewTree(t, MARKUP_A, NULL);
        	  htmlSetVariable(target, (uchar *)"NAME", baselink);
        	  for (temp = t->child; temp != NULL; temp = temp->next)
                    temp->parent = target;
        	  target->child = t->child;
        	  t->child      = target;
	        }
	      }
            }
            add_heading(parent, t->child);
          }
          last_level = level;
          break;
      default :
          if (t->child != NULL)
            parse_tree(t->child);
          break;
    }
    t = t->next;
  }
}