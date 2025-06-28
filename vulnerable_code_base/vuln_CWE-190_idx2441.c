#endif  
int main(void) {
	if (core_init() != RLC_OK) {
		core_clean();
		return 1;
	}
	util_banner("Tests for the CP module", 0);
	#if defined(WITH_BN) && defined(WITH_PC)
		util_banner("Protocols based on accumulators:\n", 0);
		if (pc_param_set_any() == RLC_OK) {
			if (psi() != RLC_OK) {
				core_clean();
				return 1;
			}
		}
	#endif
#if defined(WITH_BN)
	util_banner("Protocols based on integer factorization:\n", 0);
	if (rsa() != RLC_OK) {
		core_clean();
		return 1;
	}
	if (rabin() != RLC_OK) {
		core_clean();
		return 1;
	}
	if (benaloh() != RLC_OK) {
		core_clean();
		return 1;
	}
	if (paillier() != RLC_OK) {
		core_clean();
		return 1;
	}
	if (subgroup_paillier() != RLC_OK) {
		core_clean();
		return 1;
	}
#endif
#if defined(WITH_EC)
	util_banner("Protocols based on elliptic curves:\n", 0);
	if (ec_param_set_any() == RLC_OK) {
		if (ecdh() != RLC_OK) {
			core_clean();
			return 1;
		}
		if (ecmqv() != RLC_OK) {
			core_clean();
			return 1;
		}
#if defined(WITH_BC)
		if (ecies() != RLC_OK) {
			core_clean();
			return 1;
		}
#endif
		if (ecdsa() != RLC_OK) {
			core_clean();
			return 1;
		}
		if (ecss() != RLC_OK) {
			core_clean();
			return 1;
		}
		if (vbnn() != RLC_OK) {
			core_clean();
			return 1;
		}
		if (pok() != RLC_OK) {
			core_clean();
			return 1;
		}
		if (sok() != RLC_OK) {
			core_clean();
			return 1;
		}
		if (ers() != RLC_OK) {
			core_clean();
			return 1;
		}
		if (smlers() != RLC_OK) {
			core_clean();
			return 1;
		}
		if (etrs() != RLC_OK) {
			core_clean();
			return 1;
		}
	}
#endif
#if defined(WITH_PC)
	util_banner("Protocols based on pairings:\n", 0);
	if (pc_param_set_any() == RLC_OK) {
		if (pdpub() != RLC_OK) {
			core_clean();
			return 1;
		}
		if (pdprv() != RLC_OK) {
			core_clean();
			return 1;
		}
		if (sokaka() != RLC_OK) {
			core_clean();
			return 1;
		}
		if (ibe() != RLC_OK) {
			core_clean();
			return 1;
		}
		if (bgn() != RLC_OK) {
			core_clean();
			return 1;
		}
		if (bls() != RLC_OK) {
			core_clean();
			return 1;
		}
		if (bbs() != RLC_OK) {
			core_clean();
			return 1;
		}
		if (cls() != RLC_OK) {
			core_clean();
			return 1;
		}
		if (pss() != RLC_OK) {
			core_clean();
			return 1;
		}
#if defined(WITH_MPC)
		if (mpss() != RLC_OK) {
			core_clean();
			return 1;
		}
#endif
		if (zss() != RLC_OK) {
			core_clean();
			return 1;
		}
		if (lhs() != RLC_OK) {
			core_clean();
			return 1;
		}
	}
#endif
	util_banner("All tests have passed.\n", 0);
	core_clean();