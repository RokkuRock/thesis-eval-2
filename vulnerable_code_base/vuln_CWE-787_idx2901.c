host_name_lookup(void)
{
int old_pool, rc;
int sep = 0;
uschar *save_hostname;
uschar **aliases;
uschar *ordername;
const uschar *list = host_lookup_order;
dns_answer * dnsa = store_get_dns_answer();
dns_scan dnss;
sender_host_dnssec = host_lookup_deferred = host_lookup_failed = FALSE;
HDEBUG(D_host_lookup)
  debug_printf("looking up host name for %s\n", sender_host_address);
if (f.running_in_test_harness &&
    Ustrcmp(sender_host_address, "99.99.99.99") == 0)
  {
  HDEBUG(D_host_lookup)
    debug_printf("Test harness: host name lookup returns DEFER\n");
  host_lookup_deferred = TRUE;
  return DEFER;
  }
while ((ordername = string_nextinlist(&list, &sep, NULL, 0)))
  {
  if (strcmpic(ordername, US"bydns") == 0)
    {
    uschar * name = dns_build_reverse(sender_host_address);
    dns_init(FALSE, FALSE, FALSE);     
    rc = dns_lookup_timerwrap(dnsa, name, T_PTR, NULL);
    if (rc == DNS_SUCCEED)
      {
      uschar **aptr = NULL;
      int ssize = 264;
      int count = 0;
      int old_pool = store_pool;
      sender_host_dnssec = dns_is_secure(dnsa);
      DEBUG(D_dns)
        debug_printf("Reverse DNS security status: %s\n",
            sender_host_dnssec ? "DNSSEC verified (AD)" : "unverified");
      store_pool = POOL_PERM;         
      for (dns_record * rr = dns_next_rr(dnsa, &dnss, RESET_ANSWERS);
           rr;
           rr = dns_next_rr(dnsa, &dnss, RESET_NEXT)) if (rr->type == T_PTR)
	count++;
      aptr = sender_host_aliases = store_get(count * sizeof(uschar *), FALSE);
      for (dns_record * rr = dns_next_rr(dnsa, &dnss, RESET_ANSWERS);
           rr;
           rr = dns_next_rr(dnsa, &dnss, RESET_NEXT)) if (rr->type == T_PTR)
        {
        uschar * s = store_get(ssize, TRUE);	 
        if (dn_expand(dnsa->answer, dnsa->answer + dnsa->answerlen,
             US (rr->data), (DN_EXPAND_ARG4_TYPE)(s), ssize) < 0)
          {
          log_write(0, LOG_MAIN, "host name alias list truncated for %s",
            sender_host_address);
          break;
          }
        store_release_above(s + Ustrlen(s) + 1);
        if (!s[0])
          {
          HDEBUG(D_host_lookup) debug_printf("IP address lookup yielded an "
            "empty name: treated as non-existent host name\n");
          continue;
          }
        if (!sender_host_name) sender_host_name = s;
	else *aptr++ = s;
        while (*s) { *s = tolower(*s); s++; }
        }
      *aptr = NULL;             
      store_pool = old_pool;    
      if (sender_host_name) break;
      }
    if (rc == DNS_AGAIN)
      {
      HDEBUG(D_host_lookup)
        debug_printf("IP address PTR lookup gave temporary error\n");
      host_lookup_deferred = TRUE;
      return DEFER;
      }
    }
  else if (strcmpic(ordername, US"byaddr") == 0)
    {
    HDEBUG(D_host_lookup)
      debug_printf("IP address lookup using gethostbyaddr()\n");
    rc = host_name_lookup_byaddr();
    if (rc == DEFER)
      {
      host_lookup_deferred = TRUE;
      return rc;                        
      }
    if (rc == OK) break;                
    }
  }       
if (!sender_host_name)
  {
  if (host_checking || !f.log_testing_mode)
    log_write(L_host_lookup_failed, LOG_MAIN, "no host name found for IP "
      "address %s", sender_host_address);
  host_lookup_msg = US" (failed to find host name from IP address)";
  host_lookup_failed = TRUE;
  return FAIL;
  }
HDEBUG(D_host_lookup)
  {
  uschar **aliases = sender_host_aliases;
  debug_printf("IP address lookup yielded \"%s\"\n", sender_host_name);
  while (*aliases != NULL) debug_printf("  alias \"%s\"\n", *aliases++);
  }
save_hostname = sender_host_name;    
aliases = sender_host_aliases;
for (uschar * hname = sender_host_name; hname; hname = *aliases++)
  {
  int rc;
  BOOL ok = FALSE;
  host_item h = { .next = NULL, .name = hname, .mx = MX_NONE, .address = NULL };
  dnssec_domains d =
    { .request = sender_host_dnssec ? US"*" : NULL, .require = NULL };
  if (  (rc = host_find_bydns(&h, NULL, HOST_FIND_BY_A | HOST_FIND_BY_AAAA,
	  NULL, NULL, NULL, &d, NULL, NULL)) == HOST_FOUND
     || rc == HOST_FOUND_LOCAL
     )
    {
    HDEBUG(D_host_lookup) debug_printf("checking addresses for %s\n", hname);
    DEBUG(D_dns) debug_printf("Forward DNS security status: %s\n",
	  h.dnssec == DS_YES ? "DNSSEC verified (AD)" : "unverified");
    if (h.dnssec != DS_YES) sender_host_dnssec = FALSE;
    for (host_item * hh = &h; hh; hh = hh->next)
      if (host_is_in_net(hh->address, sender_host_address, 0))
        {
        HDEBUG(D_host_lookup) debug_printf("  %s OK\n", hh->address);
        ok = TRUE;
        break;
        }
      else
        HDEBUG(D_host_lookup) debug_printf("  %s\n", hh->address);
    if (!ok) HDEBUG(D_host_lookup)
      debug_printf("no IP address for %s matched %s\n", hname,
        sender_host_address);
    }
  else if (rc == HOST_FIND_AGAIN)
    {
    HDEBUG(D_host_lookup) debug_printf("temporary error for host name lookup\n");
    host_lookup_deferred = TRUE;
    sender_host_name = NULL;
    return DEFER;
    }
  else
    HDEBUG(D_host_lookup) debug_printf("no IP addresses found for %s\n", hname);
  if (!ok)
    {
    if (hname == sender_host_name) sender_host_name = NULL; else
      {
      uschar **a;                               
      a = --aliases;                            
      while (*a != NULL) { *a = a[1]; a++; }
      }
    }
  }
if (sender_host_name == NULL && *sender_host_aliases != NULL)
  sender_host_name = *sender_host_aliases++;
if (sender_host_name != NULL) return OK;
HDEBUG(D_host_lookup)
  debug_printf("%s does not match any IP address for %s\n",
    sender_host_address, save_hostname);
old_pool = store_pool;
store_pool = POOL_PERM;
host_lookup_msg = string_sprintf(" (%s does not match any IP address for %s)",
  sender_host_address, save_hostname);
store_pool = old_pool;
host_lookup_failed = TRUE;
return FAIL;
}