predicate_parse(const char *str, int nr_parens, int nr_preds,
		parse_pred_fn parse_pred, void *data,
		struct filter_parse_error *pe)
{
	struct prog_entry *prog_stack;
	struct prog_entry *prog;
	const char *ptr = str;
	char *inverts = NULL;
	int *op_stack;
	int *top;
	int invert = 0;
	int ret = -ENOMEM;
	int len;
	int N = 0;
	int i;
	nr_preds += 2;  
	op_stack = kmalloc_array(nr_parens, sizeof(*op_stack), GFP_KERNEL);
	if (!op_stack)
		return ERR_PTR(-ENOMEM);
	prog_stack = kcalloc(nr_preds, sizeof(*prog_stack), GFP_KERNEL);
	if (!prog_stack) {
		parse_error(pe, -ENOMEM, 0);
		goto out_free;
	}
	inverts = kmalloc_array(nr_preds, sizeof(*inverts), GFP_KERNEL);
	if (!inverts) {
		parse_error(pe, -ENOMEM, 0);
		goto out_free;
	}
	top = op_stack;
	prog = prog_stack;
	*top = 0;
	while (*ptr) {						 
		const char *next = ptr++;
		if (isspace(*next))
			continue;
		switch (*next) {
		case '(':					 
			if (top - op_stack > nr_parens)
				return ERR_PTR(-EINVAL);
			*(++top) = invert;
			continue;
		case '!':					 
			if (!is_not(next))
				break;
			invert = !invert;
			continue;
		}
		if (N >= nr_preds) {
			parse_error(pe, FILT_ERR_TOO_MANY_PREDS, next - str);
			goto out_free;
		}
		inverts[N] = invert;				 
		prog[N].target = N-1;
		len = parse_pred(next, data, ptr - str, pe, &prog[N].pred);
		if (len < 0) {
			ret = len;
			goto out_free;
		}
		ptr = next + len;
		N++;
		ret = -1;
		while (1) {					 
			next = ptr++;
			if (isspace(*next))
				continue;
			switch (*next) {
			case ')':
			case '\0':
				break;
			case '&':
			case '|':
				if (next[1] == next[0]) {
					ptr++;
					break;
				}
			default:
				parse_error(pe, FILT_ERR_TOO_MANY_PREDS,
					    next - str);
				goto out_free;
			}
			invert = *top & INVERT;
			if (*top & PROCESS_AND) {		 
				update_preds(prog, N - 1, invert);
				*top &= ~PROCESS_AND;
			}
			if (*next == '&') {			 
				*top |= PROCESS_AND;
				break;
			}
			if (*top & PROCESS_OR) {		 
				update_preds(prog, N - 1, !invert);
				*top &= ~PROCESS_OR;
			}
			if (*next == '|') {			 
				*top |= PROCESS_OR;
				break;
			}
			if (!*next)				 
				goto out;
			if (top == op_stack) {
				ret = -1;
				parse_error(pe, FILT_ERR_TOO_MANY_CLOSE, ptr - str);
				goto out_free;
			}
			top--;					 
		}
	}
 out:
	if (top != op_stack) {
		parse_error(pe, FILT_ERR_TOO_MANY_OPEN, ptr - str);
		goto out_free;
	}
	if (!N) {
		ret = -EINVAL;
		parse_error(pe, FILT_ERR_NO_FILTER, ptr - str);
		goto out_free;
	}
	prog[N].pred = NULL;					 
	prog[N].target = 1;		 
	prog[N+1].pred = NULL;
	prog[N+1].target = 0;		 
	prog[N-1].target = N;
	prog[N-1].when_to_branch = false;
	for (i = N-1 ; i--; ) {
		int target = prog[i].target;
		if (prog[i].when_to_branch == prog[target].when_to_branch)
			prog[i].target = prog[target].target;
	}
	for (i = 0; i < N; i++) {
		invert = inverts[i] ^ prog[i].when_to_branch;
		prog[i].when_to_branch = invert;
		if (WARN_ON(prog[i].target <= i)) {
			ret = -EINVAL;
			goto out_free;
		}
	}
	kfree(op_stack);
	kfree(inverts);
	return prog;
out_free:
	kfree(op_stack);
	kfree(inverts);
	if (prog_stack) {
		for (i = 0; prog_stack[i].pred; i++)
			kfree(prog_stack[i].pred);
		kfree(prog_stack);
	}
	return ERR_PTR(ret);
}