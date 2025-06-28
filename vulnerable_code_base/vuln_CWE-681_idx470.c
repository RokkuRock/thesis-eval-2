static int lookupName(
  Parse *pParse,        
  const char *zDb,      
  const char *zTab,     
  const char *zCol,     
  NameContext *pNC,     
  Expr *pExpr           
){
  int i, j;                          
  int cnt = 0;                       
  int cntTab = 0;                    
  int nSubquery = 0;                 
  sqlite3 *db = pParse->db;          
  struct SrcList_item *pItem;        
  struct SrcList_item *pMatch = 0;   
  NameContext *pTopNC = pNC;         
  Schema *pSchema = 0;               
  int eNewExprOp = TK_COLUMN;        
  Table *pTab = 0;                   
  Column *pCol;                      
  assert( pNC );      
  assert( zCol );     
  assert( !ExprHasProperty(pExpr, EP_TokenOnly|EP_Reduced) );
  pExpr->iTable = -1;
  ExprSetVVAProperty(pExpr, EP_NoReduce);
  if( zDb ){
    testcase( pNC->ncFlags & NC_PartIdx );
    testcase( pNC->ncFlags & NC_IsCheck );
    if( (pNC->ncFlags & (NC_PartIdx|NC_IsCheck))!=0 ){
      zDb = 0;
    }else{
      for(i=0; i<db->nDb; i++){
        assert( db->aDb[i].zDbSName );
        if( sqlite3StrICmp(db->aDb[i].zDbSName,zDb)==0 ){
          pSchema = db->aDb[i].pSchema;
          break;
        }
      }
    }
  }
  assert( pNC && cnt==0 );
  do{
    ExprList *pEList;
    SrcList *pSrcList = pNC->pSrcList;
    if( pSrcList ){
      for(i=0, pItem=pSrcList->a; i<pSrcList->nSrc; i++, pItem++){
        pTab = pItem->pTab;
        assert( pTab!=0 && pTab->zName!=0 );
        assert( pTab->nCol>0 );
        if( pItem->pSelect && (pItem->pSelect->selFlags & SF_NestedFrom)!=0 ){
          int hit = 0;
          pEList = pItem->pSelect->pEList;
          for(j=0; j<pEList->nExpr; j++){
            if( sqlite3MatchSpanName(pEList->a[j].zSpan, zCol, zTab, zDb) ){
              cnt++;
              cntTab = 2;
              pMatch = pItem;
              pExpr->iColumn = j;
              hit = 1;
            }
          }
          if( hit || zTab==0 ) continue;
        }
        if( zDb && pTab->pSchema!=pSchema ){
          continue;
        }
        if( zTab ){
          const char *zTabName = pItem->zAlias ? pItem->zAlias : pTab->zName;
          assert( zTabName!=0 );
          if( sqlite3StrICmp(zTabName, zTab)!=0 ){
            continue;
          }
          if( IN_RENAME_OBJECT && pItem->zAlias ){
            sqlite3RenameTokenRemap(pParse, 0, (void*)&pExpr->y.pTab);
          }
        }
        if( 0==(cntTab++) ){
          pMatch = pItem;
        }
        for(j=0, pCol=pTab->aCol; j<pTab->nCol; j++, pCol++){
          if( sqlite3StrICmp(pCol->zName, zCol)==0 ){
            if( cnt==1 ){
              if( pItem->fg.jointype & JT_NATURAL ) continue;
              if( nameInUsingClause(pItem->pUsing, zCol) ) continue;
            }
            cnt++;
            pMatch = pItem;
            pExpr->iColumn = j==pTab->iPKey ? -1 : (i16)j;
            break;
          }
        }
      }
      if( pMatch ){
        pExpr->iTable = pMatch->iCursor;
        pExpr->y.pTab = pMatch->pTab;
        assert( (pMatch->fg.jointype & JT_RIGHT)==0 );
        if( (pMatch->fg.jointype & JT_LEFT)!=0 ){
          ExprSetProperty(pExpr, EP_CanBeNull);
        }
        pSchema = pExpr->y.pTab->pSchema;
      }
    }  
#if !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT)
    if( zDb==0 && zTab!=0 && cntTab==0 ){
      pTab = 0;
#ifndef SQLITE_OMIT_TRIGGER
      if( pParse->pTriggerTab!=0 ){
        int op = pParse->eTriggerOp;
        assert( op==TK_DELETE || op==TK_UPDATE || op==TK_INSERT );
        if( op!=TK_DELETE && sqlite3StrICmp("new",zTab) == 0 ){
          pExpr->iTable = 1;
          pTab = pParse->pTriggerTab;
        }else if( op!=TK_INSERT && sqlite3StrICmp("old",zTab)==0 ){
          pExpr->iTable = 0;
          pTab = pParse->pTriggerTab;
        }
      }
#endif  
#ifndef SQLITE_OMIT_UPSERT
      if( (pNC->ncFlags & NC_UUpsert)!=0 ){
        Upsert *pUpsert = pNC->uNC.pUpsert;
        if( pUpsert && sqlite3StrICmp("excluded",zTab)==0 ){
          pTab = pUpsert->pUpsertSrc->a[0].pTab;
          pExpr->iTable = 2;
        }
      }
#endif  
      if( pTab ){ 
        int iCol;
        pSchema = pTab->pSchema;
        cntTab++;
        for(iCol=0, pCol=pTab->aCol; iCol<pTab->nCol; iCol++, pCol++){
          if( sqlite3StrICmp(pCol->zName, zCol)==0 ){
            if( iCol==pTab->iPKey ){
              iCol = -1;
            }
            break;
          }
        }
        if( iCol>=pTab->nCol && sqlite3IsRowid(zCol) && VisibleRowid(pTab) ){
          iCol = -1;
        }
        if( iCol<pTab->nCol ){
          cnt++;
#ifndef SQLITE_OMIT_UPSERT
          if( pExpr->iTable==2 ){
            testcase( iCol==(-1) );
            if( IN_RENAME_OBJECT ){
              pExpr->iColumn = iCol;
              pExpr->y.pTab = pTab;
              eNewExprOp = TK_COLUMN;
            }else{
              pExpr->iTable = pNC->uNC.pUpsert->regData + iCol;
              eNewExprOp = TK_REGISTER;
              ExprSetProperty(pExpr, EP_Alias);
            }
          }else
#endif  
          {
#ifndef SQLITE_OMIT_TRIGGER
            if( iCol<0 ){
              pExpr->affExpr = SQLITE_AFF_INTEGER;
            }else if( pExpr->iTable==0 ){
              testcase( iCol==31 );
              testcase( iCol==32 );
              pParse->oldmask |= (iCol>=32 ? 0xffffffff : (((u32)1)<<iCol));
            }else{
              testcase( iCol==31 );
              testcase( iCol==32 );
              pParse->newmask |= (iCol>=32 ? 0xffffffff : (((u32)1)<<iCol));
            }
            pExpr->y.pTab = pTab;
            pExpr->iColumn = (i16)iCol;
            eNewExprOp = TK_TRIGGER;
#endif  
          }
        }
      }
    }
#endif  
    if( cnt==0
     && cntTab==1
     && pMatch
     && (pNC->ncFlags & (NC_IdxExpr|NC_GenCol))==0
     && sqlite3IsRowid(zCol)
     && VisibleRowid(pMatch->pTab)
    ){
      cnt = 1;
      pExpr->iColumn = -1;
      pExpr->affExpr = SQLITE_AFF_INTEGER;
    }
    if( (pNC->ncFlags & NC_UEList)!=0
     && cnt==0
     && zTab==0
    ){
      pEList = pNC->uNC.pEList;
      assert( pEList!=0 );
      for(j=0; j<pEList->nExpr; j++){
        char *zAs = pEList->a[j].zName;
        if( zAs!=0 && sqlite3StrICmp(zAs, zCol)==0 ){
          Expr *pOrig;
          assert( pExpr->pLeft==0 && pExpr->pRight==0 );
          assert( pExpr->x.pList==0 );
          assert( pExpr->x.pSelect==0 );
          pOrig = pEList->a[j].pExpr;
          if( (pNC->ncFlags&NC_AllowAgg)==0 && ExprHasProperty(pOrig, EP_Agg) ){
            sqlite3ErrorMsg(pParse, "misuse of aliased aggregate %s", zAs);
            return WRC_Abort;
          }
          if( (pNC->ncFlags&NC_AllowWin)==0 && ExprHasProperty(pOrig, EP_Win) ){
            sqlite3ErrorMsg(pParse, "misuse of aliased window function %s",zAs);
            return WRC_Abort;
          }
          if( sqlite3ExprVectorSize(pOrig)!=1 ){
            sqlite3ErrorMsg(pParse, "row value misused");
            return WRC_Abort;
          }
          resolveAlias(pParse, pEList, j, pExpr, "", nSubquery);
          cnt = 1;
          pMatch = 0;
          assert( zTab==0 && zDb==0 );
          if( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (void*)pExpr);
          }
          goto lookupname_end;
        }
      } 
    }
    if( cnt ) break;
    pNC = pNC->pNext;
    nSubquery++;
  }while( pNC );
  if( cnt==0 && zTab==0 ){
    assert( pExpr->op==TK_ID );
    if( ExprHasProperty(pExpr,EP_DblQuoted)
     && areDoubleQuotedStringsEnabled(db, pTopNC)
    ){
      sqlite3_log(SQLITE_WARNING,
        "double-quoted string literal: \"%w\"", zCol);
#ifdef SQLITE_ENABLE_NORMALIZE
      sqlite3VdbeAddDblquoteStr(db, pParse->pVdbe, zCol);
#endif
      pExpr->op = TK_STRING;
      pExpr->y.pTab = 0;
      return WRC_Prune;
    }
    if( sqlite3ExprIdToTrueFalse(pExpr) ){
      return WRC_Prune;
    }
  }
  if( cnt!=1 ){
    const char *zErr;
    zErr = cnt==0 ? "no such column" : "ambiguous column name";
    if( zDb ){
      sqlite3ErrorMsg(pParse, "%s: %s.%s.%s", zErr, zDb, zTab, zCol);
    }else if( zTab ){
      sqlite3ErrorMsg(pParse, "%s: %s.%s", zErr, zTab, zCol);
    }else{
      sqlite3ErrorMsg(pParse, "%s: %s", zErr, zCol);
    }
    pParse->checkSchema = 1;
    pTopNC->nErr++;
  }
  if( pExpr->iColumn>=0 && pMatch!=0 ){
    int n = pExpr->iColumn;
    testcase( n==BMS-1 );
    if( n>=BMS ){
      n = BMS-1;
    }
    assert( pMatch->iCursor==pExpr->iTable );
    pMatch->colUsed |= ((Bitmask)1)<<n;
  }
  sqlite3ExprDelete(db, pExpr->pLeft);
  pExpr->pLeft = 0;
  sqlite3ExprDelete(db, pExpr->pRight);
  pExpr->pRight = 0;
  pExpr->op = eNewExprOp;
  ExprSetProperty(pExpr, EP_Leaf);
lookupname_end:
  if( cnt==1 ){
    assert( pNC!=0 );
    if( !ExprHasProperty(pExpr, EP_Alias) ){
      sqlite3AuthRead(pParse, pExpr, pSchema, pNC->pSrcList);
    }
    for(;;){
      assert( pTopNC!=0 );
      pTopNC->nRef++;
      if( pTopNC==pNC ) break;
      pTopNC = pTopNC->pNext;
    }
    return WRC_Prune;
  } else {
    return WRC_Abort;
  }
}