static int snd_ctl_elem_read(struct snd_card *card,
			     struct snd_ctl_elem_value *control)
{
	struct snd_kcontrol *kctl;
	struct snd_kcontrol_volatile *vd;
	unsigned int index_offset;
	int result;
	down_read(&card->controls_rwsem);
	kctl = snd_ctl_find_id(card, &control->id);
	if (kctl == NULL) {
		result = -ENOENT;
	} else {
		index_offset = snd_ctl_get_ioff(kctl, &control->id);
		vd = &kctl->vd[index_offset];
		if ((vd->access & SNDRV_CTL_ELEM_ACCESS_READ) &&
		    kctl->get != NULL) {
			snd_ctl_build_ioff(&control->id, kctl, index_offset);
			result = kctl->get(kctl, control);
		} else
			result = -EPERM;
	}
	up_read(&card->controls_rwsem);
	return result;
}