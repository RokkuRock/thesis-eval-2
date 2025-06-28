ansi_step(pansi, ch)
	struct ansi_state *pansi;
	LWCHAR ch;
{
	if (pansi->hlink)
	{
		if (ch == '\7')
			return ANSI_END;
		if (pansi->prev_esc && ch == '\\')
			return ANSI_END;
		pansi->prev_esc = (ch == ESC);
		return ANSI_MID;
	}
	if (pansi->hindex >= 0)
	{
		static char hlink_prefix[] = ESCS "]8;";
		if (ch == hlink_prefix[pansi->hindex] ||
		    (pansi->hindex == 0 && IS_CSI_START(ch)))
		{
			pansi->hindex++;
			if (hlink_prefix[pansi->hindex] == '\0')
				pansi->hlink = 1;  
			return ANSI_MID;
		}
		pansi->hindex = -1;  
	}
	if (is_ansi_middle(ch))
		return ANSI_MID;
	if (is_ansi_end(ch))
		return ANSI_END;
	return ANSI_ERR;
}