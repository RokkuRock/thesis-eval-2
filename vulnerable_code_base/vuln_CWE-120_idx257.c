static void dbOutputHexBlob(Jsi_DString *dStr, const void *pBlob, int nBlob){
  int i;
  char out[100], *zBlob = (char *)pBlob;
  Jsi_DSAppend(dStr, "X'", NULL);
  for(i=0; i<nBlob; i++){ snprintf(out, sizeof(out),"%02x",zBlob[i]&0xff);Jsi_DSAppend(dStr, out, NULL); }
  Jsi_DSAppend(dStr, "'", NULL);
}