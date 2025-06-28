int pdf_load_xrefs(FILE *fp, pdf_t *pdf)
{
    int  i, ver, is_linear;
    long pos, pos_count;
    char x, *c, buf[256];
    c = NULL;
    pdf->n_xrefs = 0;
    fseek(fp, 0, SEEK_SET);
    while (get_next_eof(fp) >= 0)
      ++pdf->n_xrefs;
    if (!pdf->n_xrefs)
      return 0;
    fseek(fp, 0, SEEK_SET);
    pdf->xrefs = calloc(1, sizeof(xref_t) * pdf->n_xrefs);
    ver = 1;
    for (i=0; i<pdf->n_xrefs; i++)
    {
        if ((pos = get_next_eof(fp)) < 0)
          break;
        pdf->xrefs[i].version = ver++;
        pos_count = 0;
        while (SAFE_F(fp, ((x = fgetc(fp)) != 'f')))
          fseek(fp, pos - (++pos_count), SEEK_SET);
        if (pos_count >= sizeof(buf)) {
          ERR("Failed to locate the startxref token. "
              "This might be a corrupt PDF.\n");
          return -1;
        }
        memset(buf, 0, sizeof(buf));
        SAFE_E(fread(buf, 1, pos_count, fp), pos_count,
               "Failed to read startxref.\n");
        c = buf;
        while (*c == ' ' || *c == '\n' || *c == '\r')
          ++c;
        pdf->xrefs[i].start = atol(c);
        if (pdf->xrefs[i].start == 0)
          get_xref_linear_skipped(fp, &pdf->xrefs[i]);
        else
        {
            pos = ftell(fp);
            fseek(fp, pdf->xrefs[i].start, SEEK_SET);
            pdf->xrefs[i].end = get_next_eof(fp);
            fseek(fp, pos, SEEK_SET);
        }
        if (!is_valid_xref(fp, pdf, &pdf->xrefs[i]))
        {
            is_linear = pdf->xrefs[i].is_linear;
            memset(&pdf->xrefs[i], 0, sizeof(xref_t));
            pdf->xrefs[i].is_linear = is_linear;
            rewind(fp);
            get_next_eof(fp);
            continue;
        }
        load_xref_entries(fp, &pdf->xrefs[i]);
    }
    if (pdf->xrefs[0].is_linear)
      resolve_linearized_pdf(pdf);
    load_creator(fp, pdf);
    return pdf->n_xrefs;
}