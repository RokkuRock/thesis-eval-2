#else
    static int input  (yyscan_t yyscanner)
#endif
{
	int c;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
	*yyg->yy_c_buf_p = yyg->yy_hold_char;
	if ( *yyg->yy_c_buf_p == YY_END_OF_BUFFER_CHAR )
		{
		if ( yyg->yy_c_buf_p < &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[yyg->yy_n_chars] )
			*yyg->yy_c_buf_p = '\0';
		else
			{  
			yy_size_t offset = yyg->yy_c_buf_p - yyg->yytext_ptr;
			++yyg->yy_c_buf_p;
			switch ( yy_get_next_buffer( yyscanner ) )
				{
				case EOB_ACT_LAST_MATCH:
					re_yyrestart(yyin ,yyscanner);
				case EOB_ACT_END_OF_FILE:
					{
					if ( re_yywrap(yyscanner ) )
						return EOF;
					if ( ! yyg->yy_did_buffer_switch_on_eof )
						YY_NEW_FILE;
#ifdef __cplusplus
					return yyinput(yyscanner);
#else
					return input(yyscanner);
#endif
					}
				case EOB_ACT_CONTINUE_SCAN:
					yyg->yy_c_buf_p = yyg->yytext_ptr + offset;
					break;
				}
			}
		}
	c = *(unsigned char *) yyg->yy_c_buf_p;	 
	*yyg->yy_c_buf_p = '\0';	 
	yyg->yy_hold_char = *++yyg->yy_c_buf_p;
	if ( c == '\n' )
    do{ yylineno++;
        yycolumn=0;
    }while(0)
;
	return c;