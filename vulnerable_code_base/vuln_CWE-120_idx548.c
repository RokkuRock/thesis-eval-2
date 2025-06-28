static int ScaKwdTab(GmfMshSct *msh)
{
   int      KwdCod, c;
   int64_t  NexPos, EndPos, LstPos;
   char     str[ GmfStrSiz ];
   if(msh->typ & Asc)
   {
      while(fscanf(msh->hdl, "%s", str) != EOF)
      {
         if(isalpha(str[0]))
         {
            for(KwdCod=1; KwdCod<= GmfMaxKwd; KwdCod++)
               if(!strcmp(str, GmfKwdFmt[ KwdCod ][0]))
               {
                  ScaKwdHdr(msh, KwdCod);
                  break;
               }
         }
         else if(str[0] == '#')
            while((c = fgetc(msh->hdl)) != '\n' && c != EOF);
      }
   }
   else
   {
      EndPos = GetFilSiz(msh);
      LstPos = -1;
      do
      {
         ScaWrd(msh, ( char *)&KwdCod);
         NexPos = GetPos(msh);
         if(NexPos > EndPos)
            longjmp(msh->err, -24);
         if(NexPos && (NexPos <= LstPos))
            longjmp(msh->err, -30);
         LstPos = NexPos;
         if( (KwdCod >= 1) && (KwdCod <= GmfMaxKwd) )
            ScaKwdHdr(msh, KwdCod);
         if(NexPos && !(SetFilPos(msh, NexPos)))
            longjmp(msh->err, -25);
      }while(NexPos && (KwdCod != GmfEnd));
   }
   return(1);
}