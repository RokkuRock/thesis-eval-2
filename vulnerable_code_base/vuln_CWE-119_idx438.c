static void intern_rec(value *dest)
{
  unsigned int code;
  tag_t tag;
  mlsize_t size, len, ofs_ind;
  value v;
  asize_t ofs;
  header_t header;
  unsigned char digest[16];
  struct custom_operations * ops;
  char * codeptr;
  struct intern_item * sp;
  sp = intern_stack;
  ReadItems(dest, 1);
  while(sp != intern_stack) {
  dest = sp->dest;
  switch (sp->op) {
  case OFreshOID:
    if (Int_val(Field((value)dest, 1)) >= 0)
      caml_set_oo_id((value)dest);
    sp--;
    break;
  case OShift:
    *dest += sp->arg;
    sp--;
    break;
  case OReadItems:
    sp->dest++;
    if (--(sp->arg) == 0) sp--;
  code = read8u();
  if (code >= PREFIX_SMALL_INT) {
    if (code >= PREFIX_SMALL_BLOCK) {
      tag = code & 0xF;
      size = (code >> 4) & 0x7;
    read_block:
      if (size == 0) {
        v = Atom(tag);
      } else {
        v = Val_hp(intern_dest);
        if (intern_obj_table != NULL) intern_obj_table[obj_counter++] = v;
        *intern_dest = Make_header(size, tag, intern_color);
        intern_dest += 1 + size;
        if (tag == Object_tag) {
          Assert(size >= 2);
          ReadItems(&Field(v, 2), size - 2);
          PushItem();
          sp->op = OFreshOID;
          sp->dest = (value*) v;
          sp->arg = 1;
          ReadItems(&Field(v, 0), 2);
        } else
          ReadItems(&Field(v, 0), size);
      }
    } else {
      v = Val_int(code & 0x3F);
    }
  } else {
    if (code >= PREFIX_SMALL_STRING) {
      len = (code & 0x1F);
    read_string:
      size = (len + sizeof(value)) / sizeof(value);
      v = Val_hp(intern_dest);
      if (intern_obj_table != NULL) intern_obj_table[obj_counter++] = v;
      *intern_dest = Make_header(size, String_tag, intern_color);
      intern_dest += 1 + size;
      Field(v, size - 1) = 0;
      ofs_ind = Bsize_wsize(size) - 1;
      Byte(v, ofs_ind) = ofs_ind - len;
      readblock(String_val(v), len);
    } else {
      switch(code) {
      case CODE_INT8:
        v = Val_long(read8s());
        break;
      case CODE_INT16:
        v = Val_long(read16s());
        break;
      case CODE_INT32:
        v = Val_long(read32s());
        break;
      case CODE_INT64:
#ifdef ARCH_SIXTYFOUR
        v = Val_long(read64s());
        break;
#else
        intern_cleanup();
        caml_failwith("input_value: integer too large");
        break;
#endif
      case CODE_SHARED8:
        ofs = read8u();
      read_shared:
        Assert (ofs > 0);
        Assert (ofs <= obj_counter);
        Assert (intern_obj_table != NULL);
        v = intern_obj_table[obj_counter - ofs];
        break;
      case CODE_SHARED16:
        ofs = read16u();
        goto read_shared;
      case CODE_SHARED32:
        ofs = read32u();
        goto read_shared;
      case CODE_BLOCK32:
        header = (header_t) read32u();
        tag = Tag_hd(header);
        size = Wosize_hd(header);
        goto read_block;
      case CODE_BLOCK64:
#ifdef ARCH_SIXTYFOUR
        header = (header_t) read64s();
        tag = Tag_hd(header);
        size = Wosize_hd(header);
        goto read_block;
#else
        intern_cleanup();
        caml_failwith("input_value: data block too large");
        break;
#endif
      case CODE_STRING8:
        len = read8u();
        goto read_string;
      case CODE_STRING32:
        len = read32u();
        goto read_string;
      case CODE_DOUBLE_LITTLE:
      case CODE_DOUBLE_BIG:
        v = Val_hp(intern_dest);
        if (intern_obj_table != NULL) intern_obj_table[obj_counter++] = v;
        *intern_dest = Make_header(Double_wosize, Double_tag, intern_color);
        intern_dest += 1 + Double_wosize;
        readfloat((double *) v, code);
        break;
      case CODE_DOUBLE_ARRAY8_LITTLE:
      case CODE_DOUBLE_ARRAY8_BIG:
        len = read8u();
      read_double_array:
        size = len * Double_wosize;
        v = Val_hp(intern_dest);
        if (intern_obj_table != NULL) intern_obj_table[obj_counter++] = v;
        *intern_dest = Make_header(size, Double_array_tag, intern_color);
        intern_dest += 1 + size;
        readfloats((double *) v, len, code);
        break;
      case CODE_DOUBLE_ARRAY32_LITTLE:
      case CODE_DOUBLE_ARRAY32_BIG:
        len = read32u();
        goto read_double_array;
      case CODE_CODEPOINTER:
        ofs = read32u();
        readblock(digest, 16);
        codeptr = intern_resolve_code_pointer(digest, ofs);
        if (codeptr != NULL) {
          v = (value) codeptr;
        } else {
          value * function_placeholder =
            caml_named_value ("Debugger.function_placeholder");
          if (function_placeholder != NULL) {
            v = *function_placeholder;
          } else {
            intern_cleanup();
            intern_bad_code_pointer(digest);
          }
        }
        break;
      case CODE_INFIXPOINTER:
        ofs = read32u();
        PushItem();
        sp->dest = dest;
        sp->op = OShift;
        sp->arg = ofs;
        ReadItems(dest, 1);
        continue;   
      case CODE_CUSTOM:
        ops = caml_find_custom_operations((char *) intern_src);
        if (ops == NULL) {
          intern_cleanup();
          caml_failwith("input_value: unknown custom block identifier");
        }
        while (*intern_src++ != 0)  ;   
        size = ops->deserialize((void *) (intern_dest + 2));
        size = 1 + (size + sizeof(value) - 1) / sizeof(value);
        v = Val_hp(intern_dest);
        if (intern_obj_table != NULL) intern_obj_table[obj_counter++] = v;
        *intern_dest = Make_header(size, Custom_tag, intern_color);
        Custom_ops_val(v) = ops;
        if (ops->finalize != NULL && Is_young(v)) {
          if (caml_finalize_table.ptr >= caml_finalize_table.limit){
            CAMLassert (caml_finalize_table.ptr == caml_finalize_table.limit);
            caml_realloc_ref_table (&caml_finalize_table);
          }
          *caml_finalize_table.ptr++ = (value *)v;
        }
        intern_dest += 1 + size;
        break;
      default:
        intern_cleanup();
        caml_failwith("input_value: ill-formed message");
      }
    }
  }
  *dest = v;
  break;
  default:
    Assert(0);
  }
  }
  intern_free_stack();
}