cib_timeout_handler(gpointer data)
{
    struct timer_rec_s *timer = data;
    timer_expired = TRUE;
    crm_err("Call %d timed out after %ds", timer->call_id, timer->timeout);
    return TRUE;
}