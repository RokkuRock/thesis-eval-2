CAMLprim value caml_alloc_dummy_float (value size)
{
  mlsize_t wosize = Int_val(size) * Double_wosize;
  if (wosize == 0) return Atom(0);
  return caml_alloc (wosize, 0);
}