static void jas_stream_initbuf(jas_stream_t *stream, int bufmode, char *buf,
  int bufsize)
{
	assert(!stream->bufbase_);
	if (bufmode != JAS_STREAM_UNBUF) {
		if (!buf) {
			if ((stream->bufbase_ = jas_malloc(JAS_STREAM_BUFSIZE +
			  JAS_STREAM_MAXPUTBACK))) {
				stream->bufmode_ |= JAS_STREAM_FREEBUF;
				stream->bufsize_ = JAS_STREAM_BUFSIZE;
			} else {
				stream->bufbase_ = stream->tinybuf_;
				stream->bufsize_ = 1;
			}
		} else {
			assert(bufsize > JAS_STREAM_MAXPUTBACK);
			stream->bufbase_ = JAS_CAST(uchar *, buf);
			stream->bufsize_ = bufsize - JAS_STREAM_MAXPUTBACK;
		}
	} else {
		assert(!buf);
		stream->bufbase_ = stream->tinybuf_;
		stream->bufsize_ = 1;
	}
	stream->bufstart_ = &stream->bufbase_[JAS_STREAM_MAXPUTBACK];
	stream->ptr_ = stream->bufstart_;
	stream->cnt_ = 0;
	stream->bufmode_ |= bufmode & JAS_STREAM_BUFMODEMASK;
}