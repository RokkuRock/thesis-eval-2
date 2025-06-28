monitor_sync(struct monitor *pmonitor)
{
	if (options.compression) {
		mm_share_sync(&pmonitor->m_zlib, &pmonitor->m_zback);
	}
}