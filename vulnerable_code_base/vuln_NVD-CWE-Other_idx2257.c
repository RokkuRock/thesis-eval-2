static int cdc_ncm_bind(struct usbnet *dev, struct usb_interface *intf)
{
	int ret;
	if (cdc_ncm_select_altsetting(intf) != CDC_NCM_COMM_ALTSETTING_NCM)
		return -ENODEV;
	ret = cdc_ncm_bind_common(dev, intf, CDC_NCM_DATA_ALTSETTING_NCM, 0);
	usbnet_link_change(dev, 0, 0);
	return ret;
}