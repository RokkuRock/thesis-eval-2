find_auth_end (FlatpakProxyClient *client, Buffer *buffer)
{
  guchar *match;
  int i;
  if (client->auth_end_offset > 0)
    {
      gsize left = strlen (AUTH_END_STRING) - client->auth_end_offset;
      gsize to_match = MIN (left, buffer->pos);
      if (memcmp (buffer->data, &AUTH_END_STRING[client->auth_end_offset], to_match) == 0)
        {
          client->auth_end_offset += to_match;
          if (client->auth_end_offset == strlen (AUTH_END_STRING))
            return to_match;
          return -1;
        }
      client->auth_end_offset = -1;
    }
  match = memmem (buffer, buffer->pos,
                  AUTH_END_STRING, strlen (AUTH_END_STRING));
  if (match != NULL)
    return match - buffer->data + strlen (AUTH_END_STRING);
  for (i = MIN (strlen (AUTH_END_STRING) - 1, buffer->pos); i > 0; i--)
    {
      if (memcmp (buffer->data + buffer->pos - i, AUTH_END_STRING, i) == 0)
        {
          client->auth_end_offset = i;
          break;
        }
    }
  return -1;
}