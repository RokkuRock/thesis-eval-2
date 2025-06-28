int
yyparse (void *yyscanner, HEX_LEX_ENVIRONMENT *lex_env)
{
int yychar;
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);
    int yynerrs;
    int yystate;
    int yyerrstatus;
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;
    YYSIZE_T yystacksize;
  int yyn;
  int yyresult;
  int yytoken = 0;
  YYSTYPE yyval;
#if YYERROR_VERBOSE
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif
#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))
  int yylen = 0;
  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;
  YYDPRINTF ((stderr, "Starting parse\n"));
  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;  
  goto yysetstate;
 yynewstate:
  yyssp++;
 yysetstate:
  *yyssp = yystate;
  if (yyss + yystacksize - 1 <= yyssp)
    {
      YYSIZE_T yysize = yyssp - yyss + 1;
#ifdef yyoverflow
      {
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
#else  
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;
      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif  
      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));
      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  if (yystate == YYFINAL)
    YYACCEPT;
  goto yybackup;
yybackup:
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, yyscanner, lex_env);
    }
  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  if (yyerrstatus)
    yyerrstatus--;
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yychar = YYEMPTY;
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  goto yynewstate;
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;
yyreduce:
  yylen = yyr2[yyn];
  yyval = yyvsp[1-yylen];
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 106 "hex_grammar.y"  
    {
        RE_AST* re_ast = yyget_extra(yyscanner);
        re_ast->root_node = (yyvsp[-1].re_node);
      }
#line 1330 "hex_grammar.c"  
    break;
  case 3:
#line 115 "hex_grammar.y"  
    {
        (yyval.re_node) = (yyvsp[0].re_node);
      }
#line 1338 "hex_grammar.c"  
    break;
  case 4:
#line 119 "hex_grammar.y"  
    {
        (yyval.re_node) = yr_re_node_create(RE_NODE_CONCAT, (yyvsp[-1].re_node), (yyvsp[0].re_node));
        DESTROY_NODE_IF((yyval.re_node) == NULL, (yyvsp[-1].re_node));
        DESTROY_NODE_IF((yyval.re_node) == NULL, (yyvsp[0].re_node));
        ERROR_IF((yyval.re_node) == NULL, ERROR_INSUFFICIENT_MEMORY);
      }
#line 1351 "hex_grammar.c"  
    break;
  case 5:
#line 128 "hex_grammar.y"  
    {
        RE_NODE* new_concat;
        RE_NODE* leftmost_concat = NULL;
        RE_NODE* leftmost_node = (yyvsp[-1].re_node);
        (yyval.re_node) = NULL;
        while (leftmost_node->type == RE_NODE_CONCAT)
        {
          leftmost_concat = leftmost_node;
          leftmost_node = leftmost_node->left;
        }
        new_concat = yr_re_node_create(
            RE_NODE_CONCAT, (yyvsp[-2].re_node), leftmost_node);
        if (new_concat != NULL)
        {
          if (leftmost_concat != NULL)
          {
            leftmost_concat->left = new_concat;
            (yyval.re_node) = yr_re_node_create(RE_NODE_CONCAT, (yyvsp[-1].re_node), (yyvsp[0].re_node));
          }
          else
          {
            (yyval.re_node) = yr_re_node_create(RE_NODE_CONCAT, new_concat, (yyvsp[0].re_node));
          }
        }
        DESTROY_NODE_IF((yyval.re_node) == NULL, (yyvsp[-2].re_node));
        DESTROY_NODE_IF((yyval.re_node) == NULL, (yyvsp[-1].re_node));
        DESTROY_NODE_IF((yyval.re_node) == NULL, (yyvsp[0].re_node));
        ERROR_IF((yyval.re_node) == NULL, ERROR_INSUFFICIENT_MEMORY);
      }
#line 1413 "hex_grammar.c"  
    break;
  case 6:
#line 190 "hex_grammar.y"  
    {
        (yyval.re_node) = (yyvsp[0].re_node);
      }
#line 1421 "hex_grammar.c"  
    break;
  case 7:
#line 194 "hex_grammar.y"  
    {
        (yyval.re_node) = yr_re_node_create(RE_NODE_CONCAT, (yyvsp[-1].re_node), (yyvsp[0].re_node));
        DESTROY_NODE_IF((yyval.re_node) == NULL, (yyvsp[-1].re_node));
        DESTROY_NODE_IF((yyval.re_node) == NULL, (yyvsp[0].re_node));
        ERROR_IF((yyval.re_node) == NULL, ERROR_INSUFFICIENT_MEMORY);
      }
