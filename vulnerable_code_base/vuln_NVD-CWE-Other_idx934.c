static void hub_activate(struct usb_hub *hub, enum hub_activation_type type)
{
	struct usb_device *hdev = hub->hdev;
	struct usb_hcd *hcd;
	int ret;
	int port1;
	int status;
	bool need_debounce_delay = false;
	unsigned delay;
	if (type == HUB_INIT2)
		goto init2;
	if (type == HUB_INIT3)
		goto init3;
	if (type != HUB_RESUME) {
		if (hdev->parent && hub_is_superspeed(hdev)) {
			ret = usb_control_msg(hdev, usb_sndctrlpipe(hdev, 0),
					HUB_SET_DEPTH, USB_RT_HUB,
					hdev->level - 1, 0, NULL, 0,
					USB_CTRL_SET_TIMEOUT);
			if (ret < 0)
				dev_err(hub->intfdev,
						"set hub depth failed\n");
		}
		if (type == HUB_INIT) {
			delay = hub_power_on_good_delay(hub);
			hub_power_on(hub, false);
			INIT_DELAYED_WORK(&hub->init_work, hub_init_func2);
			queue_delayed_work(system_power_efficient_wq,
					&hub->init_work,
					msecs_to_jiffies(delay));
			usb_autopm_get_interface_no_resume(
					to_usb_interface(hub->intfdev));
			return;		 
		} else if (type == HUB_RESET_RESUME) {
			hcd = bus_to_hcd(hdev->bus);
			if (hcd->driver->update_hub_device) {
				ret = hcd->driver->update_hub_device(hcd, hdev,
						&hub->tt, GFP_NOIO);
				if (ret < 0) {
					dev_err(hub->intfdev, "Host not "
							"accepting hub info "
							"update.\n");
					dev_err(hub->intfdev, "LS/FS devices "
							"and hubs may not work "
							"under this hub\n.");
				}
			}
			hub_power_on(hub, true);
		} else {
			hub_power_on(hub, true);
		}
	}
 init2:
	for (port1 = 1; port1 <= hdev->maxchild; ++port1) {
		struct usb_port *port_dev = hub->ports[port1 - 1];
		struct usb_device *udev = port_dev->child;
		u16 portstatus, portchange;
		portstatus = portchange = 0;
		status = hub_port_status(hub, port1, &portstatus, &portchange);
		if (udev || (portstatus & USB_PORT_STAT_CONNECTION))
			dev_dbg(&port_dev->dev, "status %04x change %04x\n",
					portstatus, portchange);
		if ((portstatus & USB_PORT_STAT_ENABLE) && (
				type != HUB_RESUME ||
				!(portstatus & USB_PORT_STAT_CONNECTION) ||
				!udev ||
				udev->state == USB_STATE_NOTATTACHED)) {
			portstatus &= ~USB_PORT_STAT_ENABLE;
			if (!hub_is_superspeed(hdev))
				usb_clear_port_feature(hdev, port1,
						   USB_PORT_FEAT_ENABLE);
		}
		if (portchange & USB_PORT_STAT_C_CONNECTION) {
			need_debounce_delay = true;
			usb_clear_port_feature(hub->hdev, port1,
					USB_PORT_FEAT_C_CONNECTION);
		}
		if (portchange & USB_PORT_STAT_C_ENABLE) {
			need_debounce_delay = true;
			usb_clear_port_feature(hub->hdev, port1,
					USB_PORT_FEAT_C_ENABLE);
		}
		if (portchange & USB_PORT_STAT_C_RESET) {
			need_debounce_delay = true;
			usb_clear_port_feature(hub->hdev, port1,
					USB_PORT_FEAT_C_RESET);
		}
		if ((portchange & USB_PORT_STAT_C_BH_RESET) &&
				hub_is_superspeed(hub->hdev)) {
			need_debounce_delay = true;
			usb_clear_port_feature(hub->hdev, port1,
					USB_PORT_FEAT_C_BH_PORT_RESET);
		}
		if (!(portstatus & USB_PORT_STAT_CONNECTION) ||
				(portchange & USB_PORT_STAT_C_CONNECTION))
			clear_bit(port1, hub->removed_bits);
		if (!udev || udev->state == USB_STATE_NOTATTACHED) {
			if (udev || (portstatus & USB_PORT_STAT_CONNECTION) ||
			    (portstatus & USB_PORT_STAT_OVERCURRENT))
				set_bit(port1, hub->change_bits);
		} else if (portstatus & USB_PORT_STAT_ENABLE) {
			bool port_resumed = (portstatus &
					USB_PORT_STAT_LINK_STATE) ==
				USB_SS_PORT_LS_U0;
			if (portchange || (hub_is_superspeed(hub->hdev) &&
						port_resumed))
				set_bit(port1, hub->change_bits);
		} else if (udev->persist_enabled) {
#ifdef CONFIG_PM
			udev->reset_resume = 1;
#endif
			if (test_bit(port1, hub->power_bits))
				set_bit(port1, hub->change_bits);
		} else {
			usb_set_device_state(udev, USB_STATE_NOTATTACHED);
			set_bit(port1, hub->change_bits);
		}
	}
	if (need_debounce_delay) {
		delay = HUB_DEBOUNCE_STABLE;
		if (type == HUB_INIT2) {
			INIT_DELAYED_WORK(&hub->init_work, hub_init_func3);
			queue_delayed_work(system_power_efficient_wq,
					&hub->init_work,
					msecs_to_jiffies(delay));
			return;		 
		} else {
			msleep(delay);
		}
	}
 init3:
	hub->quiescing = 0;
	status = usb_submit_urb(hub->urb, GFP_NOIO);
	if (status < 0)
		dev_err(hub->intfdev, "activate --> %d\n", status);
	if (hub->has_indicators && blinkenlights)
		queue_delayed_work(system_power_efficient_wq,
				&hub->leds, LED_CYCLE_PERIOD);
	kick_hub_wq(hub);
	if (type <= HUB_INIT3)
		usb_autopm_put_interface_async(to_usb_interface(hub->intfdev));
}