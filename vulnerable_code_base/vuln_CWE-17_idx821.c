_gnutls_x509_verify_certificate (const gnutls_x509_crt_t * certificate_list,
				 int clist_size,
				 const gnutls_x509_crt_t * trusted_cas,
				 int tcas_size,
				 const gnutls_x509_crl_t * CRLs,
				 int crls_size, unsigned int flags)
{
  int i = 0, ret;
  unsigned int status = 0, output;
  if (clist_size > 1)
    {
      if (gnutls_x509_crt_check_issuer (certificate_list[clist_size - 1],
					certificate_list[clist_size - 1]) > 0)
	{
	  clist_size--;
	}
    }
  if (!(flags & GNUTLS_VERIFY_DO_NOT_ALLOW_SAME))
    {
      for (i = 0; i < clist_size; i++)
	{
	  int j;
	  for (j = 0; j < tcas_size; j++)
	    {
	      if (check_if_same_cert (certificate_list[i],
				      trusted_cas[j]) == 0)
		{
		  clist_size = i;
		  break;
		}
	    }
	}
    }
  if (clist_size == 0)
    return status;
  ret = _gnutls_verify_certificate2 (certificate_list[clist_size - 1],
				     trusted_cas, tcas_size, flags, &output);
  if (ret == 0)
    {
      gnutls_assert ();
      status |= output;
      status |= GNUTLS_CERT_INVALID;
      return status;
    }
#ifdef ENABLE_PKI
  for (i = 0; i < clist_size; i++)
    {
      ret = gnutls_x509_crt_check_revocation (certificate_list[i],
					      CRLs, crls_size);
      if (ret == 1)
	{			 
	  status |= GNUTLS_CERT_REVOKED;
	  status |= GNUTLS_CERT_INVALID;
	  return status;
	}
    }
#endif
  if (!(flags & GNUTLS_VERIFY_DISABLE_TIME_CHECKS))
    {
      time_t t, now = time (0);
      for (i = 0; i < clist_size; i++)
	{
	  t = gnutls_x509_crt_get_activation_time (certificate_list[i]);
	  if (t == (time_t) - 1 || now < t)
	    {
	      status |= GNUTLS_CERT_NOT_ACTIVATED;
	      status |= GNUTLS_CERT_INVALID;
	      return status;
	    }
	  t = gnutls_x509_crt_get_expiration_time (certificate_list[i]);
	  if (t == (time_t) - 1 || now > t)
	    {
	      status |= GNUTLS_CERT_EXPIRED;
	      status |= GNUTLS_CERT_INVALID;
	      return status;
	    }
	}
    }
  for (i = clist_size - 1; i > 0; i--)
    {
      if (i - 1 < 0)
	break;
      if (!(flags & GNUTLS_VERIFY_ALLOW_ANY_X509_V1_CA_CRT))
	flags &= ~(GNUTLS_VERIFY_ALLOW_X509_V1_CA_CRT);
      if ((ret =
	   _gnutls_verify_certificate2 (certificate_list[i - 1],
					&certificate_list[i], 1, flags,
					NULL)) == 0)
	{
	  status |= GNUTLS_CERT_INVALID;
	  return status;
	}
    }
  return 0;
}