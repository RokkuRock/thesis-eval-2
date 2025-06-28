Perl_my_setenv(pTHX_ const char *nam, const char *val)
{
  dVAR;
#ifdef __amigaos4__
  amigaos4_obtain_environ(__FUNCTION__);
#endif
#ifdef USE_ITHREADS
  if (PL_curinterp == aTHX)
#endif
  {
#ifndef PERL_USE_SAFE_PUTENV
    if (!PL_use_safe_putenv) {
        I32 i;
        const I32 len = strlen(nam);
        int nlen, vlen;
        for (i = 0; environ[i]; i++) {
            if (strnEQ(environ[i],nam,len) && environ[i][len] == '=')
                break;
        }
        if (environ == PL_origenviron) {    
            I32 j;
            I32 max;
            char **tmpenv;
            max = i;
            while (environ[max])
                max++;
            tmpenv = (char**)safesysmalloc((max+2) * sizeof(char*));
            for (j=0; j<max; j++) {          
                const int len = strlen(environ[j]);
                tmpenv[j] = (char*)safesysmalloc((len+1)*sizeof(char));
                Copy(environ[j], tmpenv[j], len+1, char);
            }
            tmpenv[max] = NULL;
            environ = tmpenv;                
        }
        if (!val) {
            safesysfree(environ[i]);
            while (environ[i]) {
                environ[i] = environ[i+1];
                i++;
            }
#ifdef __amigaos4__
            goto my_setenv_out;
#else
            return;
#endif
        }
        if (!environ[i]) {                  
            environ = (char**)safesysrealloc(environ, (i+2) * sizeof(char*));
            environ[i+1] = NULL;     
        }
        else
            safesysfree(environ[i]);
        nlen = strlen(nam);
        vlen = strlen(val);
        environ[i] = (char*)safesysmalloc((nlen+vlen+2) * sizeof(char));
        my_setenv_format(environ[i], nam, nlen, val, vlen);
    } else {
# endif
#   if defined(__CYGWIN__)|| defined(__SYMBIAN32__) || defined(__riscos__) || (defined(__sun) && defined(HAS_UNSETENV)) || defined(PERL_DARWIN)
#       if defined(HAS_UNSETENV)
        if (val == NULL) {
            (void)unsetenv(nam);
        } else {
            (void)setenv(nam, val, 1);
        }
#       else  
        (void)setenv(nam, val, 1);
#       endif  
#   elif defined(HAS_UNSETENV)
        if (val == NULL) {
            if (environ)  
                (void)unsetenv(nam);
        } else {
	    const int nlen = strlen(nam);
	    const int vlen = strlen(val);
	    char * const new_env =
                (char*)safesysmalloc((nlen + vlen + 2) * sizeof(char));
            my_setenv_format(new_env, nam, nlen, val, vlen);
            (void)putenv(new_env);
        }
#   else  
        char *new_env;
	const int nlen = strlen(nam);
	int vlen;
        if (!val) {
	   val = "";
        }
        vlen = strlen(val);
        new_env = (char*)safesysmalloc((nlen + vlen + 2) * sizeof(char));
        my_setenv_format(new_env, nam, nlen, val, vlen);
        (void)putenv(new_env);
#   endif  
#ifndef PERL_USE_SAFE_PUTENV
    }
#endif
  }
#ifdef __amigaos4__
my_setenv_out:
  amigaos4_release_environ(__FUNCTION__);
#endif
}