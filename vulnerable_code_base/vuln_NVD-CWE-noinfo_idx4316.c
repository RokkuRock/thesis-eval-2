static bool freelist_state_initialize(union freelist_init_state *state,
				struct kmem_cache *cachep,
				unsigned int count)
{
	bool ret;
	unsigned int rand;
	rand = get_random_int();
	if (!cachep->random_seq) {
		prandom_seed_state(&state->rnd_state, rand);
		ret = false;
	} else {
		state->list = cachep->random_seq;
		state->count = count;
		state->pos = 0;
		state->rand = rand;
		ret = true;
	}
	return ret;
}