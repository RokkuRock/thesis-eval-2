expr_context_name(expr_context_ty ctx)
{
    switch (ctx) {
    case Load:
        return "Load";
    case Store:
        return "Store";
    case Del:
        return "Del";
    case AugLoad:
        return "AugLoad";
    case AugStore:
        return "AugStore";
    case Param:
        return "Param";
    default:
        assert(0);
        return "(unknown)";
    }
}