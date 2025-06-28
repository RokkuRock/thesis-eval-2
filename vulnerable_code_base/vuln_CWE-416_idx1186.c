static int snd_ctl_elem_read_user(struct snd_card *card,
				  struct snd_ctl_elem_value __user *_control)
{
	struct snd_ctl_elem_value *control;
	int result;
	control = memdup_user(_control, sizeof(*control));
	if (IS_ERR(control))
		return PTR_ERR(control);
	snd_power_lock(card);
	result = snd_power_wait(card, SNDRV_CTL_POWER_D0);
	if (result >= 0)
		result = snd_ctl_elem_read(card, control);
	snd_power_unlock(card);
	if (result >= 0)
		if (copy_to_user(_control, control, sizeof(*control)))
			result = -EFAULT;
	kfree(control);
	return result;
}