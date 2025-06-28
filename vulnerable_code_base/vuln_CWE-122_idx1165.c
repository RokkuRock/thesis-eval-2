fribidi_cap_rtl_to_unicode (
  const char *s,
  FriBidiStrIndex len,
  FriBidiChar *us
)
{
  FriBidiStrIndex i, j;
  if (!caprtl_to_unicode)
    init_cap_rtl ();
  j = 0;
  for (i = 0; i < len; i++)
    {
      char ch;
      ch = s[i];
      if (ch == '_')
	{
	  switch (ch = s[++i])
	    {
	    case '>':
	      us[j++] = FRIBIDI_CHAR_LRM;
	      break;
	    case '<':
	      us[j++] = FRIBIDI_CHAR_RLM;
	      break;
	    case 'l':
	      us[j++] = FRIBIDI_CHAR_LRE;
	      break;
	    case 'r':
	      us[j++] = FRIBIDI_CHAR_RLE;
	      break;
	    case 'o':
	      us[j++] = FRIBIDI_CHAR_PDF;
	      break;
	    case 'L':
	      us[j++] = FRIBIDI_CHAR_LRO;
	      break;
	    case 'R':
	      us[j++] = FRIBIDI_CHAR_RLO;
	      break;
            case 'i':
              us[j++] = FRIBIDI_CHAR_LRI;
	      break;
            case 'y':
              us[j++] = FRIBIDI_CHAR_RLI;
	      break;
            case 'f':
              us[j++] = FRIBIDI_CHAR_FSI;
	      break;
            case 'I':
              us[j++] = FRIBIDI_CHAR_PDI;
	      break;
	    case '_':
	      us[j++] = '_';
	      break;
	    default:
	      us[j++] = '_';
	      i--;
	      break;
	    }
	}
      else
	us[j++] = caprtl_to_unicode[(int) s[i]];
    }
  return j;
}