#line 1434 "hex_grammar.c"  
    break;
  case 8:
#line 207 "hex_grammar.y"  
    {
        (yyval.re_node) = (yyvsp[0].re_node);
      }
#line 1442 "hex_grammar.c"  
    break;
  case 9:
#line 211 "hex_grammar.y"  
    {
        (yyval.re_node) = (yyvsp[0].re_node);
        (yyval.re_node)->greedy = FALSE;
      }
#line 1451 "hex_grammar.c"  
    break;
  case 10:
#line 220 "hex_grammar.y"  
    {
        lex_env->token_count++;
        if (lex_env->token_count > MAX_HEX_STRING_TOKENS)
        {
          yr_re_node_destroy((yyvsp[0].re_node));
          yyerror(yyscanner, lex_env, "string too long");
          YYABORT;
        }
        (yyval.re_node) = (yyvsp[0].re_node);
      }
#line 1468 "hex_grammar.c"  
    break;
  case 11:
#line 233 "hex_grammar.y"  
    {
        lex_env->inside_or++;
      }
#line 1476 "hex_grammar.c"  
    break;
  case 12:
#line 237 "hex_grammar.y"  
    {
        (yyval.re_node) = (yyvsp[-1].re_node);
        lex_env->inside_or--;
      }
#line 1485 "hex_grammar.c"  
    break;
  case 13:
#line 246 "hex_grammar.y"  
    {
        if ((yyvsp[-1].integer) <= 0)
        {
          yyerror(yyscanner, lex_env, "invalid jump length");
          YYABORT;
        }
        if (lex_env->inside_or && (yyvsp[-1].integer) > STRING_CHAINING_THRESHOLD)
        {
          yyerror(yyscanner, lex_env, "jumps over "
              STR(STRING_CHAINING_THRESHOLD)
              " now allowed inside alternation (|)");
          YYABORT;
        }
        (yyval.re_node) = yr_re_node_create(RE_NODE_RANGE_ANY, NULL, NULL);
        ERROR_IF((yyval.re_node) == NULL, ERROR_INSUFFICIENT_MEMORY);
        (yyval.re_node)->start = (int) (yyvsp[-1].integer);
        (yyval.re_node)->end = (int) (yyvsp[-1].integer);
      }
#line 1512 "hex_grammar.c"  
    break;
  case 14:
#line 269 "hex_grammar.y"  
    {
        if (lex_env->inside_or &&
            ((yyvsp[-3].integer) > STRING_CHAINING_THRESHOLD ||
             (yyvsp[-1].integer) > STRING_CHAINING_THRESHOLD) )
        {
          yyerror(yyscanner, lex_env, "jumps over "
              STR(STRING_CHAINING_THRESHOLD)
              " now allowed inside alternation (|)");
          YYABORT;
        }
        if ((yyvsp[-3].integer) < 0 || (yyvsp[-1].integer) < 0)
        {
          yyerror(yyscanner, lex_env, "invalid negative jump length");
          YYABORT;
        }
        if ((yyvsp[-3].integer) > (yyvsp[-1].integer))
        {
          yyerror(yyscanner, lex_env, "invalid jump range");
          YYABORT;
        }
        (yyval.re_node) = yr_re_node_create(RE_NODE_RANGE_ANY, NULL, NULL);
        ERROR_IF((yyval.re_node) == NULL, ERROR_INSUFFICIENT_MEMORY);
        (yyval.re_node)->start = (int) (yyvsp[-3].integer);
        (yyval.re_node)->end = (int) (yyvsp[-1].integer);
      }
#line 1548 "hex_grammar.c"  
    break;
  case 15:
#line 301 "hex_grammar.y"  
    {
        if (lex_env->inside_or)
        {
          yyerror(yyscanner, lex_env,
              "unbounded jumps not allowed inside alternation (|)");
          YYABORT;
        }
        if ((yyvsp[-2].integer) < 0)
        {
          yyerror(yyscanner, lex_env, "invalid negative jump length");
          YYABORT;
        }
        (yyval.re_node) = yr_re_node_create(RE_NODE_RANGE_ANY, NULL, NULL);
        ERROR_IF((yyval.re_node) == NULL, ERROR_INSUFFICIENT_MEMORY);
        (yyval.re_node)->start = (int) (yyvsp[-2].integer);
        (yyval.re_node)->end = INT_MAX;
      }
