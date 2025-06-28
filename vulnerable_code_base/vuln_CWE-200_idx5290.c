static void tcp_send_challenge_ack(struct sock *sk, const struct sk_buff *skb)
{
	static u32 challenge_timestamp;
	static unsigned int challenge_count;
	struct tcp_sock *tp = tcp_sk(sk);
	u32 now;
	if (tcp_oow_rate_limited(sock_net(sk), skb,
				 LINUX_MIB_TCPACKSKIPPEDCHALLENGE,
				 &tp->last_oow_ack_time))
		return;
	now = jiffies / HZ;
	if (now != challenge_timestamp) {
		challenge_timestamp = now;
		challenge_count = 0;
	}
	if (++challenge_count <= sysctl_tcp_challenge_ack_limit) {
		NET_INC_STATS(sock_net(sk), LINUX_MIB_TCPCHALLENGEACK);
		tcp_send_ack(sk);
	}
}