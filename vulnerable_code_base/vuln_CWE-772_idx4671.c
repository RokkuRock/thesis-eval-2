processBatchMultiRuleset(batch_t *pBatch)
{
	ruleset_t *currRuleset;
	batch_t snglRuleBatch;
	int i;
	int iStart;	 
	int iNew;	 
	DEFiRet;
	CHKiRet(batchInit(&snglRuleBatch, pBatch->nElem));
	snglRuleBatch.pbShutdownImmediate = pBatch->pbShutdownImmediate;
	while(1) {  
		for(iStart = 0 ; iStart < pBatch->nElem && pBatch->pElem[iStart].state == BATCH_STATE_DISC ; ++iStart)
			 ;
		if(iStart == pBatch->nElem)
			FINALIZE;  
		currRuleset = batchElemGetRuleset(pBatch, iStart);
		iNew = 0;
		for(i = iStart ; i < pBatch->nElem ; ++i) {
			if(batchElemGetRuleset(pBatch, i) == currRuleset) {
				batchCopyElem(&(snglRuleBatch.pElem[iNew++]), &(pBatch->pElem[i]));
				pBatch->pElem[i].state = BATCH_STATE_DISC;
			}
		}
		snglRuleBatch.nElem = iNew;  
		batchSetSingleRuleset(&snglRuleBatch, 1);
		processBatch(&snglRuleBatch);
	}
	batchFree(&snglRuleBatch);
finalize_it:
	RETiRet;
}