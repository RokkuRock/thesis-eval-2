static int virtio_gpu_object_shmem_init(struct virtio_gpu_device *vgdev,
					struct virtio_gpu_object *bo,
					struct virtio_gpu_mem_entry **ents,
					unsigned int *nents)
{
	bool use_dma_api = !virtio_has_dma_quirk(vgdev->vdev);
	struct virtio_gpu_object_shmem *shmem = to_virtio_gpu_shmem(bo);
	struct scatterlist *sg;
	int si, ret;
	ret = drm_gem_shmem_pin(&bo->base);
	if (ret < 0)
		return -EINVAL;
	shmem->pages = drm_gem_shmem_get_sg_table(&bo->base);
	if (!shmem->pages) {
		drm_gem_shmem_unpin(&bo->base);
		return -EINVAL;
	}
	if (use_dma_api) {
		ret = dma_map_sgtable(vgdev->vdev->dev.parent,
				      shmem->pages, DMA_TO_DEVICE, 0);
		if (ret)
			return ret;
		*nents = shmem->mapped = shmem->pages->nents;
	} else {
		*nents = shmem->pages->orig_nents;
	}
	*ents = kvmalloc_array(*nents,
			       sizeof(struct virtio_gpu_mem_entry),
			       GFP_KERNEL);
	if (!(*ents)) {
		DRM_ERROR("failed to allocate ent list\n");
		return -ENOMEM;
	}
	if (use_dma_api) {
		for_each_sgtable_dma_sg(shmem->pages, sg, si) {
			(*ents)[si].addr = cpu_to_le64(sg_dma_address(sg));
			(*ents)[si].length = cpu_to_le32(sg_dma_len(sg));
			(*ents)[si].padding = 0;
		}
	} else {
		for_each_sgtable_sg(shmem->pages, sg, si) {
			(*ents)[si].addr = cpu_to_le64(sg_phys(sg));
			(*ents)[si].length = cpu_to_le32(sg->length);
			(*ents)[si].padding = 0;
		}
	}
	return 0;
}