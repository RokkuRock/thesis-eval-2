	while (  1 )		 
		{
		yy_cp = yyg->yy_c_buf_p;
		*yy_cp = yyg->yy_hold_char;
		yy_bp = yy_cp;
		yy_current_state = yyg->yy_start;
yy_match:
		do
			{
			YY_CHAR yy_c = yy_ec[YY_SC_TO_UI(*yy_cp)] ;
			if ( yy_accept[yy_current_state] )
				{
				yyg->yy_last_accepting_state = yy_current_state;
				yyg->yy_last_accepting_cpos = yy_cp;
				}
			while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
				{
				yy_current_state = (int) yy_def[yy_current_state];
				if ( yy_current_state >= 45 )
					yy_c = yy_meta[(unsigned int) yy_c];
				}
			yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
			++yy_cp;
			}
		while ( yy_current_state != 44 );
		yy_cp = yyg->yy_last_accepting_cpos;
		yy_current_state = yyg->yy_last_accepting_state;
yy_find_action:
		yy_act = yy_accept[yy_current_state];
		YY_DO_BEFORE_ACTION;
		if ( yy_act != YY_END_OF_BUFFER && yy_rule_can_match_eol[yy_act] )
			{
			yy_size_t yyl;
			for ( yyl = 0; yyl < yyleng; ++yyl )
				if ( yytext[yyl] == '\n' )
    do{ yylineno++;
        yycolumn=0;
    }while(0)
;
			}
do_action:	 
		switch ( yy_act )
	{  
			case 0:  
			*yy_cp = yyg->yy_hold_char;
			yy_cp = yyg->yy_last_accepting_cpos;
			yy_current_state = yyg->yy_last_accepting_state;
			goto yy_find_action;
case 1:
YY_RULE_SETUP
#line 101 "re_lexer.l"
{
  int hi_bound;
  int lo_bound = atoi(yytext + 1);
  char* comma = strchr(yytext, ',');
  if (comma - yytext == strlen(yytext) - 2)
    hi_bound = INT16_MAX;
  else
    hi_bound = atoi(comma + 1);
  if (hi_bound > INT16_MAX)
  {
    yyerror(yyscanner, lex_env, "repeat interval too large");
    yyterminate();
  }
  if (hi_bound < lo_bound || hi_bound < 0 || lo_bound < 0)
  {
    yyerror(yyscanner, lex_env, "bad repeat interval");
    yyterminate();
  }
  yylval->range = (hi_bound << 16) | lo_bound;
  return _RANGE_;
}
	YY_BREAK
case 2:
YY_RULE_SETUP
#line 135 "re_lexer.l"
{
  int value = atoi(yytext + 1);
  if (value > INT16_MAX)
  {
    yyerror(yyscanner, lex_env, "repeat interval too large");
    yyterminate();
  }
  yylval->range = (value << 16) | value;
  return _RANGE_;
}
	YY_BREAK
case 3:
YY_RULE_SETUP
#line 153 "re_lexer.l"
{
  BEGIN(char_class);
  memset(LEX_ENV->class_vector, 0, 32);
  LEX_ENV->negated_class = TRUE;
}
	YY_BREAK
case 4:
YY_RULE_SETUP
#line 162 "re_lexer.l"
{
  BEGIN(char_class);
  memset(LEX_ENV->class_vector, 0, 32);
  LEX_ENV->negated_class = TRUE;
  LEX_ENV->class_vector[']' / 8] |= 1 << ']' % 8;
}
	YY_BREAK
case 5:
YY_RULE_SETUP
#line 175 "re_lexer.l"
{
  BEGIN(char_class);
  memset(LEX_ENV->class_vector, 0, 32);
  LEX_ENV->negated_class = FALSE;
  LEX_ENV->class_vector[']' / 8] |= 1 << ']' % 8;
}
	YY_BREAK
case 6:
YY_RULE_SETUP
#line 188 "re_lexer.l"
{
  BEGIN(char_class);
  memset(LEX_ENV->class_vector, 0, 32);
  LEX_ENV->negated_class = FALSE;
}
	YY_BREAK
case 7:
YY_RULE_SETUP
#line 198 "re_lexer.l"
{
  yylval->integer = yytext[0];
  return _CHAR_;
}
	YY_BREAK
case 8:
YY_RULE_SETUP
#line 207 "re_lexer.l"
{
  return _WORD_CHAR_;
}
	YY_BREAK
case 9:
YY_RULE_SETUP
#line 212 "re_lexer.l"
{
  return _NON_WORD_CHAR_;
}
	YY_BREAK
case 10:
YY_RULE_SETUP
#line 217 "re_lexer.l"
{
  return _SPACE_;
}
	YY_BREAK
case 11:
YY_RULE_SETUP
#line 222 "re_lexer.l"
{
  return _NON_SPACE_;
}
	YY_BREAK
case 12:
YY_RULE_SETUP
#line 227 "re_lexer.l"
{
  return _DIGIT_;
}
	YY_BREAK
case 13:
YY_RULE_SETUP
#line 232 "re_lexer.l"
{
  return _NON_DIGIT_;
}
	YY_BREAK
case 14:
YY_RULE_SETUP
#line 237 "re_lexer.l"
{
  return _WORD_BOUNDARY_;
}
	YY_BREAK
case 15:
YY_RULE_SETUP
#line 241 "re_lexer.l"
{
  return _NON_WORD_BOUNDARY_;
}
	YY_BREAK
case 16:
YY_RULE_SETUP
#line 246 "re_lexer.l"
{
  yyerror(yyscanner, lex_env, "backreferences are not allowed");
  yyterminate();
}
	YY_BREAK
case 17:
YY_RULE_SETUP
#line 253 "re_lexer.l"
{
  uint8_t c;
  if (read_escaped_char(yyscanner, &c))
  {
    yylval->integer = c;
    return _CHAR_;
  }
  else
  {
    yyerror(yyscanner, lex_env, "unexpected end of buffer");
    yyterminate();
  }
}
	YY_BREAK
case 18:
YY_RULE_SETUP
#line 270 "re_lexer.l"
{
  int i;
  yylval->class_vector = (uint8_t*) yr_malloc(32);
  memcpy(yylval->class_vector, LEX_ENV->class_vector, 32);
  if (LEX_ENV->negated_class)
  {
    for(i = 0; i < 32; i++)
      yylval->class_vector[i] = ~yylval->class_vector[i];
  }
  BEGIN(INITIAL);
  return _CLASS_;
}
	YY_BREAK
case 19:
YY_RULE_SETUP
#line 291 "re_lexer.l"
{
  uint16_t c;
  uint8_t start = yytext[0];
  uint8_t end = yytext[2];
  if (start == '\\')
  {
    start = escaped_char_value(yytext);
    if (yytext[1] == 'x')
      end = yytext[5];
    else
      end = yytext[3];
  }
  if (end == '\\')
  {
    if (!read_escaped_char(yyscanner, &end))
    {
      yyerror(yyscanner, lex_env, "unexpected end of buffer");
      yyterminate();
    }
  }
  if (end < start)
  {
    yyerror(yyscanner, lex_env, "bad character range");
    yyterminate();
  }
  for (c = start; c <= end; c++)
  {
    LEX_ENV->class_vector[c / 8] |= 1 << c % 8;
  }
}
	YY_BREAK
case 20:
YY_RULE_SETUP
#line 333 "re_lexer.l"
{
  int i;
  for (i = 0; i < 32; i++)
    LEX_ENV->class_vector[i] |= word_chars[i];
}
	YY_BREAK
case 21:
YY_RULE_SETUP
#line 342 "re_lexer.l"
{
  int i;
  for (i = 0; i < 32; i++)
    LEX_ENV->class_vector[i] |= ~word_chars[i];
}
	YY_BREAK
case 22:
YY_RULE_SETUP
#line 351 "re_lexer.l"
{
  LEX_ENV->class_vector[' ' / 8] |= 1 << ' ' % 8;
  LEX_ENV->class_vector['\t' / 8] |= 1 << '\t' % 8;
}
	YY_BREAK
case 23:
YY_RULE_SETUP
#line 358 "re_lexer.l"
{
  int i;
  for (i = 0; i < 32; i++)
  {
    if (i == ' ' / 8)
      LEX_ENV->class_vector[i] |= ~(1 << ' ' % 8);
    else if (i == '\t' / 8)
      LEX_ENV->class_vector[i] |= ~(1 << '\t' % 8);
    else
      LEX_ENV->class_vector[i] = 0xFF;
  }
}
	YY_BREAK
case 24:
YY_RULE_SETUP
#line 374 "re_lexer.l"
{
  char c;
  for (c = '0'; c <= '9'; c++)
    LEX_ENV->class_vector[c / 8] |= 1 << c % 8;
}
	YY_BREAK
case 25:
YY_RULE_SETUP
#line 383 "re_lexer.l"
{
  int i;
  for (i = 0; i < 32; i++)
  {
    if (i == 6)
      continue;
    if (i == 7)
      LEX_ENV->class_vector[i] |= 0xFC;
    else
      LEX_ENV->class_vector[i] = 0xFF;
  }
}
	YY_BREAK
case 26:
YY_RULE_SETUP
#line 403 "re_lexer.l"
{
  uint8_t c;
  if (read_escaped_char(yyscanner, &c))
  {
    LEX_ENV->class_vector[c / 8] |= 1 << c % 8;
  }
  else
  {
    yyerror(yyscanner, lex_env, "unexpected end of buffer");
    yyterminate();
  }
}
	YY_BREAK
case 27:
YY_RULE_SETUP
#line 419 "re_lexer.l"
{
  if (yytext[0] >= 32 && yytext[0] < 127)
  {
    LEX_ENV->class_vector[yytext[0] / 8] |= 1 << yytext[0] % 8;
  }
  else
  {
    yyerror(yyscanner, lex_env, "non-ascii character");
    yyterminate();
  }
}
	YY_BREAK
case YY_STATE_EOF(char_class):
#line 436 "re_lexer.l"
{
  yyerror(yyscanner, lex_env, "missing terminating ] for character class");
  yyterminate();
}
	YY_BREAK
case 28:
YY_RULE_SETUP
#line 445 "re_lexer.l"
{
  if (yytext[0] >= 32 && yytext[0] < 127)
  {
    return yytext[0];
  }
  else
  {
    yyerror(yyscanner, lex_env, "non-ascii character");
    yyterminate();
  }
}
	YY_BREAK
case YY_STATE_EOF(INITIAL):
#line 459 "re_lexer.l"
{
  yyterminate();
}
	YY_BREAK
case 29:
YY_RULE_SETUP
#line 464 "re_lexer.l"
ECHO;
	YY_BREAK
#line 1358 "re_lexer.c"
	case YY_END_OF_BUFFER:
		{
		int yy_amount_of_matched_text = (int) (yy_cp - yyg->yytext_ptr) - 1;
		*yy_cp = yyg->yy_hold_char;
		YY_RESTORE_YY_MORE_OFFSET
		if ( YY_CURRENT_BUFFER_LVALUE->yy_buffer_status == YY_BUFFER_NEW )
			{
			yyg->yy_n_chars = YY_CURRENT_BUFFER_LVALUE->yy_n_chars;
			YY_CURRENT_BUFFER_LVALUE->yy_input_file = yyin;
			YY_CURRENT_BUFFER_LVALUE->yy_buffer_status = YY_BUFFER_NORMAL;
			}
		if ( yyg->yy_c_buf_p <= &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars] )
			{  
			yy_state_type yy_next_state;
			yyg->yy_c_buf_p = yyg->yytext_ptr + yy_amount_of_matched_text;
			yy_current_state = yy_get_previous_state( yyscanner );
			yy_next_state = yy_try_NUL_trans( yy_current_state , yyscanner);
			yy_bp = yyg->yytext_ptr + YY_MORE_ADJ;
			if ( yy_next_state )
				{
				yy_cp = ++yyg->yy_c_buf_p;
				yy_current_state = yy_next_state;
				goto yy_match;
				}
			else
				{
				yy_cp = yyg->yy_last_accepting_cpos;
				yy_current_state = yyg->yy_last_accepting_state;
				goto yy_find_action;
				}
			}
		else switch ( yy_get_next_buffer( yyscanner ) )
			{
			case EOB_ACT_END_OF_FILE:
				{
				yyg->yy_did_buffer_switch_on_eof = 0;
				if ( re_yywrap(yyscanner ) )
					{
					yyg->yy_c_buf_p = yyg->yytext_ptr + YY_MORE_ADJ;
					yy_act = YY_STATE_EOF(YY_START);
					goto do_action;
					}
				else
					{
					if ( ! yyg->yy_did_buffer_switch_on_eof )
						YY_NEW_FILE;
					}
				break;
				}
			case EOB_ACT_CONTINUE_SCAN:
				yyg->yy_c_buf_p =
					yyg->yytext_ptr + yy_amount_of_matched_text;
				yy_current_state = yy_get_previous_state( yyscanner );
				yy_cp = yyg->yy_c_buf_p;
				yy_bp = yyg->yytext_ptr + YY_MORE_ADJ;
				goto yy_match;
			case EOB_ACT_LAST_MATCH:
				yyg->yy_c_buf_p =
				&YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars];
				yy_current_state = yy_get_previous_state( yyscanner );
				yy_cp = yyg->yy_c_buf_p;
				yy_bp = yyg->yytext_ptr + YY_MORE_ADJ;
				goto yy_find_action;
			}
		break;
		}
	default:
		YY_FATAL_ERROR(
			"fatal flex scanner internal error--no action found" );
	}  