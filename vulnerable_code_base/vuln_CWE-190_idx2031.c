int util_bits_dig(dig_t a) {
    return RLC_DIG - arch_lzcnt(a);
}