#line 1574 "hex_grammar.c"  
    break;
  case 16:
#line 323 "hex_grammar.y"  
    {
        if (lex_env->inside_or)
        {
          yyerror(yyscanner, lex_env,
              "unbounded jumps not allowed inside alternation (|)");
          YYABORT;
        }
        (yyval.re_node) = yr_re_node_create(RE_NODE_RANGE_ANY, NULL, NULL);
        ERROR_IF((yyval.re_node) == NULL, ERROR_INSUFFICIENT_MEMORY);
        (yyval.re_node)->start = 0;
        (yyval.re_node)->end = INT_MAX;
      }
#line 1594 "hex_grammar.c"  
    break;
  case 17:
#line 343 "hex_grammar.y"  
    {
          (yyval.re_node) = (yyvsp[0].re_node);
      }
#line 1602 "hex_grammar.c"  
    break;
  case 18:
#line 347 "hex_grammar.y"  
    {
        mark_as_not_fast_regexp();
        (yyval.re_node) = yr_re_node_create(RE_NODE_ALT, (yyvsp[-2].re_node), (yyvsp[0].re_node));
        DESTROY_NODE_IF((yyval.re_node) == NULL, (yyvsp[-2].re_node));
        DESTROY_NODE_IF((yyval.re_node) == NULL, (yyvsp[0].re_node));
        ERROR_IF((yyval.re_node) == NULL, ERROR_INSUFFICIENT_MEMORY);
      }
#line 1617 "hex_grammar.c"  
    break;
  case 19:
#line 361 "hex_grammar.y"  
    {
        (yyval.re_node) = yr_re_node_create(RE_NODE_LITERAL, NULL, NULL);
        ERROR_IF((yyval.re_node) == NULL, ERROR_INSUFFICIENT_MEMORY);
        (yyval.re_node)->value = (int) (yyvsp[0].integer);
      }
#line 1629 "hex_grammar.c"  
    break;
  case 20:
#line 369 "hex_grammar.y"  
    {
        uint8_t mask = (uint8_t) ((yyvsp[0].integer) >> 8);
        if (mask == 0x00)
        {
          (yyval.re_node) = yr_re_node_create(RE_NODE_ANY, NULL, NULL);
          ERROR_IF((yyval.re_node) == NULL, ERROR_INSUFFICIENT_MEMORY);
        }
        else
        {
          (yyval.re_node) = yr_re_node_create(RE_NODE_MASKED_LITERAL, NULL, NULL);
          ERROR_IF((yyval.re_node) == NULL, ERROR_INSUFFICIENT_MEMORY);
          (yyval.re_node)->value = (yyvsp[0].integer) & 0xFF;
          (yyval.re_node)->mask = mask;
        }
      }
#line 1653 "hex_grammar.c"  
    break;
#line 1657 "hex_grammar.c"  
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  *++yyvsp = yyval;
  yyn = yyr1[yyn];
  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];
  goto yynewstate;
yyerrlab:
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (yyscanner, lex_env, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yyscanner, lex_env, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }
  if (yyerrstatus == 3)
    {
      if (yychar <= YYEOF)
        {
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, yyscanner, lex_env);
          yychar = YYEMPTY;
        }
    }
  goto yyerrlab1;
yyerrorlab:
  if (  0)
     goto yyerrorlab;
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;
yyerrlab1:
  yyerrstatus = 3;       
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }
      if (yyssp == yyss)
        YYABORT;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yyscanner, lex_env);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);
  yystate = yyn;
  goto yynewstate;
yyacceptlab:
  yyresult = 0;
  goto yyreturn;
yyabortlab:
  yyresult = 1;
  goto yyreturn;
#if !defined yyoverflow || YYERROR_VERBOSE
yyexhaustedlab:
  yyerror (yyscanner, lex_env, YY_("memory exhausted"));
  yyresult = 2;
#endif
yyreturn:
  if (yychar != YYEMPTY)
    {
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, yyscanner, lex_env);
    }
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yyscanner, lex_env);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif