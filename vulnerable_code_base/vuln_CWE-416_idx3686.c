max3421_select_and_start_urb(struct usb_hcd *hcd)
{
	struct spi_device *spi = to_spi_device(hcd->self.controller);
	struct max3421_hcd *max3421_hcd = hcd_to_max3421(hcd);
	struct urb *urb, *curr_urb = NULL;
	struct max3421_ep *max3421_ep;
	int epnum, force_toggles = 0;
	struct usb_host_endpoint *ep;
	struct list_head *pos;
	unsigned long flags;
	spin_lock_irqsave(&max3421_hcd->lock, flags);
	for (;
	     max3421_hcd->sched_pass < SCHED_PASS_DONE;
	     ++max3421_hcd->sched_pass)
		list_for_each(pos, &max3421_hcd->ep_list) {
			urb = NULL;
			max3421_ep = container_of(pos, struct max3421_ep,
						  ep_list);
			ep = max3421_ep->ep;
			switch (usb_endpoint_type(&ep->desc)) {
			case USB_ENDPOINT_XFER_ISOC:
			case USB_ENDPOINT_XFER_INT:
				if (max3421_hcd->sched_pass !=
				    SCHED_PASS_PERIODIC)
					continue;
				break;
			case USB_ENDPOINT_XFER_CONTROL:
			case USB_ENDPOINT_XFER_BULK:
				if (max3421_hcd->sched_pass !=
				    SCHED_PASS_NON_PERIODIC)
					continue;
				break;
			}
			if (list_empty(&ep->urb_list))
				continue;	 
			urb = list_first_entry(&ep->urb_list, struct urb,
					       urb_list);
			if (urb->unlinked) {
				dev_dbg(&spi->dev, "%s: URB %p unlinked=%d",
					__func__, urb, urb->unlinked);
				max3421_hcd->curr_urb = urb;
				max3421_hcd->urb_done = 1;
				spin_unlock_irqrestore(&max3421_hcd->lock,
						       flags);
				return 1;
			}
			switch (usb_endpoint_type(&ep->desc)) {
			case USB_ENDPOINT_XFER_CONTROL:
				if (frame_diff(max3421_ep->last_active,
					       max3421_hcd->frame_number) == 0)
					continue;
				break;
			case USB_ENDPOINT_XFER_BULK:
				if (max3421_ep->retransmit
				    && (frame_diff(max3421_ep->last_active,
						   max3421_hcd->frame_number)
					== 0))
					continue;
				break;
			case USB_ENDPOINT_XFER_ISOC:
			case USB_ENDPOINT_XFER_INT:
				if (frame_diff(max3421_hcd->frame_number,
					       max3421_ep->last_active)
				    < urb->interval)
					continue;
				break;
			}
			list_move_tail(pos, &max3421_hcd->ep_list);
			curr_urb = urb;
			goto done;
		}
done:
	if (!curr_urb) {
		spin_unlock_irqrestore(&max3421_hcd->lock, flags);
		return 0;
	}
	urb = max3421_hcd->curr_urb = curr_urb;
	epnum = usb_endpoint_num(&urb->ep->desc);
	if (max3421_ep->retransmit)
		max3421_ep->retransmit = 0;
	else {
		if (usb_endpoint_xfer_control(&ep->desc)) {
			usb_settoggle(urb->dev, epnum, 0, 1);
			usb_settoggle(urb->dev, epnum, 1, 1);
			max3421_ep->pkt_state = PKT_STATE_SETUP;
			force_toggles = 1;
		} else
			max3421_ep->pkt_state = PKT_STATE_TRANSFER;
	}
	spin_unlock_irqrestore(&max3421_hcd->lock, flags);
	max3421_ep->last_active = max3421_hcd->frame_number;
	max3421_set_address(hcd, urb->dev, epnum, force_toggles);
	max3421_set_speed(hcd, urb->dev);
	max3421_next_transfer(hcd, 0);
	return 1;
}