rewrite_elf_program_header (bfd *ibfd, bfd *obfd, bfd_vma maxpagesize)
{
  Elf_Internal_Ehdr *iehdr;
  struct elf_segment_map *map;
  struct elf_segment_map *map_first;
  struct elf_segment_map **pointer_to_map;
  Elf_Internal_Phdr *segment;
  asection *section;
  unsigned int i;
  unsigned int num_segments;
  bool phdr_included = false;
  bool p_paddr_valid;
  struct elf_segment_map *phdr_adjust_seg = NULL;
  unsigned int phdr_adjust_num = 0;
  const struct elf_backend_data *bed;
  unsigned int opb = bfd_octets_per_byte (ibfd, NULL);
  bed = get_elf_backend_data (ibfd);
  iehdr = elf_elfheader (ibfd);
  map_first = NULL;
  pointer_to_map = &map_first;
  num_segments = elf_elfheader (ibfd)->e_phnum;
#define SEGMENT_END(segment, start)					\
  (start + (segment->p_memsz > segment->p_filesz			\
	    ? segment->p_memsz : segment->p_filesz))
#define SECTION_SIZE(section, segment)					\
  (((section->flags & (SEC_HAS_CONTENTS | SEC_THREAD_LOCAL))		\
    != SEC_THREAD_LOCAL || segment->p_type == PT_TLS)			\
   ? section->size : 0)
#define IS_CONTAINED_BY_VMA(section, segment, opb)			\
  (section->vma * (opb) >= segment->p_vaddr				\
   && (section->vma * (opb) + SECTION_SIZE (section, segment)		\
       <= (SEGMENT_END (segment, segment->p_vaddr))))
#define IS_CONTAINED_BY_LMA(section, segment, base, opb)		\
  (section->lma * (opb) >= base						\
   && (section->lma + SECTION_SIZE (section, segment) / (opb) >= section->lma) \
   && (section->lma * (opb) + SECTION_SIZE (section, segment)		\
       <= SEGMENT_END (segment, base)))
#define IS_NOTE(p, s)							\
  (p->p_type == PT_NOTE							\
   && elf_section_type (s) == SHT_NOTE					\
   && (bfd_vma) s->filepos >= p->p_offset				\
   && ((bfd_vma) s->filepos + s->size					\
       <= p->p_offset + p->p_filesz))
#define IS_COREFILE_NOTE(p, s)						\
  (IS_NOTE (p, s)							\
   && bfd_get_format (ibfd) == bfd_core					\
   && s->vma == 0							\
   && s->lma == 0)
#define IS_SOLARIS_PT_INTERP(p, s)					\
  (p->p_vaddr == 0							\
   && p->p_paddr == 0							\
   && p->p_memsz == 0							\
   && p->p_filesz > 0							\
   && (s->flags & SEC_HAS_CONTENTS) != 0				\
   && s->size > 0							\
   && (bfd_vma) s->filepos >= p->p_offset				\
   && ((bfd_vma) s->filepos + s->size					\
       <= p->p_offset + p->p_filesz))
#define IS_SECTION_IN_INPUT_SEGMENT(section, segment, bed, opb)		\
  ((((segment->p_paddr							\
      ? IS_CONTAINED_BY_LMA (section, segment, segment->p_paddr, opb)	\
      : IS_CONTAINED_BY_VMA (section, segment, opb))			\
     && (section->flags & SEC_ALLOC) != 0)				\
    || IS_NOTE (segment, section))					\
   && segment->p_type != PT_GNU_STACK					\
   && (segment->p_type != PT_TLS					\
       || (section->flags & SEC_THREAD_LOCAL))				\
   && (segment->p_type == PT_LOAD					\
       || segment->p_type == PT_TLS					\
       || (section->flags & SEC_THREAD_LOCAL) == 0)			\
   && (segment->p_type != PT_DYNAMIC					\
       || SECTION_SIZE (section, segment) > 0				\
       || (segment->p_paddr						\
	   ? segment->p_paddr != section->lma * (opb)			\
	   : segment->p_vaddr != section->vma * (opb))			\
       || (strcmp (bfd_section_name (section), ".dynamic") == 0))	\
   && (segment->p_type != PT_LOAD || !section->segment_mark))
#define INCLUDE_SECTION_IN_SEGMENT(section, segment, bed, opb)		\
  (IS_SECTION_IN_INPUT_SEGMENT (section, segment, bed, opb)		\
   && section->output_section != NULL)
#define SEGMENT_AFTER_SEGMENT(seg1, seg2, field)			\
  (seg1->field >= SEGMENT_END (seg2, seg2->field))
#define SEGMENT_OVERLAPS(seg1, seg2)					\
  (   !(SEGMENT_AFTER_SEGMENT (seg1, seg2, p_vaddr)			\
	|| SEGMENT_AFTER_SEGMENT (seg2, seg1, p_vaddr))			\
   && !(SEGMENT_AFTER_SEGMENT (seg1, seg2, p_paddr)			\
	|| SEGMENT_AFTER_SEGMENT (seg2, seg1, p_paddr)))
  for (section = ibfd->sections; section != NULL; section = section->next)
    {
      asection *o = section->output_section;
      if (o != NULL && o->alignment_power >= (sizeof (bfd_vma) * 8) - 1)
	o->alignment_power = 0;
      section->segment_mark = false;
    }
  p_paddr_valid = false;
  for (i = 0, segment = elf_tdata (ibfd)->phdr;
       i < num_segments;
       i++, segment++)
    if (segment->p_paddr != 0)
      {
	p_paddr_valid = true;
	break;
      }
  for (i = 0, segment = elf_tdata (ibfd)->phdr;
       i < num_segments;
       i++, segment++)
    {
      unsigned int j;
      Elf_Internal_Phdr *segment2;
      if (segment->p_type == PT_INTERP)
	for (section = ibfd->sections; section; section = section->next)
	  if (IS_SOLARIS_PT_INTERP (segment, section))
	    {
	      segment->p_vaddr = section->vma * opb;
	      break;
	    }
      if (segment->p_type != PT_LOAD)
	{
	  if (segment->p_type == PT_GNU_RELRO)
	    segment->p_type = PT_NULL;
	  continue;
	}
      for (j = 0, segment2 = elf_tdata (ibfd)->phdr; j < i; j++, segment2++)
	{
	  bfd_signed_vma extra_length;
	  if (segment2->p_type != PT_LOAD
	      || !SEGMENT_OVERLAPS (segment, segment2))
	    continue;
	  if (segment2->p_vaddr < segment->p_vaddr)
	    {
	      extra_length = (SEGMENT_END (segment, segment->p_vaddr)
			      - SEGMENT_END (segment2, segment2->p_vaddr));
	      if (extra_length > 0)
		{
		  segment2->p_memsz += extra_length;
		  segment2->p_filesz += extra_length;
		}
	      segment->p_type = PT_NULL;
	      i = 0;
	      segment = elf_tdata (ibfd)->phdr;
	      break;
	    }
	  else
	    {
	      extra_length = (SEGMENT_END (segment2, segment2->p_vaddr)
			      - SEGMENT_END (segment, segment->p_vaddr));
	      if (extra_length > 0)
		{
		  segment->p_memsz += extra_length;
		  segment->p_filesz += extra_length;
		}
	      segment2->p_type = PT_NULL;
	    }
	}
    }
  for (i = 0, segment = elf_tdata (ibfd)->phdr;
       i < num_segments;
       i++, segment++)
    {
      unsigned int section_count;
      asection **sections;
      asection *output_section;
      unsigned int isec;
      asection *matching_lma;
      asection *suggested_lma;
      unsigned int j;
      size_t amt;
      asection *first_section;
      if (segment->p_type == PT_NULL)
	continue;
      first_section = NULL;
      for (section = ibfd->sections, section_count = 0;
	   section != NULL;
	   section = section->next)
	{
	  if (IS_SECTION_IN_INPUT_SEGMENT (section, segment, bed, opb))
	    {
	      if (first_section == NULL)
		first_section = section;
	      if (section->output_section != NULL)
		++section_count;
	    }
	}
      amt = sizeof (struct elf_segment_map) - sizeof (asection *);
      amt += section_count * sizeof (asection *);
      map = (struct elf_segment_map *) bfd_zalloc (obfd, amt);
      if (map == NULL)
	return false;
      map->next = NULL;
      map->p_type = segment->p_type;
      map->p_flags = segment->p_flags;
      map->p_flags_valid = 1;
      if (map->p_type == PT_LOAD
	  && (ibfd->flags & D_PAGED) != 0
	  && maxpagesize > 1
	  && segment->p_align > 1)
	{
	  map->p_align = segment->p_align;
	  if (segment->p_align > maxpagesize)
	    map->p_align = maxpagesize;
	  map->p_align_valid = 1;
	}
      if (!first_section || first_section->output_section != NULL)
	{
	  map->p_paddr = segment->p_paddr;
	  map->p_paddr_valid = p_paddr_valid;
	}
      map->includes_filehdr = (segment->p_offset == 0
			       && segment->p_filesz >= iehdr->e_ehsize);
      map->includes_phdrs = 0;
      if (!phdr_included || segment->p_type != PT_LOAD)
	{
	  map->includes_phdrs =
	    (segment->p_offset <= (bfd_vma) iehdr->e_phoff
	     && (segment->p_offset + segment->p_filesz
		 >= ((bfd_vma) iehdr->e_phoff
		     + iehdr->e_phnum * iehdr->e_phentsize)));
	  if (segment->p_type == PT_LOAD && map->includes_phdrs)
	    phdr_included = true;
	}
      if (section_count == 0)
	{
	  if (segment->p_type == PT_LOAD
	      && (segment->p_filesz > 0 || segment->p_memsz == 0))
	    _bfd_error_handler
	      (_("%pB: warning: empty loadable segment detected"
		 " at vaddr=%#" PRIx64 ", is this intentional?"),
	       ibfd, (uint64_t) segment->p_vaddr);
	  map->p_vaddr_offset = segment->p_vaddr / opb;
	  map->count = 0;
	  *pointer_to_map = map;
	  pointer_to_map = &map->next;
	  continue;
	}
      amt = section_count * sizeof (asection *);
      sections = (asection **) bfd_malloc (amt);
      if (sections == NULL)
	return false;
      isec = 0;
      matching_lma = NULL;
      suggested_lma = NULL;
      for (section = first_section, j = 0;
	   section != NULL;
	   section = section->next)
	{
	  if (INCLUDE_SECTION_IN_SEGMENT (section, segment, bed, opb))
	    {
	      output_section = section->output_section;
	      sections[j++] = section;
	      if (!p_paddr_valid
		  && segment->p_vaddr != 0
		  && !bed->want_p_paddr_set_to_zero
		  && isec == 0
		  && output_section->lma != 0
		  && (align_power (segment->p_vaddr
				   + (map->includes_filehdr
				      ? iehdr->e_ehsize : 0)
				   + (map->includes_phdrs
				      ? iehdr->e_phnum * iehdr->e_phentsize
				      : 0),
				   output_section->alignment_power * opb)
		      == (output_section->vma * opb)))
		map->p_paddr = segment->p_vaddr;
	      if (IS_CONTAINED_BY_LMA (output_section, segment, map->p_paddr,
				       opb)
		  || IS_COREFILE_NOTE (segment, section)
		  || (bed->want_p_paddr_set_to_zero
		      && IS_CONTAINED_BY_VMA (output_section, segment, opb)))
		{
		  if (matching_lma == NULL
		      || output_section->lma < matching_lma->lma)
		    matching_lma = output_section;
		  map->sections[isec++] = output_section;
		}
	      else if (suggested_lma == NULL)
		suggested_lma = output_section;
	      if (j == section_count)
		break;
	    }
	}
      BFD_ASSERT (j == section_count);
      if (isec == section_count)
	{
	  map->count = section_count;
	  *pointer_to_map = map;
	  pointer_to_map = &map->next;
	  if (p_paddr_valid
	      && !bed->want_p_paddr_set_to_zero)
	    {
	      bfd_vma hdr_size = 0;
	      if (map->includes_filehdr)
		hdr_size = iehdr->e_ehsize;
	      if (map->includes_phdrs)
		hdr_size += iehdr->e_phnum * iehdr->e_phentsize;
	      map->p_vaddr_offset = ((map->p_paddr + hdr_size) / opb
				     - matching_lma->lma);
	    }
	  free (sections);
	  continue;
	}
      else
	{
	  if (matching_lma == NULL)
	    matching_lma = suggested_lma;
	  map->p_paddr = matching_lma->lma * opb;
	  if (map->includes_phdrs)
	    {
	      map->p_paddr -= iehdr->e_phnum * iehdr->e_phentsize;
	      phdr_adjust_num = iehdr->e_phnum;
	      phdr_adjust_seg = map;
	    }
	  if (map->includes_filehdr)
	    {
	      bfd_vma align = (bfd_vma) 1 << matching_lma->alignment_power;
	      map->p_paddr -= iehdr->e_ehsize;
	      if (segment->p_align != 0 && segment->p_align < align)
		align = segment->p_align;
	      map->p_paddr &= -(align * opb);
	    }
	}
      isec = 0;
      do
	{
	  map->count = 0;
	  suggested_lma = NULL;
	  for (j = 0; j < section_count; j++)
	    {
	      section = sections[j];
	      if (section == NULL)
		continue;
	      output_section = section->output_section;
	      BFD_ASSERT (output_section != NULL);
	      if (IS_CONTAINED_BY_LMA (output_section, segment, map->p_paddr,
				       opb)
		  || IS_COREFILE_NOTE (segment, section))
		{
		  if (map->count == 0)
		    {
		      if (align_power (map->p_paddr
				       + (map->includes_filehdr
					  ? iehdr->e_ehsize : 0)
				       + (map->includes_phdrs
					  ? iehdr->e_phnum * iehdr->e_phentsize
					  : 0),
				       output_section->alignment_power * opb)
			  != output_section->lma * opb)
			goto sorry;
		    }
		  else
		    {
		      asection *prev_sec;
		      prev_sec = map->sections[map->count - 1];
		      if ((BFD_ALIGN (prev_sec->lma + prev_sec->size,
				      maxpagesize)
			   < BFD_ALIGN (output_section->lma, maxpagesize))
			  || (prev_sec->lma + prev_sec->size
			      > output_section->lma))
			{
			  if (suggested_lma == NULL)
			    suggested_lma = output_section;
			  continue;
			}
		    }
		  map->sections[map->count++] = output_section;
		  ++isec;
		  sections[j] = NULL;
		  if (segment->p_type == PT_LOAD)
		    section->segment_mark = true;
		}
	      else if (suggested_lma == NULL)
		suggested_lma = output_section;
	    }
	  *pointer_to_map = map;
	  pointer_to_map = &map->next;
	  if (isec < section_count)
	    {
	      amt = sizeof (struct elf_segment_map) - sizeof (asection *);
	      amt += section_count * sizeof (asection *);
	      map = (struct elf_segment_map *) bfd_zalloc (obfd, amt);
	      if (map == NULL)
		{
		  free (sections);
		  return false;
		}
	      map->next = NULL;
	      map->p_type = segment->p_type;
	      map->p_flags = segment->p_flags;
	      map->p_flags_valid = 1;
	      map->p_paddr = suggested_lma->lma * opb;
	      map->p_paddr_valid = p_paddr_valid;
	      map->includes_filehdr = 0;
	      map->includes_phdrs = 0;
	    }
	  continue;
	sorry:
	  bfd_set_error (bfd_error_sorry);
	  free (sections);
	  return false;
	}
      while (isec < section_count);
      free (sections);
    }
  elf_seg_map (obfd) = map_first;
  if (phdr_adjust_seg != NULL)
    {
      unsigned int count;
      for (count = 0, map = map_first; map != NULL; map = map->next)
	count++;
      if (count > phdr_adjust_num)
	phdr_adjust_seg->p_paddr
	  -= (count - phdr_adjust_num) * iehdr->e_phentsize;
      for (map = map_first; map != NULL; map = map->next)
	if (map->p_type == PT_PHDR)
	  {
	    bfd_vma adjust
	      = phdr_adjust_seg->includes_filehdr ? iehdr->e_ehsize : 0;
	    map->p_paddr = phdr_adjust_seg->p_paddr + adjust;
	    break;
	  }
    }
#undef SEGMENT_END
#undef SECTION_SIZE
#undef IS_CONTAINED_BY_VMA
#undef IS_CONTAINED_BY_LMA
#undef IS_NOTE
#undef IS_COREFILE_NOTE
#undef IS_SOLARIS_PT_INTERP
#undef IS_SECTION_IN_INPUT_SEGMENT
#undef INCLUDE_SECTION_IN_SEGMENT
#undef SEGMENT_AFTER_SEGMENT
#undef SEGMENT_OVERLAPS
  return true;
}