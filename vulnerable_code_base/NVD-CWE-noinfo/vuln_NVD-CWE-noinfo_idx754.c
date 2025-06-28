static int orinoco_ioctl_set_auth(struct net_device *dev,
				  struct iw_request_info *info,
				  union iwreq_data *wrqu, char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	hermes_t *hw = &priv->hw;
	struct iw_param *param = &wrqu->param;
	unsigned long flags;
	int ret = -EINPROGRESS;
	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;
	switch (param->flags & IW_AUTH_INDEX) {
	case IW_AUTH_WPA_VERSION:
	case IW_AUTH_CIPHER_PAIRWISE:
	case IW_AUTH_CIPHER_GROUP:
	case IW_AUTH_RX_UNENCRYPTED_EAPOL:
	case IW_AUTH_PRIVACY_INVOKED:
	case IW_AUTH_DROP_UNENCRYPTED:
		break;
	case IW_AUTH_KEY_MGMT:
		priv->key_mgmt = param->value;
		break;
	case IW_AUTH_TKIP_COUNTERMEASURES:
		if (param->value) {
			priv->tkip_cm_active = 1;
			ret = hermes_enable_port(hw, 0);
		} else {
			priv->tkip_cm_active = 0;
			ret = hermes_disable_port(hw, 0);
		}
		break;
	case IW_AUTH_80211_AUTH_ALG:
		if (param->value & IW_AUTH_ALG_SHARED_KEY)
			priv->wep_restrict = 1;
		else if (param->value & IW_AUTH_ALG_OPEN_SYSTEM)
			priv->wep_restrict = 0;
		else
			ret = -EINVAL;
		break;
	case IW_AUTH_WPA_ENABLED:
		if (priv->has_wpa) {
			priv->wpa_enabled = param->value ? 1 : 0;
		} else {
			if (param->value)
				ret = -EOPNOTSUPP;
			priv->wpa_enabled = 0;
		}
		break;
	default:
		ret = -EOPNOTSUPP;
	}
	orinoco_unlock(priv, &flags);
	return ret;
}