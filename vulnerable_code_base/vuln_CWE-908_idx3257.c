char* parse_via(char* buffer, char* end, struct via_body *vbody)
{
	char* tmp;
	char* param_start;
	unsigned char state;
	unsigned char saved_state;
	int c_nest;
	int err;
	struct via_body* vb;
	struct via_param* param;
	vb=vbody;  
parse_again:
	vb->error=PARSE_ERROR;
	state=F_SIP;
	saved_state=0;  
	param_start=0;
	for(tmp=buffer;tmp<end;tmp++){
		switch(*tmp){
			case ' ':
			case'\t':
				switch(state){
					case L_VER:  
					case L_PROTO:
					case F_SIP:
					case F_VER:
					case F_PROTO:
						break;
					case FIN_UDP:
						vb->transport.len=tmp-vb->transport.s;
						vb->proto=PROTO_UDP;
						state=F_HOST;  
						goto main_via;
					case FIN_TCP:
						vb->transport.len=tmp-vb->transport.s;
						vb->proto=PROTO_TCP;
						state=F_HOST;  
						goto main_via;
					case FIN_TLS:
						vb->transport.len=tmp-vb->transport.s;
						vb->proto=PROTO_TLS;
						state=F_HOST;  
						goto main_via;
					case FIN_SCTP:
						vb->transport.len=tmp-vb->transport.s;
						vb->proto=PROTO_SCTP;
						state=F_HOST;  
						goto main_via;
					case FIN_WS:
					case WS_WSS:
						vb->transport.len=tmp-vb->transport.s;
						vb->proto=PROTO_WS;
						state=F_HOST;  
						goto main_via;
					case FIN_WSS:
						vb->transport.len=tmp-vb->transport.s;
						vb->proto=PROTO_WSS;
						state=F_HOST;  
						goto main_via;
					case OTHER_PROTO:
						vb->transport.len=tmp-vb->transport.s;
						vb->proto=PROTO_OTHER;
						state=F_HOST;  
						goto main_via;
					case FIN_SIP:
						vb->name.len=tmp-vb->name.s;
						state=L_VER;
						break;
					case FIN_VER:
						vb->version.len=tmp-vb->version.s;
						state=L_PROTO;
						break;
					case F_LF:
					case F_CRLF:
					case F_CR:  
						state=saved_state;
						break;
					default:
						LM_ERR("bad char <%c> on state %d\n", *tmp, state);
						goto parse_error;
				}
				break;
			case '\n':
				switch(state){
					case L_VER:
					case F_SIP:
					case F_VER:
					case F_PROTO:
					case L_PROTO:
						saved_state=state;
						state=F_LF;
						break;
					case FIN_UDP:
						vb->transport.len=tmp-vb->transport.s;
						vb->proto=PROTO_UDP;
						state=F_LF;
						saved_state=F_HOST;  
						goto main_via;
					case FIN_TCP:
						vb->transport.len=tmp-vb->transport.s;
						vb->proto=PROTO_TCP;
						state=F_LF;
						saved_state=F_HOST;  
						goto main_via;
					case FIN_TLS:
						vb->transport.len=tmp-vb->transport.s;
						vb->proto=PROTO_TLS;
						state=F_LF;
						saved_state=F_HOST;  
						goto main_via;
					case WS_WSS:
					case FIN_WS:
						vb->transport.len=tmp-vb->transport.s;
						vb->proto=PROTO_WS;
						state=F_LF;
						saved_state=F_HOST;  
						goto main_via;
					case FIN_WSS:
						vb->transport.len=tmp-vb->transport.s;
						vb->proto=PROTO_WS;
						state=F_LF;
						saved_state=F_HOST;  
						goto main_via;
					case OTHER_PROTO:
						vb->transport.len=tmp-vb->transport.s;
						vb->proto=PROTO_OTHER;
						state=F_LF;
						saved_state=F_HOST;  
						goto main_via;
					case FIN_SIP:
						vb->name.len=tmp-vb->name.s;
						state=F_LF;
						saved_state=L_VER;
						break;
					case FIN_VER:
						vb->version.len=tmp-vb->version.s;
						state=F_LF;
						saved_state=L_PROTO;
						break;
					case F_CR:
						state=F_CRLF;
						break;
					case F_LF:
					case F_CRLF:
						state=saved_state;
						goto endofheader;
					default:
						LM_ERR("bad char <%c> on state %d\n", *tmp, state);
						goto parse_error;
				}
				break;
			case '\r':
				switch(state){
					case L_VER:
					case F_SIP:
					case F_VER:
					case F_PROTO:
					case L_PROTO:
						saved_state=state;
						state=F_CR;
						break;
					case FIN_UDP:
						vb->transport.len=tmp-vb->transport.s;
						vb->proto=PROTO_UDP;
						state=F_CR;
						saved_state=F_HOST;
						goto main_via;
					case FIN_TCP:
						vb->transport.len=tmp-vb->transport.s;
						vb->proto=PROTO_TCP;
						state=F_CR;
						saved_state=F_HOST;
						goto main_via;
					case FIN_TLS:
						vb->transport.len=tmp-vb->transport.s;
						vb->proto=PROTO_TLS;
						state=F_CR;
						saved_state=F_HOST;
						goto main_via;
					case WS_WSS:
					case FIN_WS:
						vb->transport.len=tmp-vb->transport.s;
						vb->proto=PROTO_WS;
						state=F_CR;
						saved_state=F_HOST;
						goto main_via;
					case FIN_WSS:
						vb->transport.len=tmp-vb->transport.s;
						vb->proto=PROTO_WSS;
						state=F_CR;
						saved_state=F_HOST;
						goto main_via;
					case OTHER_PROTO:
						vb->transport.len=tmp-vb->transport.s;
						vb->proto=PROTO_OTHER;
						state=F_CR;
						saved_state=F_HOST;
						goto main_via;
					case FIN_SIP:
						vb->name.len=tmp-vb->name.s;
						state=F_CR;
						saved_state=L_VER;
						break;
					case FIN_VER:
						vb->version.len=tmp-vb->version.s;
						state=F_CR;
						saved_state=L_PROTO;
						break;
					case F_LF:  
					case F_CR:
					case F_CRLF:
						state=saved_state;
						goto endofheader;
					default:
						LM_ERR("bad char <%c> on state %d\n", *tmp, state);
						goto parse_error;
				}
				break;
			case '/':
				switch(state){
					case FIN_SIP:
						vb->name.len=tmp-vb->name.s;
						state=F_VER;
						break;
					case FIN_VER:
						vb->version.len=tmp-vb->version.s;
						state=F_PROTO;
						break;
					case L_VER:
						state=F_VER;
						break;
					case L_PROTO:
						state=F_PROTO;
						break;
					default:
						LM_ERR("bad char <%c> on state %d\n", *tmp, state);
						goto parse_error;
				}
				break;
			case 'S':
			case 's':
				switch(state){
					case F_SIP:
						state=SIP1;
						vb->name.s=tmp;
						break;
					case TLS2:
						state=FIN_TLS;
						break;
					case F_PROTO:
						state=SCTP1;
						vb->transport.s=tmp;
						break;
					case WS1:
						state=WS_WSS;
						break;
					case WS_WSS:
						state=FIN_WSS;
						break;
					case OTHER_PROTO:
						break;
					case UDP1:
					case UDP2:
					case FIN_UDP:
					case TCP_TLS1:
					case TCP2:
					case FIN_TCP:
					case FIN_TLS:
					case SCTP1:
					case SCTP2:
					case SCTP3:
					case FIN_SCTP:
					case FIN_WS:
					case FIN_WSS:
						state=OTHER_PROTO;
						break;
					default:
						LM_ERR("bad char <%c> on state %d\n", *tmp, state);
						goto parse_error;
				}
				break;
			case 'I':
			case 'i':
				switch(state){
					case SIP1:
						state=SIP2;
						break;
					case OTHER_PROTO:
						break;
					case UDP1:
					case UDP2:
					case FIN_UDP:
					case TCP_TLS1:
					case TCP2:
					case FIN_TCP:
					case TLS2:
					case FIN_TLS:
					case SCTP1:
					case SCTP2:
					case SCTP3:
					case FIN_SCTP:
					case WS1:
					case WS_WSS:
					case FIN_WS:
					case FIN_WSS:
						state=OTHER_PROTO;
						break;
					default:
						LM_ERR("bad char <%c> on state %d\n", *tmp, state);
						goto parse_error;
				}
				break;
			case 'p':
			case 'P':
				switch(state){
					case SIP2:
						state=FIN_SIP;
						break;
					case UDP2:
						state=FIN_UDP;
						break;
					case TCP2:
						state=FIN_TCP;
						break;
					case SCTP3:
						state=FIN_SCTP;
						break;
					case OTHER_PROTO:
						break;
					case UDP1:
					case FIN_UDP:
					case TCP_TLS1:
					case FIN_TCP:
					case TLS2:
					case FIN_TLS:
					case SCTP1:
					case SCTP2:
					case FIN_SCTP:
					case WS1:
					case WS_WSS:
					case FIN_WS:
					case FIN_WSS:
						state=OTHER_PROTO;
						break;
					default:
						LM_ERR("bad char <%c> on state %d\n", *tmp, state);
						goto parse_error;
				}
				break;
			case 'U':
			case 'u':
				switch(state){
					case F_PROTO:
						state=UDP1;
						vb->transport.s=tmp;
						break;
					case OTHER_PROTO:
						break;
					case UDP1:
					case UDP2:
					case FIN_UDP:
					case TCP_TLS1:
					case TCP2:
					case FIN_TCP:
					case TLS2:
					case FIN_TLS:
					case SCTP1:
					case SCTP2:
					case SCTP3:
					case FIN_SCTP:
					case WS1:
					case WS_WSS:
					case FIN_WS:
					case FIN_WSS:
						state=OTHER_PROTO;
						break;
					default:
						LM_ERR("bad char <%c> on state %d\n", *tmp, state);
						goto parse_error;
				}
				break;
			case 'D':
			case 'd':
				switch(state){
					case UDP1:
						state=UDP2;
						break;
					case OTHER_PROTO:
						break;
					case UDP2:
					case FIN_UDP:
					case TCP_TLS1:
					case TCP2:
					case FIN_TCP:
					case TLS2:
					case FIN_TLS:
					case SCTP1:
					case SCTP2:
					case SCTP3:
					case FIN_SCTP:
					case WS1:
					case WS_WSS:
					case FIN_WS:
					case FIN_WSS:
						state=OTHER_PROTO;
						break;
					default:
						LM_ERR("bad char <%c> on state %d\n", *tmp, state);
						goto parse_error;
				}
				break;
			case 'T':
			case 't':
				switch(state){
					case F_PROTO:
						state=TCP_TLS1;
						vb->transport.s=tmp;
						break;
					case SCTP2:
						state=SCTP3;
						break;
					case OTHER_PROTO:
						break;
					case UDP1:
					case UDP2:
					case FIN_UDP:
					case TCP_TLS1:
					case TCP2:
					case FIN_TCP:
					case TLS2:
					case FIN_TLS:
					case SCTP1:
					case SCTP3:
					case FIN_SCTP:
					case WS1:
					case WS_WSS:
					case FIN_WS:
					case FIN_WSS:
						state=OTHER_PROTO;
						break;
					default:
						LM_ERR("bad char <%c> on state %d\n", *tmp, state);
						goto parse_error;
				}
				break;
			case 'C':
			case 'c':
				switch(state){
					case TCP_TLS1:
						state=TCP2;
						break;
					case SCTP1:
						state=SCTP2;
						break;
					case OTHER_PROTO:
						break;
					case UDP1:
					case UDP2:
					case FIN_UDP:
					case TCP2:
					case FIN_TCP:
					case TLS2:
					case FIN_TLS:
					case SCTP2:
					case SCTP3:
					case FIN_SCTP:
					case WS1:
					case WS_WSS:
					case FIN_WS:
					case FIN_WSS:
						state=OTHER_PROTO;
						break;
					default:
						LM_ERR("bad char <%c> on state %d\n", *tmp, state);
						goto parse_error;
				}
				break;
			case 'L':
			case 'l':
				switch(state){
					case TCP_TLS1:
						state=TLS2;
						break;
					case OTHER_PROTO:
						break;
					case UDP1:
					case UDP2:
					case FIN_UDP:
					case TCP2:
					case FIN_TCP:
					case TLS2:
					case FIN_TLS:
					case SCTP1:
					case SCTP2:
					case SCTP3:
					case FIN_SCTP:
					case WS1:
					case WS_WSS:
					case FIN_WS:
					case FIN_WSS:
						state=OTHER_PROTO;
						break;
					default:
						LM_ERR("bad char <%c> on state %d\n", *tmp, state);
						goto parse_error;
				}
				break;
			case 'W':
			case 'w':
				switch(state){
					case F_PROTO:
						state=WS1;
						vb->transport.s=tmp;
						break;
					case OTHER_PROTO:
						break;
					case UDP1:
					case UDP2:
					case FIN_UDP:
					case TCP_TLS1:
					case TCP2:
					case FIN_TCP:
					case TLS2:
					case FIN_TLS:
					case SCTP1:
					case SCTP2:
					case SCTP3:
					case FIN_SCTP:
					case WS1:
					case WS_WSS:
					case FIN_WS:
					case FIN_WSS:
						state=OTHER_PROTO;
						break;
					default:
						LM_ERR("bad char <%c> on state %d\n", *tmp, state);
						goto parse_error;
				}
				break;
			case '2':
				switch(state){
					case F_VER:
						state=VER1;
						vb->version.s=tmp;
						break;
					case OTHER_PROTO:
						break;
					case UDP1:
					case UDP2:
					case FIN_UDP:
					case TCP_TLS1:
					case TCP2:
					case FIN_TCP:
					case TLS2:
					case FIN_TLS:
					case SCTP1:
					case SCTP2:
					case SCTP3:
					case FIN_SCTP:
					case WS1:
					case WS_WSS:
					case FIN_WS:
					case FIN_WSS:
						state=OTHER_PROTO;
						break;
					default:
						LM_ERR("bad char <%c> on state %d\n", *tmp, state);
						goto parse_error;
				}
				break;
			case '.':
				switch(state){
					case VER1:
						state=VER2;
						break;
					case OTHER_PROTO:
						break;
					case UDP1:
					case UDP2:
					case FIN_UDP:
					case TCP_TLS1:
					case TCP2:
					case FIN_TCP:
					case TLS2:
					case FIN_TLS:
					case SCTP1:
					case SCTP2:
					case SCTP3:
					case FIN_SCTP:
					case WS1:
					case WS_WSS:
					case FIN_WS:
					case FIN_WSS:
						state=OTHER_PROTO;
						break;
					default:
						LM_ERR("bad char <%c> on state %d\n", *tmp, state);
						goto parse_error;
				}
				 break;
			case '0':
				switch(state){
					case VER2:
						state=FIN_VER;
						break;
					case OTHER_PROTO:
						break;
					case UDP1:
					case UDP2:
					case FIN_UDP:
					case TCP_TLS1:
					case TCP2:
					case FIN_TCP:
					case TLS2:
					case FIN_TLS:
					case SCTP1:
					case SCTP2:
					case SCTP3:
					case FIN_SCTP:
					case WS1:
					case WS_WSS:
					case FIN_WS:
					case FIN_WSS:
						state=OTHER_PROTO;
						break;
					default:
						LM_ERR("bad char <%c> on state %d\n", *tmp, state);
						goto parse_error;
				}
				break;
			default:
				switch(state){
					case F_PROTO:
						state=OTHER_PROTO;
						vb->transport.s=tmp;
						break;
					case OTHER_PROTO:
						break;
					case UDP1:
					case UDP2:
					case FIN_UDP:
					case TCP_TLS1:
					case TCP2:
					case FIN_TCP:
					case TLS2:
					case FIN_TLS:
					case SCTP1:
					case SCTP2:
					case SCTP3:
					case FIN_SCTP:
					case WS1:
					case WS_WSS:
					case FIN_WS:
					case FIN_WSS:
						state=OTHER_PROTO;
						break;
					default:
						LM_ERR("bad char <%c> on state %d\n", *tmp, state);
						goto parse_error;
				}
				break;
		}
	}  
	LM_ERR("bad via: end of packet on state=%d\n", state);
	goto parse_error;
 main_via:
	tmp++;
	c_nest=0;
	 ;
	for(;*tmp;tmp++){
		switch(*tmp){
		case ' ':
		case '\t':
			switch(state){
					case F_HOST: 
						break;
					case P_HOST:
						 vb->host.len=tmp-vb->host.s;
						 state=L_PORT;
						 break;
					case L_PORT:  
					case F_PORT:
						break;
					case P_PORT:
						vb->port_str.len=tmp-vb->port_str.s;
						state=L_PARAM;
						break;
					case L_PARAM:  
					case F_PARAM:
						break;
					case P_PARAM:
						state=L_PARAM;
						break;
					case L_VIA:
					case F_VIA:  
						break;
					case F_COMMENT:
					case P_COMMENT:
						break;
					case F_IP6HOST:  
					case P_IP6HOST:
						LM_ERR("bad ipv6 reference\n");
						goto parse_error;
					case F_CRLF:
					case F_LF:
					case F_CR:
						state=saved_state;
						break;
					default:
						LM_CRIT("on <%c>, state=%d\n",*tmp, state);
						goto parse_error;
				}
			break;
			case '\n':
				switch(state){
					case F_HOST: 
					case L_PORT:  
					case F_PORT:
					case L_PARAM:  
					case F_PARAM:
					case F_VIA:  
					case L_VIA:
					case F_COMMENT:
					case P_COMMENT:
					case F_IP6HOST:
					case P_IP6HOST:
						saved_state=state;
						state=F_LF;
						break;
					case P_HOST:
						 vb->host.len=tmp-vb->host.s;
						 saved_state=L_PORT;
						 state=F_LF;
						 break;
					case P_PORT:
						vb->port_str.len=tmp-vb->port_str.s;
						saved_state=L_PARAM;
						state=F_LF;
						break;
					case P_PARAM:
						saved_state=L_PARAM;
						state=F_LF;
						break;
					case F_CR:
						state=F_CRLF;
						break;
					case F_CRLF:
					case F_LF:
						state=saved_state;
						goto endofheader;
					default:
						LM_CRIT("BUG on <%c>\n",*tmp);
						goto  parse_error;
				}
			break;
		case '\r':
				switch(state){
					case F_HOST: 
					case L_PORT:  
					case F_PORT:
					case L_PARAM:  
					case F_PARAM:
					case F_VIA:  
					case L_VIA:
					case F_COMMENT:
					case P_COMMENT:
					case F_IP6HOST:
					case P_IP6HOST:
						saved_state=state;
						state=F_CR;
						break;
					case P_HOST:
						 vb->host.len=tmp-vb->host.s;
						 saved_state=L_PORT;
						 state=F_CR;
						 break;
					case P_PORT:
						vb->port_str.len=tmp-vb->port_str.s;
						saved_state=L_PARAM;
						state=F_CR;
						break;
					case P_PARAM:
						saved_state=L_PARAM;
						state=F_CR;
						break;
					case F_CRLF:
					case F_CR:
					case F_LF:
						state=saved_state;
						goto endofheader;
					default:
						LM_CRIT("on <%c>\n",*tmp);
						goto parse_error;
				}
			break;
			case ':':
				switch(state){
					case F_HOST:
					case F_IP6HOST:
						state=P_IP6HOST;
						break;
					case P_IP6HOST:
						break;
					case P_HOST:
						vb->host.len=tmp-vb->host.s;
						state=F_PORT;
						break;
					case L_PORT:
						state=F_PORT;
						break;
					case P_PORT:
						LM_ERR("bad port\n");
						goto parse_error;
					case L_PARAM:
					case F_PARAM:
					case P_PARAM:
						LM_ERR("bad char <%c> in state %d\n",
							*tmp,state);
						goto parse_error;
					case L_VIA:
					case F_VIA:
						LM_ERR("bad char in compact via\n");
						goto parse_error;
					case F_CRLF:
					case F_LF:
					case F_CR:
						goto endofheader;
					case F_COMMENT: 
						vb->comment.s=tmp;
						state=P_COMMENT;
						break;
					case P_COMMENT:  
						break;
					default:
						LM_CRIT("on <%c> state %d\n", *tmp, state);
						goto parse_error;
				}
				break;
			case ';':
				switch(state){
					case F_HOST:
					case F_IP6HOST:
						LM_ERR(" no host found\n");
						goto parse_error;
					case P_IP6HOST:
						LM_ERR(" bad ipv6 reference\n");
						goto parse_error;
					case P_HOST:
						vb->host.len=tmp-vb->host.s;
						state=F_PARAM;
						param_start=tmp+1;
						break;
					case P_PORT:
						vb->port_str.len=tmp-vb->port_str.s;
					case L_PORT:
					case L_PARAM:
						state=F_PARAM;
						param_start=tmp+1;
						break;
					case F_PORT:
						LM_ERR(" bad char <%c> in state %d\n",
							*tmp,state);
						goto parse_error;
					case F_PARAM:
						LM_ERR("null param?\n");
						goto parse_error;
					case P_PARAM:
						state=F_PARAM;
						param_start=tmp+1;
						break;
					case L_VIA:
					case F_VIA:
						LM_ERR("bad char <%c> in next via\n", *tmp);
						goto parse_error;
					case F_CRLF:
					case F_LF:
					case F_CR:
						goto endofheader;
					case F_COMMENT: 
						vb->comment.s=tmp;
						state=P_COMMENT;
						break;
					case P_COMMENT:  
						break;
					default:
						LM_CRIT("on <%c> state %d\n", *tmp, state);
						goto parse_error;
				}
			break;
			case ',':
				switch(state){
					case F_HOST:
					case F_IP6HOST:
						LM_ERR("no host found\n");
						goto parse_error;
					case P_IP6HOST:
						LM_ERR(" bad ipv6 reference\n");
						goto parse_error;
					case P_HOST:
						vb->host.len=tmp-vb->host.s;
						state=F_VIA;
						break;
					case P_PORT:
						vb->port_str.len=tmp-vb->port_str.s;
						state=F_VIA;
						break;
					case L_PORT:
					case L_PARAM:
					case P_PARAM:
					case L_VIA:
						state=F_VIA;
						break;
					case F_PORT:
					case F_PARAM:
						LM_ERR("invalid char <%c> in state %d\n", *tmp,state);
						goto parse_error;
					case F_VIA:
						break;
					case F_CRLF:
					case F_LF:
					case F_CR:
						goto endofheader;
					case F_COMMENT: 
						vb->comment.s=tmp;
						state=P_COMMENT;
						break;
					case P_COMMENT:  
						break;
					default:
						LM_CRIT("on <%c> state %d\n",*tmp, state);
						goto  parse_error;
				}
			break;
			case '(':
				switch(state){
					case F_HOST:
					case F_PORT:
					case F_PARAM:
					case F_VIA:
					case F_IP6HOST:
					case P_IP6HOST:  
						LM_ERR(" on <%c> state %d\n", *tmp, state);
						goto  parse_error;
					case P_HOST:
						vb->host.len=tmp-vb->host.s;
						state=F_COMMENT;
						c_nest++;
						break;
					case P_PORT:
						vb->port_str.len=tmp-vb->port_str.s;
						state=F_COMMENT;
						c_nest++;
						break;
					case P_PARAM:
						vb->params.len=tmp-vb->params.s;
						state=F_COMMENT;
						c_nest++;
						break;
					case L_PORT:
					case L_PARAM:
					case L_VIA:
						state=F_COMMENT;
						vb->params.len=tmp-vb->params.s;
						c_nest++;
						break;
					case P_COMMENT:
					case F_COMMENT:
						c_nest++;
						break;
					case F_CRLF:
					case F_LF:
					case F_CR:
						goto endofheader;
					default:
						LM_CRIT("on <%c> state %d\n", *tmp, state);
						goto  parse_error;
				}
			break;
			case ')':
				switch(state){
					case F_COMMENT:
					case P_COMMENT:
						if (c_nest){
							c_nest--;
							if(c_nest==0){
								state=L_VIA;
								vb->comment.len=tmp-vb->comment.s;
								break;
							}
						}else{
							LM_ERR(" missing '(' - nesting= %d\n", c_nest);
							 goto parse_error;
						}
						break;
					case F_HOST:
					case F_PORT:
					case F_PARAM:
					case F_VIA:
					case P_HOST:
					case P_PORT:
					case P_PARAM:
					case L_PORT:
					case L_PARAM:
					case L_VIA:
					case F_IP6HOST:
					case P_IP6HOST:
						LM_ERR(" on <%c> state %d\n",*tmp, state);
						goto  parse_error;
					case F_CRLF:
					case F_LF:
					case F_CR:
						goto endofheader;
					default:
						LM_CRIT("on <%c> state %d\n", *tmp, state);
						goto  parse_error;
				}
				break;
			case '[':
				switch(state){
					case F_HOST:
						vb->host.s=tmp;  
						state=F_IP6HOST;
						break;
					case F_COMMENT: 
						vb->comment.s=tmp;
						state=P_COMMENT;
						break;
					case P_COMMENT:
						break;
					case F_CRLF:
					case F_LF:
					case F_CR:
						goto endofheader;
					default:
						LM_ERR("on <%c> state %d\n",*tmp, state);
						goto  parse_error;
				}
				break;
			case ']':
				switch(state){
					case P_IP6HOST:
						vb->host.len=(tmp-vb->host.s)+1;  
						state=L_PORT;
						break;
					case F_CRLF:
					case F_LF:
					case F_CR:
						goto endofheader;
					case F_COMMENT: 
						vb->comment.s=tmp;
						state=P_COMMENT;
						break;
					case P_COMMENT:
						break;
					default:
						LM_ERR("on <%c> state %d\n",*tmp, state);
						goto  parse_error;
				}
				break;
			default:
				switch(state){
					case F_HOST:
						state=P_HOST;
						vb->host.s=tmp;
					case P_HOST:
						if ( (*tmp<'a' || *tmp>'z') && (*tmp<'A' || *tmp>'Z')
						&& (*tmp<'0' || *tmp>'9') && *tmp!='-' && *tmp!='.')
							goto parse_error;
						break;
					case F_PORT:
						state=P_PORT;
						vb->port_str.s=tmp;
					case P_PORT:
						if ( *tmp<'0' || *tmp>'9' )
							goto parse_error;
						break;
					case F_PARAM:
						 ;
						if(vb->params.s==0) vb->params.s=param_start;
						param=pkg_malloc(sizeof(struct via_param));
						if (param==0){
							LM_ERR("no pkg memory left\n");
							goto error;
						}
						memset(param,0, sizeof(struct via_param));
						param->start=param_start;
						tmp=parse_via_param(tmp, end, &state, &saved_state,
											param);
						switch(state){
							case F_PARAM:
								param_start=tmp+1;
							case L_PARAM:
							case F_LF:
							case F_CR:
								vb->params.len=tmp - vb->params.s;
								break;
							case F_VIA:
								vb->params.len=param->start+param->size
												-vb->params.s;
								break;
							case END_OF_HEADER:
								vb->params.len=param->start+param->size
												-vb->params.s;
								break;
							case PARAM_ERROR:
								pkg_free(param);
								goto parse_error;
							default:
								pkg_free(param);
								LM_ERR(" after parse_via_param: invalid "
										"char <%c> on state %d\n",*tmp, state);
								goto parse_error;
						}
						if (vb->last_param)	vb->last_param->next=param;
						else				vb->param_lst=param;
						vb->last_param=param;
						switch(param->type){
							case PARAM_BRANCH:
								vb->branch=param;
								break;
							case PARAM_RECEIVED:
								vb->received=param;
								break;
							case PARAM_RPORT:
								vb->rport=param;
								break;
							case PARAM_I:
								vb->i=param;
								break;
							case PARAM_ALIAS:
								vb->alias=param;
								break;
							case PARAM_MADDR:
								vb->maddr=param;
								break;
						}
						if (state==END_OF_HEADER){
							state=saved_state;
							goto endofheader;
						}
						break;
					case P_PARAM:
						break;
					case F_VIA:
						goto nextvia;
					case L_PORT:
					case L_PARAM:
					case L_VIA:
						LM_ERR("on <%c> state %d (default)\n",*tmp, state);
						goto  parse_error;
					case F_COMMENT:
						state=P_COMMENT;
						vb->comment.s=tmp;
						break;
					case P_COMMENT:
						break;
					case F_IP6HOST:
						state=P_IP6HOST;
					case P_IP6HOST:
						if ( (*tmp<'a' || *tmp>'f') && (*tmp<'A' || *tmp>'F')
						&& (*tmp<'0' || *tmp>'9') && *tmp!=':')
							goto parse_error;
						break;
					case F_CRLF:
					case F_LF:
					case F_CR:
						goto endofheader;
					default:
						LM_CRIT("invalid char <%c> in state %d\n",*tmp, state);
						goto parse_error;
				}
		}
	}
	LM_DBG("end of packet reached, state=%d\n", state);
	goto endofpacket;  
endofheader:
	state=saved_state;
	LM_DBG("end of header reached, state=%d\n", state);
endofpacket:
	switch(state){
		case P_HOST:
		case L_PORT:
		case P_PORT:
		case L_PARAM:
		case P_PARAM:
		case P_VALUE:
		case GEN_PARAM:
		case FIN_HIDDEN:
		case L_VIA:
			break;
		default:
			LM_ERR(" invalid via - end of header in state %d\n", state);
			goto parse_error;
	}
	vb->error=PARSE_OK;
	vb->bsize=tmp-buffer;
	if (vb->port_str.s){
		vb->port=str2s(vb->port_str.s, vb->port_str.len, &err);
		if (err){
					LM_ERR(" invalid port number <%.*s>\n",
						vb->port_str.len, ZSW(vb->port_str.s));
					goto parse_error;
		}
	}
	return tmp;
nextvia:
	LM_DBG("next_via\n");
	vb->error=PARSE_OK;
	vb->bsize=tmp-buffer;
	if (vb->port_str.s){
		vb->port=str2s(vb->port_str.s, vb->port_str.len, &err);
		if (err){
					LM_ERR(" invalid port number <%.*s>\n",
						vb->port_str.len, ZSW(vb->port_str.s));
					goto parse_error;
		}
	}
	vb->next=pkg_malloc(sizeof(struct via_body));
	if (vb->next==0){
		LM_ERR(" out of pkg memory\n");
		goto error;
	}
	vb=vb->next;
	memset(vb, 0, sizeof(struct via_body));
	buffer=tmp;
	goto parse_again;
parse_error:
	if (end>buffer){
		LM_ERR(" <%.*s>\n", (int)(end-buffer), ZSW(buffer));
	}
	if ((tmp>buffer)&&(tmp<end)){
		LM_ERR("parsed so far:<%.*s>\n",
				(int)(tmp-buffer), ZSW(buffer) );
	}else{
		LM_ERR("via parse failed\n");
	}
error:
	vb->error=PARSE_ERROR;
	vbody->error=PARSE_ERROR;  
	return tmp;
}