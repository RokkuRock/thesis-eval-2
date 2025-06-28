static bool split_region(struct uc_struct *uc, MemoryRegion *mr,
                         uint64_t address, size_t size, bool do_delete)
{
    uint8_t *backup;
    uint32_t perms;
    uint64_t begin, end, chunk_end;
    size_t l_size, m_size, r_size;
    RAMBlock *block = NULL;
    bool prealloc = false;
    chunk_end = address + size;
    if (address <= mr->addr && chunk_end >= mr->end) {
        return true;
    }
    if (size == 0) {
        return true;
    }
    if (address >= mr->end || chunk_end <= mr->addr) {
        return false;
    }
    QLIST_FOREACH(block, &uc->ram_list.blocks, next)
    {
        if (block->offset <= mr->addr &&
            block->used_length >= (mr->end - mr->addr)) {
            break;
        }
    }
    if (block == NULL) {
        return false;
    }
    prealloc = !!(block->flags & 1);
    if (block->flags & 1) {
        backup = block->host;
    } else {
        backup = copy_region(uc, mr);
        if (backup == NULL) {
            return false;
        }
    }
    perms = mr->perms;
    begin = mr->addr;
    end = mr->end;
    if (uc_mem_unmap(uc, mr->addr, (size_t)int128_get64(mr->size)) !=
        UC_ERR_OK) {
        goto error;
    }
    if (address < begin) {
        address = begin;
    }
    if (chunk_end > end) {
        chunk_end = end;
    }
    l_size = (size_t)(address - begin);
    r_size = (size_t)(end - chunk_end);
    m_size = (size_t)(chunk_end - address);
    if (l_size > 0) {
        if (!prealloc) {
            if (uc_mem_map(uc, begin, l_size, perms) != UC_ERR_OK) {
                goto error;
            }
            if (uc_mem_write(uc, begin, backup, l_size) != UC_ERR_OK) {
                goto error;
            }
        } else {
            if (uc_mem_map_ptr(uc, begin, l_size, perms, backup) != UC_ERR_OK) {
                goto error;
            }
        }
    }
    if (m_size > 0 && !do_delete) {
        if (!prealloc) {
            if (uc_mem_map(uc, address, m_size, perms) != UC_ERR_OK) {
                goto error;
            }
            if (uc_mem_write(uc, address, backup + l_size, m_size) !=
                UC_ERR_OK) {
                goto error;
            }
        } else {
            if (uc_mem_map_ptr(uc, address, m_size, perms, backup + l_size) !=
                UC_ERR_OK) {
                goto error;
            }
        }
    }
    if (r_size > 0) {
        if (!prealloc) {
            if (uc_mem_map(uc, chunk_end, r_size, perms) != UC_ERR_OK) {
                goto error;
            }
            if (uc_mem_write(uc, chunk_end, backup + l_size + m_size, r_size) !=
                UC_ERR_OK) {
                goto error;
            }
        } else {
            if (uc_mem_map_ptr(uc, chunk_end, r_size, perms,
                               backup + l_size + m_size) != UC_ERR_OK) {
                goto error;
            }
        }
    }
    if (!prealloc) {
        free(backup);
    }
    return true;
error:
    if (!prealloc) {
        free(backup);
    }
    return false;
}