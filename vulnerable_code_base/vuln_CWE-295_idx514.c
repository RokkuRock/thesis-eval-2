static LUA_FUNCTION(openssl_x509_check_email)
{
  X509 * cert = CHECK_OBJECT(1, X509, "openssl.x509");
  if (lua_isstring(L, 2))
  {
    const char *email = lua_tostring(L, 2);
    lua_pushboolean(L, X509_check_email(cert, email, strlen(email), 0));
  }
  else
  {
    lua_pushboolean(L, 0);
  }
  return 1;
}