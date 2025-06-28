print_line_header(Dwarf_Debug dbg,
    Dwarf_Bool is_single_tab,
    Dwarf_Bool is_actuals_tab)
{
if (!is_single_tab) {
if (is_actuals_tab) {
_dwarf_printf(dbg,"\nActuals Table\n");
_dwarf_printf(dbg,
"                                                         be\n"
"                                                         ls\n"
"                                                         ce\n"
" section    op                                           kq\n"
" offset     code                address/index    row isa ??\n");
    return;
} else {
_dwarf_printf(dbg,"\nLogicals Table\n");
_dwarf_printf(dbg,
"                                                                              s pe\n"
"                                                                              tirp\n"
"                                                                              msoi\n"
" section          op                                                          tall\n"
" offset      row  code                address/indx fil lne col disc cntx subp ????\n");
    return;
}
}
_dwarf_printf(dbg,
"                                                         s b e p e i d\n"
"                                                         t l s r p s i\n"
"                                                         m c e o i a s\n"
" section    op                                       col t k q l l   c\n"
" offset     code               address     file line umn ? ? ? ? ?\n");
}  