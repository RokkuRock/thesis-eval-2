void luaV_execute (lua_State *L, CallInfo *ci) {
  LClosure *cl;
  TValue *k;
  StkId base;
  const Instruction *pc;
  int trap;
#if LUA_USE_JUMPTABLE
#include "ljumptab.h"
#endif
 tailcall:
  trap = L->hookmask;
  cl = clLvalue(s2v(ci->func));
  k = cl->p->k;
  pc = ci->u.l.savedpc;
  if (trap) {
    if (cl->p->is_vararg)
      trap = 0;   
    else if (pc == cl->p->code)   
      luaD_hookcall(L, ci);
    ci->u.l.trap = 1;   
  }
  base = ci->func + 1;
  for (;;) {
    Instruction i;   
    StkId ra;   
    vmfetch();
    lua_assert(base == ci->func + 1);
    lua_assert(base <= L->top && L->top < L->stack + L->stacksize);
    lua_assert(isIT(i) || (cast_void(L->top = base), 1));
    vmdispatch (GET_OPCODE(i)) {
      vmcase(OP_MOVE) {
        setobjs2s(L, ra, RB(i));
        vmbreak;
      }
      vmcase(OP_LOADI) {
        lua_Integer b = GETARG_sBx(i);
        setivalue(s2v(ra), b);
        vmbreak;
      }
      vmcase(OP_LOADF) {
        int b = GETARG_sBx(i);
        setfltvalue(s2v(ra), cast_num(b));
        vmbreak;
      }
      vmcase(OP_LOADK) {
        TValue *rb = k + GETARG_Bx(i);
        setobj2s(L, ra, rb);
        vmbreak;
      }
      vmcase(OP_LOADKX) {
        TValue *rb;
        rb = k + GETARG_Ax(*pc); pc++;
        setobj2s(L, ra, rb);
        vmbreak;
      }
      vmcase(OP_LOADFALSE) {
        setbfvalue(s2v(ra));
        vmbreak;
      }
      vmcase(OP_LFALSESKIP) {
        setbfvalue(s2v(ra));
        pc++;   
        vmbreak;
      }
      vmcase(OP_LOADTRUE) {
        setbtvalue(s2v(ra));
        vmbreak;
      }
      vmcase(OP_LOADNIL) {
        int b = GETARG_B(i);
        do {
          setnilvalue(s2v(ra++));
        } while (b--);
        vmbreak;
      }
      vmcase(OP_GETUPVAL) {
        int b = GETARG_B(i);
        setobj2s(L, ra, cl->upvals[b]->v);
        vmbreak;
      }
      vmcase(OP_SETUPVAL) {
        UpVal *uv = cl->upvals[GETARG_B(i)];
        setobj(L, uv->v, s2v(ra));
        luaC_barrier(L, uv, s2v(ra));
        vmbreak;
      }
      vmcase(OP_GETTABUP) {
        const TValue *slot;
        TValue *upval = cl->upvals[GETARG_B(i)]->v;
        TValue *rc = KC(i);
        TString *key = tsvalue(rc);   
        if (luaV_fastget(L, upval, key, slot, luaH_getshortstr)) {
          setobj2s(L, ra, slot);
        }
        else
          Protect(luaV_finishget(L, upval, rc, ra, slot));
        vmbreak;
      }
      vmcase(OP_GETTABLE) {
        const TValue *slot;
        TValue *rb = vRB(i);
        TValue *rc = vRC(i);
        lua_Unsigned n;
        if (ttisinteger(rc)   
            ? (cast_void(n = ivalue(rc)), luaV_fastgeti(L, rb, n, slot))
            : luaV_fastget(L, rb, rc, slot, luaH_get)) {
          setobj2s(L, ra, slot);
        }
        else
          Protect(luaV_finishget(L, rb, rc, ra, slot));
        vmbreak;
      }
      vmcase(OP_GETI) {
        const TValue *slot;
        TValue *rb = vRB(i);
        int c = GETARG_C(i);
        if (luaV_fastgeti(L, rb, c, slot)) {
          setobj2s(L, ra, slot);
        }
        else {
          TValue key;
          setivalue(&key, c);
          Protect(luaV_finishget(L, rb, &key, ra, slot));
        }
        vmbreak;
      }
      vmcase(OP_GETFIELD) {
        const TValue *slot;
        TValue *rb = vRB(i);
        TValue *rc = KC(i);
        TString *key = tsvalue(rc);   
        if (luaV_fastget(L, rb, key, slot, luaH_getshortstr)) {
          setobj2s(L, ra, slot);
        }
        else
          Protect(luaV_finishget(L, rb, rc, ra, slot));
        vmbreak;
      }
      vmcase(OP_SETTABUP) {
        const TValue *slot;
        TValue *upval = cl->upvals[GETARG_A(i)]->v;
        TValue *rb = KB(i);
        TValue *rc = RKC(i);
        TString *key = tsvalue(rb);   
        if (luaV_fastget(L, upval, key, slot, luaH_getshortstr)) {
          luaV_finishfastset(L, upval, slot, rc);
        }
        else
          Protect(luaV_finishset(L, upval, rb, rc, slot));
        vmbreak;
      }
      vmcase(OP_SETTABLE) {
        const TValue *slot;
        TValue *rb = vRB(i);   
        TValue *rc = RKC(i);   
        lua_Unsigned n;
        if (ttisinteger(rb)   
            ? (cast_void(n = ivalue(rb)), luaV_fastgeti(L, s2v(ra), n, slot))
            : luaV_fastget(L, s2v(ra), rb, slot, luaH_get)) {
          luaV_finishfastset(L, s2v(ra), slot, rc);
        }
        else
          Protect(luaV_finishset(L, s2v(ra), rb, rc, slot));
        vmbreak;
      }
      vmcase(OP_SETI) {
        const TValue *slot;
        int c = GETARG_B(i);
        TValue *rc = RKC(i);
        if (luaV_fastgeti(L, s2v(ra), c, slot)) {
          luaV_finishfastset(L, s2v(ra), slot, rc);
        }
        else {
          TValue key;
          setivalue(&key, c);
          Protect(luaV_finishset(L, s2v(ra), &key, rc, slot));
        }
        vmbreak;
      }
      vmcase(OP_SETFIELD) {
        const TValue *slot;
        TValue *rb = KB(i);
        TValue *rc = RKC(i);
        TString *key = tsvalue(rb);   
        if (luaV_fastget(L, s2v(ra), key, slot, luaH_getshortstr)) {
          luaV_finishfastset(L, s2v(ra), slot, rc);
        }
        else
          Protect(luaV_finishset(L, s2v(ra), rb, rc, slot));
        vmbreak;
      }
      vmcase(OP_NEWTABLE) {
        int b = GETARG_B(i);   
        int c = GETARG_C(i);   
        Table *t;
        if (b > 0)
          b = 1 << (b - 1);   
        lua_assert((!TESTARG_k(i)) == (GETARG_Ax(*pc) == 0));
        if (TESTARG_k(i))   
          c += GETARG_Ax(*pc) * (MAXARG_C + 1);   
        pc++;   
        L->top = ra + 1;   
        t = luaH_new(L);   
        sethvalue2s(L, ra, t);
        if (b != 0 || c != 0)
          luaH_resize(L, t, c, b);   
        checkGC(L, ra + 1);
        vmbreak;
      }
      vmcase(OP_SELF) {
        const TValue *slot;
        TValue *rb = vRB(i);
        TValue *rc = RKC(i);
        TString *key = tsvalue(rc);   
        setobj2s(L, ra + 1, rb);
        if (luaV_fastget(L, rb, key, slot, luaH_getstr)) {
          setobj2s(L, ra, slot);
        }
        else
          Protect(luaV_finishget(L, rb, rc, ra, slot));
        vmbreak;
      }
      vmcase(OP_ADDI) {
        op_arithI(L, l_addi, luai_numadd);
        vmbreak;
      }
      vmcase(OP_ADDK) {
        op_arithK(L, l_addi, luai_numadd);
        vmbreak;
      }
      vmcase(OP_SUBK) {
        op_arithK(L, l_subi, luai_numsub);
        vmbreak;
      }
      vmcase(OP_MULK) {
        op_arithK(L, l_muli, luai_nummul);
        vmbreak;
      }
      vmcase(OP_MODK) {
        op_arithK(L, luaV_mod, luaV_modf);
        vmbreak;
      }
      vmcase(OP_POWK) {
        op_arithfK(L, luai_numpow);
        vmbreak;
      }
      vmcase(OP_DIVK) {
        op_arithfK(L, luai_numdiv);
        vmbreak;
      }
      vmcase(OP_IDIVK) {
        op_arithK(L, luaV_idiv, luai_numidiv);
        vmbreak;
      }
      vmcase(OP_BANDK) {
        op_bitwiseK(L, l_band);
        vmbreak;
      }
      vmcase(OP_BORK) {
        op_bitwiseK(L, l_bor);
        vmbreak;
      }
      vmcase(OP_BXORK) {
        op_bitwiseK(L, l_bxor);
        vmbreak;
      }
      vmcase(OP_SHRI) {
        TValue *rb = vRB(i);
        int ic = GETARG_sC(i);
        lua_Integer ib;
        if (tointegerns(rb, &ib)) {
          pc++; setivalue(s2v(ra), luaV_shiftl(ib, -ic));
        }
        vmbreak;
      }
      vmcase(OP_SHLI) {
        TValue *rb = vRB(i);
        int ic = GETARG_sC(i);
        lua_Integer ib;
        if (tointegerns(rb, &ib)) {
          pc++; setivalue(s2v(ra), luaV_shiftl(ic, ib));
        }
        vmbreak;
      }
      vmcase(OP_ADD) {
        op_arith(L, l_addi, luai_numadd);
        vmbreak;
      }
      vmcase(OP_SUB) {
        op_arith(L, l_subi, luai_numsub);
        vmbreak;
      }
      vmcase(OP_MUL) {
        op_arith(L, l_muli, luai_nummul);
        vmbreak;
      }
      vmcase(OP_MOD) {
        op_arith(L, luaV_mod, luaV_modf);
        vmbreak;
      }
      vmcase(OP_POW) {
        op_arithf(L, luai_numpow);
        vmbreak;
      }
      vmcase(OP_DIV) {   
        op_arithf(L, luai_numdiv);
        vmbreak;
      }
      vmcase(OP_IDIV) {   
        op_arith(L, luaV_idiv, luai_numidiv);
        vmbreak;
      }
      vmcase(OP_BAND) {
        op_bitwise(L, l_band);
        vmbreak;
      }
      vmcase(OP_BOR) {
        op_bitwise(L, l_bor);
        vmbreak;
      }
      vmcase(OP_BXOR) {
        op_bitwise(L, l_bxor);
        vmbreak;
      }
      vmcase(OP_SHR) {
        op_bitwise(L, luaV_shiftr);
        vmbreak;
      }
      vmcase(OP_SHL) {
        op_bitwise(L, luaV_shiftl);
        vmbreak;
      }
      vmcase(OP_MMBIN) {
        Instruction pi = *(pc - 2);   
        TValue *rb = vRB(i);
        TMS tm = (TMS)GETARG_C(i);
        StkId result = RA(pi);
        lua_assert(OP_ADD <= GET_OPCODE(pi) && GET_OPCODE(pi) <= OP_SHR);
        Protect(luaT_trybinTM(L, s2v(ra), rb, result, tm));
        vmbreak;
      }
      vmcase(OP_MMBINI) {
        Instruction pi = *(pc - 2);   
        int imm = GETARG_sB(i);
        TMS tm = (TMS)GETARG_C(i);
        int flip = GETARG_k(i);
        StkId result = RA(pi);
        Protect(luaT_trybiniTM(L, s2v(ra), imm, flip, result, tm));
        vmbreak;
      }
      vmcase(OP_MMBINK) {
        Instruction pi = *(pc - 2);   
        TValue *imm = KB(i);
        TMS tm = (TMS)GETARG_C(i);
        int flip = GETARG_k(i);
        StkId result = RA(pi);
        Protect(luaT_trybinassocTM(L, s2v(ra), imm, flip, result, tm));
        vmbreak;
      }
      vmcase(OP_UNM) {
        TValue *rb = vRB(i);
        lua_Number nb;
        if (ttisinteger(rb)) {
          lua_Integer ib = ivalue(rb);
          setivalue(s2v(ra), intop(-, 0, ib));
        }
        else if (tonumberns(rb, nb)) {
          setfltvalue(s2v(ra), luai_numunm(L, nb));
        }
        else
          Protect(luaT_trybinTM(L, rb, rb, ra, TM_UNM));
        vmbreak;
      }
      vmcase(OP_BNOT) {
        TValue *rb = vRB(i);
        lua_Integer ib;
        if (tointegerns(rb, &ib)) {
          setivalue(s2v(ra), intop(^, ~l_castS2U(0), ib));
        }
        else
          Protect(luaT_trybinTM(L, rb, rb, ra, TM_BNOT));
        vmbreak;
      }
      vmcase(OP_NOT) {
        TValue *rb = vRB(i);
        if (l_isfalse(rb))
          setbtvalue(s2v(ra));
        else
          setbfvalue(s2v(ra));
        vmbreak;
      }
      vmcase(OP_LEN) {
        Protect(luaV_objlen(L, ra, vRB(i)));
        vmbreak;
      }
      vmcase(OP_CONCAT) {
        int n = GETARG_B(i);   
        L->top = ra + n;   
        ProtectNT(luaV_concat(L, n));
        checkGC(L, L->top);  
        vmbreak;
      }
      vmcase(OP_CLOSE) {
        Protect(luaF_close(L, ra, LUA_OK));
        vmbreak;
      }
      vmcase(OP_TBC) {
        halfProtect(luaF_newtbcupval(L, ra));
        vmbreak;
      }
      vmcase(OP_JMP) {
        dojump(ci, i, 0);
        vmbreak;
      }
      vmcase(OP_EQ) {
        int cond;
        TValue *rb = vRB(i);
        Protect(cond = luaV_equalobj(L, s2v(ra), rb));
        docondjump();
        vmbreak;
      }
      vmcase(OP_LT) {
        op_order(L, l_lti, LTnum, lessthanothers);
        vmbreak;
      }
      vmcase(OP_LE) {
        op_order(L, l_lei, LEnum, lessequalothers);
        vmbreak;
      }
      vmcase(OP_EQK) {
        TValue *rb = KB(i);
        int cond = luaV_rawequalobj(s2v(ra), rb);
        docondjump();
        vmbreak;
      }
      vmcase(OP_EQI) {
        int cond;
        int im = GETARG_sB(i);
        if (ttisinteger(s2v(ra)))
          cond = (ivalue(s2v(ra)) == im);
        else if (ttisfloat(s2v(ra)))
          cond = luai_numeq(fltvalue(s2v(ra)), cast_num(im));
        else
          cond = 0;   
        docondjump();
        vmbreak;
      }
      vmcase(OP_LTI) {
        op_orderI(L, l_lti, luai_numlt, 0, TM_LT);
        vmbreak;
      }
      vmcase(OP_LEI) {
        op_orderI(L, l_lei, luai_numle, 0, TM_LE);
        vmbreak;
      }
      vmcase(OP_GTI) {
        op_orderI(L, l_gti, luai_numgt, 1, TM_LT);
        vmbreak;
      }
      vmcase(OP_GEI) {
        op_orderI(L, l_gei, luai_numge, 1, TM_LE);
        vmbreak;
      }
      vmcase(OP_TEST) {
        int cond = !l_isfalse(s2v(ra));
        docondjump();
        vmbreak;
      }
      vmcase(OP_TESTSET) {
        TValue *rb = vRB(i);
        if (l_isfalse(rb) == GETARG_k(i))
          pc++;
        else {
          setobj2s(L, ra, rb);
          donextjump(ci);
        }
        vmbreak;
      }
      vmcase(OP_CALL) {
        int b = GETARG_B(i);
        int nresults = GETARG_C(i) - 1;
        if (b != 0)   
          L->top = ra + b;   
        ProtectNT(luaD_call(L, ra, nresults));
        vmbreak;
      }
      vmcase(OP_TAILCALL) {
        int b = GETARG_B(i);   
        int nparams1 = GETARG_C(i);
        int delta = (nparams1) ? ci->u.l.nextraargs + nparams1 : 0;
        if (b != 0)
          L->top = ra + b;
        else   
          b = cast_int(L->top - ra);
        savepc(ci);   
        if (TESTARG_k(i)) {
          luaF_close(L, base, NOCLOSINGMETH);
          lua_assert(base == ci->func + 1);
        }
        while (!ttisfunction(s2v(ra))) {   
          luaD_tryfuncTM(L, ra);   
          b++;   
          checkstackGCp(L, 1, ra);
        }
        if (!ttisLclosure(s2v(ra))) {   
          luaD_call(L, ra, LUA_MULTRET);   
          updatetrap(ci);
          updatestack(ci);   
          ci->func -= delta;
          luaD_poscall(L, ci, cast_int(L->top - ra));
          return;
        }
        ci->func -= delta;
        luaD_pretailcall(L, ci, ra, b);   
        goto tailcall;
      }
      vmcase(OP_RETURN) {
        int n = GETARG_B(i) - 1;   
        int nparams1 = GETARG_C(i);
        if (n < 0)   
          n = cast_int(L->top - ra);   
        savepc(ci);
        if (TESTARG_k(i)) {   
          if (L->top < ci->top)
            L->top = ci->top;
          luaF_close(L, base, LUA_OK);
          updatetrap(ci);
          updatestack(ci);
        }
        if (nparams1)   
          ci->func -= ci->u.l.nextraargs + nparams1;
        L->top = ra + n;   
        luaD_poscall(L, ci, n);
        return;
      }
      vmcase(OP_RETURN0) {
        if (L->hookmask) {
          L->top = ra;
          halfProtectNT(luaD_poscall(L, ci, 0));   
        }
        else {   
          int nres = ci->nresults;
          L->ci = ci->previous;   
          L->top = base - 1;
          while (nres-- > 0)
            setnilvalue(s2v(L->top++));   
        }
        return;
      }
      vmcase(OP_RETURN1) {
        if (L->hookmask) {
          L->top = ra + 1;
          halfProtectNT(luaD_poscall(L, ci, 1));   
        }
        else {   
          int nres = ci->nresults;
          L->ci = ci->previous;   
          if (nres == 0)
            L->top = base - 1;   
          else {
            setobjs2s(L, base - 1, ra);   
            L->top = base;
            while (--nres > 0)   
              setnilvalue(s2v(L->top++));
          }
        }
        return;
      }
      vmcase(OP_FORLOOP) {
        if (ttisinteger(s2v(ra + 2))) {   
          lua_Unsigned count = l_castS2U(ivalue(s2v(ra + 1)));
          if (count > 0) {   
            lua_Integer step = ivalue(s2v(ra + 2));
            lua_Integer idx = ivalue(s2v(ra));   
            chgivalue(s2v(ra + 1), count - 1);   
            idx = intop(+, idx, step);   
            chgivalue(s2v(ra), idx);   
            setivalue(s2v(ra + 3), idx);   
            pc -= GETARG_Bx(i);   
          }
        }
        else if (floatforloop(ra))   
          pc -= GETARG_Bx(i);   
        updatetrap(ci);   
        vmbreak;
      }
      vmcase(OP_FORPREP) {
        savestate(L, ci);   
        if (forprep(L, ra))
          pc += GETARG_Bx(i) + 1;   
        vmbreak;
      }
      vmcase(OP_TFORPREP) {
        halfProtect(luaF_newtbcupval(L, ra + 3));
        pc += GETARG_Bx(i);
        i = *(pc++);   
        lua_assert(GET_OPCODE(i) == OP_TFORCALL && ra == RA(i));
        goto l_tforcall;
      }
      vmcase(OP_TFORCALL) {
       l_tforcall:
        memcpy(ra + 4, ra, 3 * sizeof(*ra));
        L->top = ra + 4 + 3;
        ProtectNT(luaD_call(L, ra + 4, GETARG_C(i)));   
        updatestack(ci);   
        i = *(pc++);   
        lua_assert(GET_OPCODE(i) == OP_TFORLOOP && ra == RA(i));
        goto l_tforloop;
      }
      vmcase(OP_TFORLOOP) {
        l_tforloop:
        if (!ttisnil(s2v(ra + 4))) {   
          setobjs2s(L, ra + 2, ra + 4);   
          pc -= GETARG_Bx(i);   
        }
        vmbreak;
      }
      vmcase(OP_SETLIST) {
        int n = GETARG_B(i);
        unsigned int last = GETARG_C(i);
        Table *h = hvalue(s2v(ra));
        if (n == 0)
          n = cast_int(L->top - ra) - 1;   
        else
          L->top = ci->top;   
        last += n;
        if (TESTARG_k(i)) {
          last += GETARG_Ax(*pc) * (MAXARG_C + 1);
          pc++;
        }
        if (last > luaH_realasize(h))   
          luaH_resizearray(L, h, last);   
        for (; n > 0; n--) {
          TValue *val = s2v(ra + n);
          setobj2t(L, &h->array[last - 1], val);
          last--;
          luaC_barrierback(L, obj2gco(h), val);
        }
        vmbreak;
      }
      vmcase(OP_CLOSURE) {
        Proto *p = cl->p->p[GETARG_Bx(i)];
        halfProtect(pushclosure(L, p, cl->upvals, base, ra));
        checkGC(L, ra + 1);
        vmbreak;
      }
      vmcase(OP_VARARG) {
        int n = GETARG_C(i) - 1;   
        Protect(luaT_getvarargs(L, ci, ra, n));
        vmbreak;
      }
      vmcase(OP_VARARGPREP) {
        ProtectNT(luaT_adjustvarargs(L, GETARG_A(i), ci, cl->p));
        if (trap) {
          luaD_hookcall(L, ci);
          L->oldpc = pc + 1;   
        }
        updatebase(ci);   
        vmbreak;
      }
      vmcase(OP_EXTRAARG) {
        lua_assert(0);
        vmbreak;
      }
    }
  }
}