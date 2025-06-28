static int
process_data(void)
{
PCRE2_SIZE len, ulen, arg_ulen;
uint32_t gmatched;
uint32_t c, k;
uint32_t g_notempty = 0;
uint8_t *p, *pp, *start_rep;
size_t needlen;
void *use_dat_context;
BOOL utf;
BOOL subject_literal;
PCRE2_SIZE *ovector;
PCRE2_SIZE ovecsave[3];
uint32_t oveccount;
#ifdef SUPPORT_PCRE2_8
uint8_t *q8 = NULL;
#endif
#ifdef SUPPORT_PCRE2_16
uint16_t *q16 = NULL;
#endif
#ifdef SUPPORT_PCRE2_32
uint32_t *q32 = NULL;
#endif
subject_literal = (pat_patctl.control2 & CTL2_SUBJECT_LITERAL) != 0;
DATCTXCPY(dat_context, default_dat_context);
memcpy(&dat_datctl, &def_datctl, sizeof(datctl));
dat_datctl.control |= (pat_patctl.control & CTL_ALLPD);
dat_datctl.control2 |= (pat_patctl.control2 & CTL2_ALLPD);
strcpy((char *)dat_datctl.replacement, (char *)pat_patctl.replacement);
if (dat_datctl.jitstack == 0) dat_datctl.jitstack = pat_patctl.jitstack;
if (dat_datctl.substitute_skip == 0)
    dat_datctl.substitute_skip = pat_patctl.substitute_skip;
if (dat_datctl.substitute_stop == 0)
    dat_datctl.substitute_stop = pat_patctl.substitute_stop;
#ifdef SUPPORT_PCRE2_8
utf = ((((pat_patctl.control & CTL_POSIX) != 0)?
  ((pcre2_real_code_8 *)preg.re_pcre2_code)->overall_options :
  FLD(compiled_code, overall_options)) & PCRE2_UTF) != 0;
#else
utf = (FLD(compiled_code, overall_options) & PCRE2_UTF) != 0;
#endif
start_rep = NULL;
len = strlen((const char *)buffer);
while (len > 0 && isspace(buffer[len-1])) len--;
buffer[len] = 0;
p = buffer;
while (isspace(*p)) p++;
if (utf)
  {
  uint8_t *q;
  uint32_t cc;
  int n = 1;
  for (q = p; n > 0 && *q; q += n) n = utf82ord(q, &cc);
  if (n <= 0)
    {
    fprintf(outfile, "** Failed: invalid UTF-8 string cannot be used as input "
      "in UTF mode\n");
    return PR_OK;
    }
  }
#ifdef SUPPORT_VALGRIND
if (dbuffer != NULL)
  {
  VALGRIND_MAKE_MEM_UNDEFINED(dbuffer, dbuffer_size);
  }
#endif
needlen = (size_t)((len+1) * code_unit_size);
if (dbuffer == NULL || needlen >= dbuffer_size)
  {
  while (needlen >= dbuffer_size) dbuffer_size *= 2;
  dbuffer = (uint8_t *)realloc(dbuffer, dbuffer_size);
  if (dbuffer == NULL)
    {
    fprintf(stderr, "pcre2test: realloc(%d) failed\n", (int)dbuffer_size);
    exit(1);
    }
  }
SETCASTPTR(q, dbuffer);   
while ((c = *p++) != 0)
  {
  int32_t i = 0;
  size_t replen;
  if (c == ']' && start_rep != NULL)
    {
    long li;
    char *endptr;
    if (*p++ != '{')
      {
      fprintf(outfile, "** Expected '{' after \\[....]\n");
      return PR_OK;
      }
    li = strtol((const char *)p, &endptr, 10);
    if (S32OVERFLOW(li))
      {
      fprintf(outfile, "** Repeat count too large\n");
      return PR_OK;
      }
    p = (uint8_t *)endptr;
    if (*p++ != '}')
      {
      fprintf(outfile, "** Expected '}' after \\[...]{...\n");
      return PR_OK;
      }
    i = (int32_t)li;
    if (i-- == 0)
      {
      fprintf(outfile, "** Zero repeat not allowed\n");
      return PR_OK;
      }
    replen = CAST8VAR(q) - start_rep;
    needlen += replen * i;
    if (needlen >= dbuffer_size)
      {
      size_t qoffset = CAST8VAR(q) - dbuffer;
      size_t rep_offset = start_rep - dbuffer;
      while (needlen >= dbuffer_size) dbuffer_size *= 2;
      dbuffer = (uint8_t *)realloc(dbuffer, dbuffer_size);
      if (dbuffer == NULL)
        {
        fprintf(stderr, "pcre2test: realloc(%d) failed\n", (int)dbuffer_size);
        exit(1);
        }
      SETCASTPTR(q, dbuffer + qoffset);
      start_rep = dbuffer + rep_offset;
      }
    while (i-- > 0)
      {
      memcpy(CAST8VAR(q), start_rep, replen);
      SETPLUS(q, replen/code_unit_size);
      }
    start_rep = NULL;
    continue;
    }
  if (c != '\\' || subject_literal)
    {
    uint32_t topbit = 0;
    if (test_mode == PCRE32_MODE && c == 0xff && *p != 0)
      {
      topbit = 0x80000000;
      c = *p++;
      }
    if ((utf || (pat_patctl.control & CTL_UTF8_INPUT) != 0) &&
      HASUTF8EXTRALEN(c)) { GETUTF8INC(c, p); }
    c |= topbit;
    }
  else switch ((c = *p++))
    {
    case '\\': break;
    case 'a': c = CHAR_BEL; break;
    case 'b': c = '\b'; break;
    case 'e': c = CHAR_ESC; break;
    case 'f': c = '\f'; break;
    case 'n': c = '\n'; break;
    case 'r': c = '\r'; break;
    case 't': c = '\t'; break;
    case 'v': c = '\v'; break;
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
    c -= '0';
    while (i++ < 2 && isdigit(*p) && *p != '8' && *p != '9')
      c = c * 8 + *p++ - '0';
    break;
    case 'o':
    if (*p == '{')
      {
      uint8_t *pt = p;
      c = 0;
      for (pt++; isdigit(*pt) && *pt != '8' && *pt != '9'; pt++)
        {
        if (++i == 12)
          fprintf(outfile, "** Too many octal digits in \\o{...} item; "
                           "using only the first twelve.\n");
        else c = c * 8 + *pt - '0';
        }
      if (*pt == '}') p = pt + 1;
        else fprintf(outfile, "** Missing } after \\o{ (assumed)\n");
      }
    break;
    case 'x':
    if (*p == '{')
      {
      uint8_t *pt = p;
      c = 0;
      for (pt++; isxdigit(*pt); pt++)
        {
        if (++i == 9)
          fprintf(outfile, "** Too many hex digits in \\x{...} item; "
                           "using only the first eight.\n");
        else c = c * 16 + tolower(*pt) - ((isdigit(*pt))? '0' : 'a' - 10);
        }
      if (*pt == '}')
        {
        p = pt + 1;
        break;
        }
      }
    c = 0;
    while (i++ < 2 && isxdigit(*p))
      {
      c = c * 16 + tolower(*p) - ((isdigit(*p))? '0' : 'a' - 10);
      p++;
      }
#if defined SUPPORT_PCRE2_8
    if (utf && (test_mode == PCRE8_MODE))
      {
      *q8++ = c;
      continue;
      }
#endif
    break;
    case 0:      
    p--;
    continue;
    case '=':    
    goto ENDSTRING;
    case '[':    
    if (start_rep != NULL)
      {
      fprintf(outfile, "** Nested replication is not supported\n");
      return PR_OK;
      }
    start_rep = CAST8VAR(q);
    continue;
    default:
    if (isalnum(c))
      {
      fprintf(outfile, "** Unrecognized escape sequence \"\\%c\"\n", c);
      return PR_OK;
      }
    }
#ifdef SUPPORT_PCRE2_8
  if (test_mode == PCRE8_MODE)
    {
    if (utf)
      {
      if (c > 0x7fffffff)
        {
        fprintf(outfile, "** Character \\x{%x} is greater than 0x7fffffff "
          "and so cannot be converted to UTF-8\n", c);
        return PR_OK;
        }
      q8 += ord2utf8(c, q8);
      }
    else
      {
      if (c > 0xffu)
        {
        fprintf(outfile, "** Character \\x{%x} is greater than 255 "
          "and UTF-8 mode is not enabled.\n", c);
        fprintf(outfile, "** Truncation will probably give the wrong "
          "result.\n");
        }
      *q8++ = (uint8_t)c;
      }
    }
#endif
#ifdef SUPPORT_PCRE2_16
  if (test_mode == PCRE16_MODE)
    {
    if (utf)
      {
      if (c > 0x10ffffu)
        {
        fprintf(outfile, "** Failed: character \\x{%x} is greater than "
          "0x10ffff and so cannot be converted to UTF-16\n", c);
        return PR_OK;
        }
      else if (c >= 0x10000u)
        {
        c-= 0x10000u;
        *q16++ = 0xD800 | (c >> 10);
        *q16++ = 0xDC00 | (c & 0x3ff);
        }
      else
        *q16++ = c;
      }
    else
      {
      if (c > 0xffffu)
        {
        fprintf(outfile, "** Character \\x{%x} is greater than 0xffff "
          "and UTF-16 mode is not enabled.\n", c);
        fprintf(outfile, "** Truncation will probably give the wrong "
          "result.\n");
        }
      *q16++ = (uint16_t)c;
      }
    }
#endif
#ifdef SUPPORT_PCRE2_32
  if (test_mode == PCRE32_MODE)
    {
    *q32++ = c;
    }
#endif
  }
ENDSTRING:
SET(*q, 0);
len = CASTVAR(uint8_t *, q) - dbuffer;     
ulen = len/code_unit_size;                 
arg_ulen = ulen;                           
if (p[-1] != 0 && !decode_modifiers(p, CTX_DAT, NULL, &dat_datctl))
  return PR_OK;
if (dat_datctl.substitute_skip != 0 || dat_datctl.substitute_stop != 0)
  dat_datctl.control2 |= CTL2_SUBSTITUTE_CALLOUT;
for (k = 0; k < sizeof(exclusive_dat_controls)/sizeof(uint32_t); k++)
  {
  c = dat_datctl.control & exclusive_dat_controls[k];
  if (c != 0 && c != (c & (~c+1)))
    {
    show_controls(c, 0, "** Not allowed together:");
    fprintf(outfile, "\n");
    return PR_OK;
    }
  }
if (pat_patctl.replacement[0] != 0)
  {
  if ((dat_datctl.control2 & CTL2_SUBSTITUTE_CALLOUT) != 0 &&
      (dat_datctl.control & CTL_NULLCONTEXT) != 0)
    {
    fprintf(outfile, "** Replacement callouts are not supported with null_context.\n");
    return PR_OK;
    }
  if ((dat_datctl.control & CTL_ALLCAPTURES) != 0)
    fprintf(outfile, "** Ignored with replacement text: allcaptures\n");
  }
if ((dat_datctl.control & CTL_DFA) != 0)
  {
  if ((dat_datctl.control & CTL_ALLCAPTURES) != 0)
    fprintf(outfile, "** Ignored after DFA matching: allcaptures\n");
  }
c = code_unit_size * (((pat_patctl.control & CTL_POSIX) +
                       (dat_datctl.control & CTL_ZERO_TERMINATE) != 0)? 1:0);
pp = memmove(dbuffer + dbuffer_size - len - c, dbuffer, len + c);
#ifdef SUPPORT_VALGRIND
  VALGRIND_MAKE_MEM_NOACCESS(dbuffer, dbuffer_size - (len + c));
#endif
if ((dat_datctl.control2 & CTL2_NULL_SUBJECT) != 0) pp = NULL;
#ifdef SUPPORT_PCRE2_8
if ((pat_patctl.control & CTL_POSIX) != 0)
  {
  int rc;
  int eflags = 0;
  regmatch_t *pmatch = NULL;
  const char *msg = "** Ignored with POSIX interface:";
  if (dat_datctl.cerror[0] != CFORE_UNSET || dat_datctl.cerror[1] != CFORE_UNSET)
    prmsg(&msg, "callout_error");
  if (dat_datctl.cfail[0] != CFORE_UNSET || dat_datctl.cfail[1] != CFORE_UNSET)
    prmsg(&msg, "callout_fail");
  if (dat_datctl.copy_numbers[0] >= 0 || dat_datctl.copy_names[0] != 0)
    prmsg(&msg, "copy");
  if (dat_datctl.get_numbers[0] >= 0 || dat_datctl.get_names[0] != 0)
    prmsg(&msg, "get");
  if (dat_datctl.jitstack != 0) prmsg(&msg, "jitstack");
  if (dat_datctl.offset != 0) prmsg(&msg, "offset");
  if ((dat_datctl.options & ~POSIX_SUPPORTED_MATCH_OPTIONS) != 0)
    {
    fprintf(outfile, "%s", msg);
    show_match_options(dat_datctl.options & ~POSIX_SUPPORTED_MATCH_OPTIONS);
    msg = "";
    }
  if ((dat_datctl.control & ~POSIX_SUPPORTED_MATCH_CONTROLS) != 0 ||
      (dat_datctl.control2 & ~POSIX_SUPPORTED_MATCH_CONTROLS2) != 0)
    {
    show_controls(dat_datctl.control & ~POSIX_SUPPORTED_MATCH_CONTROLS,
                  dat_datctl.control2 & ~POSIX_SUPPORTED_MATCH_CONTROLS2, msg);
    msg = "";
    }
  if (msg[0] == 0) fprintf(outfile, "\n");
  if (dat_datctl.oveccount > 0)
    {
    pmatch = (regmatch_t *)malloc(sizeof(regmatch_t) * dat_datctl.oveccount);
    if (pmatch == NULL)
      {
      fprintf(outfile, "** Failed to get memory for recording matching "
        "information (size set = %du)\n", dat_datctl.oveccount);
      return PR_OK;
      }
    }
  if (dat_datctl.startend[0] != CFORE_UNSET)
    {
    pmatch[0].rm_so = dat_datctl.startend[0];
    pmatch[0].rm_eo = (dat_datctl.startend[1] != 0)?
      dat_datctl.startend[1] : len;
    eflags |= REG_STARTEND;
    }
  if ((dat_datctl.options & PCRE2_NOTBOL) != 0) eflags |= REG_NOTBOL;
  if ((dat_datctl.options & PCRE2_NOTEOL) != 0) eflags |= REG_NOTEOL;
  if ((dat_datctl.options & PCRE2_NOTEMPTY) != 0) eflags |= REG_NOTEMPTY;
  rc = regexec(&preg, (const char *)pp, dat_datctl.oveccount, pmatch, eflags);
  if (rc != 0)
    {
    (void)regerror(rc, &preg, (char *)pbuffer8, pbuffer8_size);
    fprintf(outfile, "No match: POSIX code %d: %s\n", rc, pbuffer8);
    }
  else if ((pat_patctl.control & CTL_POSIX_NOSUB) != 0)
    fprintf(outfile, "Matched with REG_NOSUB\n");
  else if (dat_datctl.oveccount == 0)
    fprintf(outfile, "Matched without capture\n");
  else
    {
    size_t i, j;
    size_t last_printed = (size_t)dat_datctl.oveccount;
    for (i = 0; i < (size_t)dat_datctl.oveccount; i++)
      {
      if (pmatch[i].rm_so >= 0)
        {
        PCRE2_SIZE start = pmatch[i].rm_so;
        PCRE2_SIZE end = pmatch[i].rm_eo;
        for (j = last_printed + 1; j < i; j++)
          fprintf(outfile, "%2d: <unset>\n", (int)j);
        last_printed = i;
        if (start > end)
          {
          start = pmatch[i].rm_eo;
          end = pmatch[i].rm_so;
          fprintf(outfile, "Start of matched string is beyond its end - "
            "displaying from end to start.\n");
          }
        fprintf(outfile, "%2d: ", (int)i);
        PCHARSV(pp, start, end - start, utf, outfile);
        fprintf(outfile, "\n");
        if ((i == 0 && (dat_datctl.control & CTL_AFTERTEXT) != 0) ||
            (dat_datctl.control & CTL_ALLAFTERTEXT) != 0)
          {
          fprintf(outfile, "%2d+ ", (int)i);
          PCHARSV(pp, pmatch[i].rm_eo, len - pmatch[i].rm_eo, utf, outfile);
          fprintf(outfile, "\n"); }
        }
      }
    }
  free(pmatch);
  return PR_OK;
  }
#endif   
if (dat_datctl.startend[0] != CFORE_UNSET)
  fprintf(outfile, "** \\=posix_startend ignored for non-POSIX matching\n");
if ((dat_datctl.control & (CTL_ALLUSEDTEXT|CTL_DFA)) == CTL_ALLUSEDTEXT &&
    FLD(compiled_code, executable_jit) != NULL)
  {
  fprintf(outfile, "** Showing all consulted text is not supported by JIT: ignored\n");
  dat_datctl.control &= ~CTL_ALLUSEDTEXT;
  }
if ((dat_datctl.control & CTL_ZERO_TERMINATE) != 0)
  arg_ulen = PCRE2_ZERO_TERMINATED;
use_dat_context = ((dat_datctl.control & CTL_NULLCONTEXT) != 0)?
  NULL : PTR(dat_context);
show_memory = (dat_datctl.control & CTL_MEMORY) != 0;
if (show_memory &&
    (pat_patctl.control & dat_datctl.control & CTL_NULLCONTEXT) != 0)
  fprintf(outfile, "** \\=memory requires either a pattern or a subject "
    "context: ignored\n");
if (dat_datctl.jitstack != 0)
  {
  if (dat_datctl.jitstack != jit_stack_size)
    {
    PCRE2_JIT_STACK_FREE(jit_stack);
    PCRE2_JIT_STACK_CREATE(jit_stack, 1, dat_datctl.jitstack * 1024, NULL);
    jit_stack_size = dat_datctl.jitstack;
    }
  PCRE2_JIT_STACK_ASSIGN(dat_context, jit_callback, jit_stack);
  }
else if (jit_stack != NULL)
  {
  PCRE2_JIT_STACK_ASSIGN(dat_context, NULL, NULL);
  PCRE2_JIT_STACK_FREE(jit_stack);
  jit_stack = NULL;
  jit_stack_size = 0;
  }
if ((pat_patctl.control & CTL_JITVERIFY) != 0 && jit_stack == NULL)
   {
   PCRE2_JIT_STACK_ASSIGN(dat_context, jit_callback, NULL);
   }
if (dat_datctl.oveccount == 0)
  {
  PCRE2_MATCH_DATA_FREE(match_data);
  PCRE2_MATCH_DATA_CREATE_FROM_PATTERN(match_data, compiled_code,
    general_context);
  PCRE2_GET_OVECTOR_COUNT(max_oveccount, match_data);
  }
else if (dat_datctl.oveccount <= max_oveccount)
  {
  SETFLD(match_data, oveccount, dat_datctl.oveccount);
  }
else
  {
  max_oveccount = dat_datctl.oveccount;
  PCRE2_MATCH_DATA_FREE(match_data);
  PCRE2_MATCH_DATA_CREATE(match_data, max_oveccount, general_context);
  }
if (CASTVAR(void *, match_data) == NULL)
  {
  fprintf(outfile, "** Failed to get memory for recording matching "
    "information (size requested: %d)\n", dat_datctl.oveccount);
  max_oveccount = 0;
  return PR_OK;
  }
ovector = FLD(match_data, ovector);
PCRE2_GET_OVECTOR_COUNT(oveccount, match_data);
if (dat_datctl.replacement[0] != 0 && (dat_datctl.control & CTL_DFA) != 0)
  {
  fprintf(outfile, "** Ignored for DFA matching: replace\n");
  dat_datctl.replacement[0] = 0;
  }
if (dat_datctl.replacement[0] != 0)
  {
  int rc;
  uint8_t *pr;
  uint8_t rbuffer[REPLACE_BUFFSIZE];
  uint8_t nbuffer[REPLACE_BUFFSIZE];
  uint8_t *rbptr;
  uint32_t xoptions;
  uint32_t emoption;   
  PCRE2_SIZE j, rlen, nsize, erroroffset;
  BOOL badutf = FALSE;
#ifdef SUPPORT_PCRE2_8
  uint8_t *r8 = NULL;
#endif
#ifdef SUPPORT_PCRE2_16
  uint16_t *r16 = NULL;
#endif
#ifdef SUPPORT_PCRE2_32
  uint32_t *r32 = NULL;
#endif
  for (j = 0; j < 2*oveccount; j++) ovector[j] = JUNK_OFFSET;
  if (timeitm)
    fprintf(outfile, "** Timing is not supported with replace: ignored\n");
  if ((dat_datctl.control & CTL_ALTGLOBAL) != 0)
    fprintf(outfile, "** Altglobal is not supported with replace: ignored\n");
  emoption = ((dat_datctl.control2 & CTL2_SUBSTITUTE_MATCHED) == 0)? 0 :
    PCRE2_SUBSTITUTE_MATCHED;
  if (emoption != 0)
    {
    PCRE2_MATCH(rc, compiled_code, pp, arg_ulen, dat_datctl.offset,
      dat_datctl.options, match_data, use_dat_context);
    }
  xoptions = emoption |
             (((dat_datctl.control & CTL_GLOBAL) == 0)? 0 :
                PCRE2_SUBSTITUTE_GLOBAL) |
             (((dat_datctl.control2 & CTL2_SUBSTITUTE_EXTENDED) == 0)? 0 :
                PCRE2_SUBSTITUTE_EXTENDED) |
             (((dat_datctl.control2 & CTL2_SUBSTITUTE_LITERAL) == 0)? 0 :
                PCRE2_SUBSTITUTE_LITERAL) |
             (((dat_datctl.control2 & CTL2_SUBSTITUTE_OVERFLOW_LENGTH) == 0)? 0 :
                PCRE2_SUBSTITUTE_OVERFLOW_LENGTH) |
             (((dat_datctl.control2 & CTL2_SUBSTITUTE_REPLACEMENT_ONLY) == 0)? 0 :
                PCRE2_SUBSTITUTE_REPLACEMENT_ONLY) |
             (((dat_datctl.control2 & CTL2_SUBSTITUTE_UNKNOWN_UNSET) == 0)? 0 :
                PCRE2_SUBSTITUTE_UNKNOWN_UNSET) |
             (((dat_datctl.control2 & CTL2_SUBSTITUTE_UNSET_EMPTY) == 0)? 0 :
                PCRE2_SUBSTITUTE_UNSET_EMPTY);
  SETCASTPTR(r, rbuffer);   
  pr = dat_datctl.replacement;
  nsize = REPLACE_BUFFSIZE/code_unit_size;
  if (*pr == '[')
    {
    PCRE2_SIZE n = 0;
    while ((c = *(++pr)) >= CHAR_0 && c <= CHAR_9) n = n * 10 + c - CHAR_0;
    if (*pr++ != ']')
      {
      fprintf(outfile, "Bad buffer size in replacement string\n");
      return PR_OK;
      }
    if (n > nsize)
      {
      fprintf(outfile, "Replacement buffer setting (%" SIZ_FORM ") is too "
        "large (max %" SIZ_FORM ")\n", n, nsize);
      return PR_OK;
      }
    nsize = n;
    }
  if (utf) badutf = valid_utf(pr, strlen((const char *)pr), &erroroffset);
  if (!utf || badutf)
    {
    while ((c = *pr++) != 0)
      {
#ifdef SUPPORT_PCRE2_8
      if (test_mode == PCRE8_MODE) *r8++ = c;
#endif
#ifdef SUPPORT_PCRE2_16
      if (test_mode == PCRE16_MODE) *r16++ = c;
#endif
#ifdef SUPPORT_PCRE2_32
      if (test_mode == PCRE32_MODE) *r32++ = c;
#endif
      }
    }
  else while ((c = *pr++) != 0)
    {
    if (HASUTF8EXTRALEN(c)) { GETUTF8INC(c, pr); }
#ifdef SUPPORT_PCRE2_8
    if (test_mode == PCRE8_MODE) r8 += ord2utf8(c, r8);
#endif
#ifdef SUPPORT_PCRE2_16
    if (test_mode == PCRE16_MODE)
      {
      if (c >= 0x10000u)
        {
        c-= 0x10000u;
        *r16++ = 0xD800 | (c >> 10);
        *r16++ = 0xDC00 | (c & 0x3ff);
        }
      else *r16++ = c;
      }
#endif
#ifdef SUPPORT_PCRE2_32
    if (test_mode == PCRE32_MODE) *r32++ = c;
#endif
    }
  SET(*r, 0);
  if ((dat_datctl.control & CTL_ZERO_TERMINATE) != 0)
    rlen = PCRE2_ZERO_TERMINATED;
  else
    rlen = (CASTVAR(uint8_t *, r) - rbuffer)/code_unit_size;
  if ((dat_datctl.control2 & CTL2_SUBSTITUTE_CALLOUT) != 0)
    {
    PCRE2_SET_SUBSTITUTE_CALLOUT(dat_context, substitute_callout_function, NULL);
    }
  else
    {
    PCRE2_SET_SUBSTITUTE_CALLOUT(dat_context, NULL, NULL);   
    }
  rbptr = ((dat_datctl.control2 & CTL2_NULL_REPLACEMENT) == 0)? rbuffer : NULL;
  PCRE2_SUBSTITUTE(rc, compiled_code, pp, arg_ulen, dat_datctl.offset,
    dat_datctl.options|xoptions, match_data, use_dat_context,
    rbptr, rlen, nbuffer, &nsize);
  if (rc < 0)
    {
    fprintf(outfile, "Failed: error %d", rc);
    if (rc != PCRE2_ERROR_NOMEMORY && nsize != PCRE2_UNSET)
      fprintf(outfile, " at offset %ld in replacement", (long int)nsize);
    fprintf(outfile, ": ");
    if (!print_error_message(rc, "", "")) return PR_ABEND;
    if (rc == PCRE2_ERROR_NOMEMORY &&
        (xoptions & PCRE2_SUBSTITUTE_OVERFLOW_LENGTH) != 0)
      fprintf(outfile, ": %ld code units are needed", (long int)nsize);
    }
  else
    {
    fprintf(outfile, "%2d: ", rc);
    PCHARSV(nbuffer, 0, nsize, utf, outfile);
    }
  fprintf(outfile, "\n");
  show_memory = FALSE;
  if ((dat_datctl.control2 & CTL2_ALLVECTOR) != 0)
    show_ovector(ovector, oveccount);
  return PR_OK;
  }    
ovecsave[0] = ovecsave[1] = ovecsave[2] = PCRE2_UNSET;
for (gmatched = 0;; gmatched++)
  {
  PCRE2_SIZE j;
  int capcount;
  for (j = 0; j < 2*oveccount; j++) ovector[j] = JUNK_OFFSET;
  jit_was_used = (pat_patctl.control & CTL_JITFAST) != 0;
  if (timeitm > 0)
    {
    int i;
    clock_t start_time, time_taken;
    if ((dat_datctl.control & CTL_DFA) != 0)
      {
      if ((dat_datctl.options & PCRE2_DFA_RESTART) != 0)
        {
        fprintf(outfile, "Timing DFA restarts is not supported\n");
        return PR_OK;
        }
      if (dfa_workspace == NULL)
        dfa_workspace = (int *)malloc(DFA_WS_DIMENSION*sizeof(int));
      start_time = clock();
      for (i = 0; i < timeitm; i++)
        {
        PCRE2_DFA_MATCH(capcount, compiled_code, pp, arg_ulen,
          dat_datctl.offset, dat_datctl.options | g_notempty, match_data,
          use_dat_context, dfa_workspace, DFA_WS_DIMENSION);
        }
      }
    else if ((pat_patctl.control & CTL_JITFAST) != 0)
      {
      start_time = clock();
      for (i = 0; i < timeitm; i++)
        {
        PCRE2_JIT_MATCH(capcount, compiled_code, pp, arg_ulen,
          dat_datctl.offset, dat_datctl.options | g_notempty, match_data,
          use_dat_context);
        }
      }
    else
      {
      start_time = clock();
      for (i = 0; i < timeitm; i++)
        {
        PCRE2_MATCH(capcount, compiled_code, pp, arg_ulen,
          dat_datctl.offset, dat_datctl.options | g_notempty, match_data,
          use_dat_context);
        }
      }
    total_match_time += (time_taken = clock() - start_time);
    fprintf(outfile, "Match time %.4f milliseconds\n",
      (((double)time_taken * 1000.0) / (double)timeitm) /
        (double)CLOCKS_PER_SEC);
    }
  if ((dat_datctl.control & (CTL_FINDLIMITS|CTL_FINDLIMITS_NOHEAP)) != 0)
    {
    capcount = 0;   
    if ((dat_datctl.control & CTL_FINDLIMITS_NOHEAP) == 0 &&
        (FLD(compiled_code, executable_jit) == NULL ||
          (dat_datctl.options & PCRE2_NO_JIT) != 0))
      {
      (void)check_match_limit(pp, arg_ulen, PCRE2_ERROR_HEAPLIMIT, "heap");
      }
    capcount = check_match_limit(pp, arg_ulen, PCRE2_ERROR_MATCHLIMIT,
      "match");
    if (FLD(compiled_code, executable_jit) == NULL ||
        (dat_datctl.options & PCRE2_NO_JIT) != 0 ||
        (dat_datctl.control & CTL_DFA) != 0)
      {
      capcount = check_match_limit(pp, arg_ulen, PCRE2_ERROR_DEPTHLIMIT,
        "depth");
      }
    if (capcount == 0)
      {
      fprintf(outfile, "Matched, but offsets vector is too small to show all matches\n");
      capcount = dat_datctl.oveccount;
      }
    }
  else
    {
    if ((dat_datctl.control & CTL_CALLOUT_NONE) == 0)
      {
      PCRE2_SET_CALLOUT(dat_context, callout_function,
        (void *)(&dat_datctl.callout_data));
      first_callout = TRUE;
      last_callout_mark = NULL;
      callout_count = 0;
      }
    else
      {
      PCRE2_SET_CALLOUT(dat_context, NULL, NULL);   
      }
    if ((dat_datctl.control & CTL_DFA) != 0)
      {
      if (dfa_workspace == NULL)
        dfa_workspace = (int *)malloc(DFA_WS_DIMENSION*sizeof(int));
      if (dfa_matched++ == 0)
        dfa_workspace[0] = -1;   
      PCRE2_DFA_MATCH(capcount, compiled_code, pp, arg_ulen,
        dat_datctl.offset, dat_datctl.options | g_notempty, match_data,
        use_dat_context, dfa_workspace, DFA_WS_DIMENSION);
      if (capcount == 0)
        {
        fprintf(outfile, "Matched, but offsets vector is too small to show all matches\n");
        capcount = dat_datctl.oveccount;
        }
      }
    else
      {
      if ((pat_patctl.control & CTL_JITFAST) != 0)
        PCRE2_JIT_MATCH(capcount, compiled_code, pp, arg_ulen, dat_datctl.offset,
          dat_datctl.options | g_notempty, match_data, use_dat_context);
      else
        PCRE2_MATCH(capcount, compiled_code, pp, arg_ulen, dat_datctl.offset,
          dat_datctl.options | g_notempty, match_data, use_dat_context);
      if (capcount == 0)
        {
        fprintf(outfile, "Matched, but too many substrings\n");
        capcount = dat_datctl.oveccount;
        }
      }
    }
  if (capcount >= 0)
    {
    int i;
    if (pp == NULL) pp = (uint8_t *)"";
    if (capcount > (int)oveccount)    
      {
      fprintf(outfile,
        "** PCRE2 error: returned count %d is too big for ovector count %d\n",
        capcount, oveccount);
      capcount = oveccount;
      if ((dat_datctl.control & CTL_ANYGLOB) != 0)
        {
        fprintf(outfile, "** Global loop abandoned\n");
        dat_datctl.control &= ~CTL_ANYGLOB;         
        }
      }
    if ((dat_datctl.options & PCRE2_COPY_MATCHED_SUBJECT) != 0 &&
        (pat_patctl.control & CTL_JITFAST) == 0)
      {
      if ((FLD(match_data, flags) & PCRE2_MD_COPIED_SUBJECT) == 0)
        fprintf(outfile,
          "** PCRE2 error: flag not set after copy_matched_subject\n");
      if (CASTFLD(void *, match_data, subject) == pp)
        fprintf(outfile,
          "** PCRE2 error: copy_matched_subject has not copied\n");
      if (memcmp(CASTFLD(void *, match_data, subject), pp, ulen) != 0)
        fprintf(outfile,
          "** PCRE2 error: copy_matched_subject mismatch\n");
      }
    if (gmatched > 0 && ovecsave[0] == ovector[0] && ovecsave[1] == ovector[1])
      {
      if (ovector[0] == ovector[1] && ovecsave[2] != dat_datctl.offset)
        {
        g_notempty = PCRE2_NOTEMPTY_ATSTART | PCRE2_ANCHORED;
        ovecsave[2] = dat_datctl.offset;
        continue;     
        }
      fprintf(outfile,
        "** PCRE2 error: global repeat returned the same string as previous\n");
      fprintf(outfile, "** Global loop abandoned\n");
      dat_datctl.control &= ~CTL_ANYGLOB;         
      }
    if ((dat_datctl.control & (CTL_ALLCAPTURES|CTL_DFA)) == CTL_ALLCAPTURES)
      {
      capcount = maxcapcount + 1;    
      if (capcount > (int)oveccount) capcount = oveccount;
      }
    if ((dat_datctl.control2 & CTL2_ALLVECTOR) != 0) capcount = oveccount;
    for (i = 0; i < 2*capcount; i += 2)
      {
      PCRE2_SIZE lleft, lmiddle, lright;
      PCRE2_SIZE start = ovector[i];
      PCRE2_SIZE end = ovector[i+1];
      if (start > end)
        {
        start = ovector[i+1];
        end = ovector[i];
        fprintf(outfile, "Start of matched string is beyond its end - "
          "displaying from end to start.\n");
        }
      fprintf(outfile, "%2d: ", i/2);
      if (start == PCRE2_UNSET && end == PCRE2_UNSET)
        {
        fprintf(outfile, "<unset>\n");
        continue;
        }
      if (start > ulen || end > ulen)
        {
        if (((dat_datctl.control & CTL_DFA) != 0 ||
              i >= (int)(2*maxcapcount + 2)) &&
            start == JUNK_OFFSET && end == JUNK_OFFSET)
          fprintf(outfile, "<unchanged>\n");
        else
          fprintf(outfile, "ERROR: bad value(s) for offset(s): 0x%lx 0x%lx\n",
            (unsigned long int)start, (unsigned long int)end);
        continue;
        }
      if (i == 0)
        {
        BOOL showallused;
        PCRE2_SIZE leftchar, rightchar;
        if ((dat_datctl.control & CTL_ALLUSEDTEXT) != 0)
          {
          leftchar = FLD(match_data, leftchar);
          rightchar = FLD(match_data, rightchar);
          showallused = i == 0 && (leftchar < start || rightchar > end);
          }
        else showallused = FALSE;
        if (showallused)
          {
          PCHARS(lleft, pp, leftchar, start - leftchar, utf, outfile);
          PCHARS(lmiddle, pp, start, end - start, utf, outfile);
          PCHARS(lright, pp, end, rightchar - end, utf, outfile);
          if ((pat_patctl.control & CTL_JITVERIFY) != 0 && jit_was_used)
            fprintf(outfile, " (JIT)");
          fprintf(outfile, "\n    ");
          for (j = 0; j < lleft; j++) fprintf(outfile, "<");
          for (j = 0; j < lmiddle; j++) fprintf(outfile, " ");
          for (j = 0; j < lright; j++) fprintf(outfile, ">");
          }
        else if ((dat_datctl.control & CTL_STARTCHAR) != 0)
          {
          PCRE2_SIZE startchar;
          PCRE2_GET_STARTCHAR(startchar, match_data);
          PCHARS(lleft, pp, startchar, start - startchar, utf, outfile);
          PCHARSV(pp, start, end - start, utf, outfile);
          if ((pat_patctl.control & CTL_JITVERIFY) != 0 && jit_was_used)
            fprintf(outfile, " (JIT)");
          if (startchar != start)
            {
            fprintf(outfile, "\n    ");
            for (j = 0; j < lleft; j++) fprintf(outfile, "^");
            }
          }
        else
          {
          PCHARSV(pp, start, end - start, utf, outfile);
          if ((pat_patctl.control & CTL_JITVERIFY) != 0 && jit_was_used)
            fprintf(outfile, " (JIT)");
          }
        }
      else
        {
        PCHARSV(pp, start, end - start, utf, outfile);
        }
      fprintf(outfile, "\n");
      if ((dat_datctl.control & CTL_ALLAFTERTEXT) != 0 ||
          (i == 0 && (dat_datctl.control & CTL_AFTERTEXT) != 0))
        {
        fprintf(outfile, "%2d+ ", i/2);
        PCHARSV(pp, ovector[i+1], ulen - ovector[i+1], utf, outfile);
        fprintf(outfile, "\n");
        }
      }
    if ((dat_datctl.control & CTL_MARK) != 0 &&
         TESTFLD(match_data, mark, !=, NULL))
      {
      fprintf(outfile, "MK: ");
      PCHARSV(CASTFLD(void *, match_data, mark), -1, -1, utf, outfile);
      fprintf(outfile, "\n");
      }
    if (!copy_and_get(utf, capcount)) return PR_ABEND;
    }     
  else if (capcount == PCRE2_ERROR_PARTIAL)
    {
    PCRE2_SIZE leftchar;
    int backlength;
    int rubriclength = 0;
    if ((dat_datctl.control & CTL_ALLUSEDTEXT) != 0)
      {
      leftchar = FLD(match_data, leftchar);
      }
    else leftchar = ovector[0];
    fprintf(outfile, "Partial match");
    if ((dat_datctl.control & CTL_MARK) != 0 &&
         TESTFLD(match_data, mark, !=, NULL))
      {
      fprintf(outfile, ", mark=");
      PCHARS(rubriclength, CASTFLD(void *, match_data, mark), -1, -1, utf,
        outfile);
      rubriclength += 7;
      }
    fprintf(outfile, ": ");
    rubriclength += 15;
    PCHARS(backlength, pp, leftchar, ovector[0] - leftchar, utf, outfile);
    PCHARSV(pp, ovector[0], ulen - ovector[0], utf, outfile);
    if ((pat_patctl.control & CTL_JITVERIFY) != 0 && jit_was_used)
      fprintf(outfile, " (JIT)");
    fprintf(outfile, "\n");
    if (backlength != 0)
      {
      int i;
      for (i = 0; i < rubriclength; i++) fprintf(outfile, " ");
      for (i = 0; i < backlength; i++) fprintf(outfile, "<");
      fprintf(outfile, "\n");
      }
    if (ulen != ovector[1])
      fprintf(outfile, "** ovector[1] is not equal to the subject length: "
        "%ld != %ld\n", (unsigned long int)ovector[1], (unsigned long int)ulen);
    if (!copy_and_get(utf, 1)) return PR_ABEND;
    if ((dat_datctl.control2 & CTL2_ALLVECTOR) != 0)
      show_ovector(ovector, oveccount);
    break;   
    }        
  else if (g_notempty != 0)    
    {
    uint16_t nl = FLD(compiled_code, newline_convention);
    PCRE2_SIZE start_offset = dat_datctl.offset;     
    PCRE2_SIZE end_offset = start_offset + 1;
    if ((nl == PCRE2_NEWLINE_CRLF || nl == PCRE2_NEWLINE_ANY ||
         nl == PCRE2_NEWLINE_ANYCRLF) &&
        start_offset < ulen - 1 &&
        CODE_UNIT(pp, start_offset) == '\r' &&
        CODE_UNIT(pp, end_offset) == '\n')
      end_offset++;
    else if (utf && test_mode != PCRE32_MODE)
      {
      if (test_mode == PCRE8_MODE)
        {
        for (; end_offset < ulen; end_offset++)
          if ((((PCRE2_SPTR8)pp)[end_offset] & 0xc0) != 0x80) break;
        }
      else   
        {
        for (; end_offset < ulen; end_offset++)
          if ((((PCRE2_SPTR16)pp)[end_offset] & 0xfc00) != 0xdc00) break;
        }
      }
    SETFLDVEC(match_data, ovector, 0, start_offset);
    SETFLDVEC(match_data, ovector, 1, end_offset);
    }   
  else
    {
    switch(capcount)
      {
      case PCRE2_ERROR_NOMATCH:
      if (gmatched == 0)
        {
        fprintf(outfile, "No match");
        if ((dat_datctl.control & CTL_MARK) != 0 &&
             TESTFLD(match_data, mark, !=, NULL))
          {
          fprintf(outfile, ", mark = ");
          PCHARSV(CASTFLD(void *, match_data, mark), -1, -1, utf, outfile);
          }
        if ((pat_patctl.control & CTL_JITVERIFY) != 0 && jit_was_used)
          fprintf(outfile, " (JIT)");
        fprintf(outfile, "\n");
        if ((dat_datctl.control2 & CTL2_ALLVECTOR) != 0)
          show_ovector(ovector, oveccount);
        }
      break;
      case PCRE2_ERROR_BADUTFOFFSET:
      fprintf(outfile, "Error %d (bad UTF-%d offset)\n", capcount, test_mode);
      break;
      default:
      fprintf(outfile, "Failed: error %d: ", capcount);
      if (!print_error_message(capcount, "", "")) return PR_ABEND;
      if (capcount <= PCRE2_ERROR_UTF8_ERR1 &&
          capcount >= PCRE2_ERROR_UTF32_ERR2)
        {
        PCRE2_SIZE startchar;
        PCRE2_GET_STARTCHAR(startchar, match_data);
        fprintf(outfile, " at offset %" SIZ_FORM, startchar);
        }
      fprintf(outfile, "\n");
      break;
      }
    break;   
    }        
  if ((dat_datctl.control & CTL_ANYGLOB) == 0) break; else
    {
    PCRE2_SIZE match_offset = FLD(match_data, ovector)[0];
    PCRE2_SIZE end_offset = FLD(match_data, ovector)[1];
    if (match_offset == end_offset)
      {
      if (end_offset == ulen) break;            
      if (match_offset <= dat_datctl.offset)
        g_notempty = PCRE2_NOTEMPTY_ATSTART | PCRE2_ANCHORED;
      }
    else
      {
      g_notempty = 0;    
      if ((dat_datctl.control & CTL_GLOBAL) != 0)
        {
        PCRE2_SIZE startchar;
        PCRE2_GET_STARTCHAR(startchar, match_data);
        if (end_offset <= startchar)
          {
          if (startchar >= ulen) break;        
          end_offset = startchar + 1;
          if (utf && test_mode != PCRE32_MODE)
            {
            if (test_mode == PCRE8_MODE)
              {
              for (; end_offset < ulen; end_offset++)
                if ((((PCRE2_SPTR8)pp)[end_offset] & 0xc0) != 0x80) break;
              }
            else   
              {
              for (; end_offset < ulen; end_offset++)
                if ((((PCRE2_SPTR16)pp)[end_offset] & 0xfc00) != 0xdc00) break;
              }
            }
          }
        }
      }
    if ((dat_datctl.control & CTL_GLOBAL) != 0)
      {
      ovecsave[0] = ovector[0];
      ovecsave[1] = ovector[1];
      ovecsave[2] = dat_datctl.offset;
      dat_datctl.offset = end_offset;
      }
    else
      {
      pp += end_offset * code_unit_size;
      len -= end_offset * code_unit_size;
      ulen -= end_offset;
      if (arg_ulen != PCRE2_ZERO_TERMINATED) arg_ulen -= end_offset;
      }
    }
  }   
show_memory = FALSE;