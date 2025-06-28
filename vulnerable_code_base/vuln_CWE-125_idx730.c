static int uas_use_uas_driver(struct usb_interface *intf,
			      const struct usb_device_id *id,
			      unsigned long *flags_ret)
{
	struct usb_host_endpoint *eps[4] = { };
	struct usb_device *udev = interface_to_usbdev(intf);
	struct usb_hcd *hcd = bus_to_hcd(udev->bus);
	unsigned long flags = id->driver_info;
	int r, alt;
	alt = uas_find_uas_alt_setting(intf);
	if (alt < 0)
		return 0;
	r = uas_find_endpoints(&intf->altsetting[alt], eps);
	if (r < 0)
		return 0;
	if (le16_to_cpu(udev->descriptor.idVendor) == 0x174c &&
			(le16_to_cpu(udev->descriptor.idProduct) == 0x5106 ||
			 le16_to_cpu(udev->descriptor.idProduct) == 0x55aa)) {
		if (udev->actconfig->desc.bMaxPower == 0) {
		} else if (udev->speed < USB_SPEED_SUPER) {
			flags |= US_FL_IGNORE_UAS;
		} else if (usb_ss_max_streams(&eps[1]->ss_ep_comp) == 32) {
			flags |= US_FL_IGNORE_UAS;
		} else {
			flags |= US_FL_MAX_SECTORS_240;
		}
	}
	usb_stor_adjust_quirks(udev, &flags);
	if (flags & US_FL_IGNORE_UAS) {
		dev_warn(&udev->dev,
			"UAS is blacklisted for this device, using usb-storage instead\n");
		return 0;
	}
	if (udev->bus->sg_tablesize == 0) {
		dev_warn(&udev->dev,
			"The driver for the USB controller %s does not support scatter-gather which is\n",
			hcd->driver->description);
		dev_warn(&udev->dev,
			"required by the UAS driver. Please try an other USB controller if you wish to use UAS.\n");
		return 0;
	}
	if (udev->speed >= USB_SPEED_SUPER && !hcd->can_do_streams) {
		dev_warn(&udev->dev,
			"USB controller %s does not support streams, which are required by the UAS driver.\n",
			hcd_to_bus(hcd)->bus_name);
		dev_warn(&udev->dev,
			"Please try an other USB controller if you wish to use UAS.\n");
		return 0;
	}
	if (flags_ret)
		*flags_ret = flags;
	return 1;
}