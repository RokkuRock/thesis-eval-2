static LUA_FUNCTION(openssl_x509_check_host)
{
  X509 * cert = CHECK_OBJECT(1, X509, "openssl.x509");
  if (lua_isstring(L, 2))
  {
    const char *hostname = lua_tostring(L, 2);
    lua_pushboolean(L, X509_check_host(cert, hostname, strlen(hostname), 0, NULL));
  }
  else
  {
    lua_pushboolean(L, 0);
  }
  return 1;
}