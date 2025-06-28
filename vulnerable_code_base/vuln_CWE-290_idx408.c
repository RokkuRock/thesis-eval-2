valid_host(cupsd_client_t *con)		 
{
  cupsd_alias_t	*a;			 
  cupsd_netif_t	*netif;			 
  const char	*end;			 
  char		*ptr;			 
  strlcpy(con->clientname, httpGetField(con->http, HTTP_FIELD_HOST),
          sizeof(con->clientname));
  if ((ptr = strrchr(con->clientname, ':')) != NULL && !strchr(ptr, ']'))
  {
    *ptr++ = '\0';
    con->clientport = atoi(ptr);
  }
  else
    con->clientport = con->serverport;
  if (httpAddrLocalhost(httpGetAddress(con->http)))
  {
    return (!_cups_strcasecmp(con->clientname, "localhost") ||
	    !_cups_strcasecmp(con->clientname, "localhost.") ||
#ifdef __linux
	    !_cups_strcasecmp(con->clientname, "localhost.localdomain") ||
#endif  
            !strcmp(con->clientname, "127.0.0.1") ||
	    !strcmp(con->clientname, "[::1]"));
  }
#if defined(HAVE_DNSSD) || defined(HAVE_AVAHI)
  if ((end = strrchr(con->clientname, '.')) != NULL && end > con->clientname &&
      !end[1])
  {
    for (end --; end > con->clientname && *end != '.'; end --);
  }
  if (end && (!_cups_strcasecmp(end, ".local") ||
	      !_cups_strcasecmp(end, ".local.")))
    return (1);
#endif  
  if (isdigit(con->clientname[0] & 255) || con->clientname[0] == '[')
  {
    http_addrlist_t *addrlist;		 
    if ((addrlist = httpAddrGetList(con->clientname, AF_UNSPEC, NULL)) != NULL)
    {
      httpAddrFreeList(addrlist);
      return (1);
    }
  }
  for (a = (cupsd_alias_t *)cupsArrayFirst(ServerAlias);
       a;
       a = (cupsd_alias_t *)cupsArrayNext(ServerAlias))
  {
    if (!strcmp(a->name, "*"))
      return (1);
    if (!_cups_strncasecmp(con->clientname, a->name, a->namelen))
    {
      end = con->clientname + a->namelen;
      if (!*end || (*end == '.' && !end[1]))
        return (1);
    }
  }
#if defined(HAVE_DNSSD) || defined(HAVE_AVAHI)
  for (a = (cupsd_alias_t *)cupsArrayFirst(DNSSDAlias);
       a;
       a = (cupsd_alias_t *)cupsArrayNext(DNSSDAlias))
  {
    if (!strcmp(a->name, "*"))
      return (1);
    if (!_cups_strncasecmp(con->clientname, a->name, a->namelen))
    {
      end = con->clientname + a->namelen;
      if (!*end || (*end == '.' && !end[1]))
        return (1);
    }
  }
#endif  
  for (netif = (cupsd_netif_t *)cupsArrayFirst(NetIFList);
       netif;
       netif = (cupsd_netif_t *)cupsArrayNext(NetIFList))
  {
    if (!_cups_strncasecmp(con->clientname, netif->hostname, netif->hostlen))
    {
      end = con->clientname + netif->hostlen;
      if (!*end || (*end == '.' && !end[1]))
        return (1);
    }
  }
  return (0);
}