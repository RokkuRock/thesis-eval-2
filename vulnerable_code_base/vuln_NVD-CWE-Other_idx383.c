static int gtco_probe(struct usb_interface *usbinterface,
		      const struct usb_device_id *id)
{
	struct gtco             *gtco;
	struct input_dev        *input_dev;
	struct hid_descriptor   *hid_desc;
	char                    *report;
	int                     result = 0, retry;
	int			error;
	struct usb_endpoint_descriptor *endpoint;
	gtco = kzalloc(sizeof(struct gtco), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!gtco || !input_dev) {
		dev_err(&usbinterface->dev, "No more memory\n");
		error = -ENOMEM;
		goto err_free_devs;
	}
	gtco->inputdevice = input_dev;
	gtco->usbdev = interface_to_usbdev(usbinterface);
	gtco->intf = usbinterface;
	gtco->buffer = usb_alloc_coherent(gtco->usbdev, REPORT_MAX_SIZE,
					  GFP_KERNEL, &gtco->buf_dma);
	if (!gtco->buffer) {
		dev_err(&usbinterface->dev, "No more memory for us buffers\n");
		error = -ENOMEM;
		goto err_free_devs;
	}
	gtco->urbinfo = usb_alloc_urb(0, GFP_KERNEL);
	if (!gtco->urbinfo) {
		dev_err(&usbinterface->dev, "Failed to allocate URB\n");
		error = -ENOMEM;
		goto err_free_buf;
	}
	endpoint = &usbinterface->altsetting[0].endpoint[0].desc;
	dev_dbg(&usbinterface->dev, "gtco # interfaces: %d\n", usbinterface->num_altsetting);
	dev_dbg(&usbinterface->dev, "num endpoints:     %d\n", usbinterface->cur_altsetting->desc.bNumEndpoints);
	dev_dbg(&usbinterface->dev, "interface class:   %d\n", usbinterface->cur_altsetting->desc.bInterfaceClass);
	dev_dbg(&usbinterface->dev, "endpoint: attribute:0x%x type:0x%x\n", endpoint->bmAttributes, endpoint->bDescriptorType);
	if (usb_endpoint_xfer_int(endpoint))
		dev_dbg(&usbinterface->dev, "endpoint: we have interrupt endpoint\n");
	dev_dbg(&usbinterface->dev, "endpoint extra len:%d\n", usbinterface->altsetting[0].extralen);
	if (usb_get_extra_descriptor(usbinterface->cur_altsetting,
				     HID_DEVICE_TYPE, &hid_desc) != 0){
		dev_err(&usbinterface->dev,
			"Can't retrieve exta USB descriptor to get hid report descriptor length\n");
		error = -EIO;
		goto err_free_urb;
	}
	dev_dbg(&usbinterface->dev,
		"Extra descriptor success: type:%d  len:%d\n",
		hid_desc->bDescriptorType,  hid_desc->wDescriptorLength);
	report = kzalloc(le16_to_cpu(hid_desc->wDescriptorLength), GFP_KERNEL);
	if (!report) {
		dev_err(&usbinterface->dev, "No more memory for report\n");
		error = -ENOMEM;
		goto err_free_urb;
	}
	for (retry = 0; retry < 3; retry++) {
		result = usb_control_msg(gtco->usbdev,
					 usb_rcvctrlpipe(gtco->usbdev, 0),
					 USB_REQ_GET_DESCRIPTOR,
					 USB_RECIP_INTERFACE | USB_DIR_IN,
					 REPORT_DEVICE_TYPE << 8,
					 0,  
					 report,
					 le16_to_cpu(hid_desc->wDescriptorLength),
					 5000);  
		dev_dbg(&usbinterface->dev, "usb_control_msg result: %d\n", result);
		if (result == le16_to_cpu(hid_desc->wDescriptorLength)) {
			parse_hid_report_descriptor(gtco, report, result);
			break;
		}
	}
	kfree(report);
	if (result != le16_to_cpu(hid_desc->wDescriptorLength)) {
		dev_err(&usbinterface->dev,
			"Failed to get HID Report Descriptor of size: %d\n",
			hid_desc->wDescriptorLength);
		error = -EIO;
		goto err_free_urb;
	}
	usb_make_path(gtco->usbdev, gtco->usbpath, sizeof(gtco->usbpath));
	strlcat(gtco->usbpath, "/input0", sizeof(gtco->usbpath));
	input_dev->open = gtco_input_open;
	input_dev->close = gtco_input_close;
	input_dev->name = "GTCO_CalComp";
	input_dev->phys = gtco->usbpath;
	input_set_drvdata(input_dev, gtco);
	gtco_setup_caps(input_dev);
	usb_to_input_id(gtco->usbdev, &input_dev->id);
	input_dev->dev.parent = &usbinterface->dev;
	endpoint = &usbinterface->altsetting[0].endpoint[0].desc;
	usb_fill_int_urb(gtco->urbinfo,
			 gtco->usbdev,
			 usb_rcvintpipe(gtco->usbdev,
					endpoint->bEndpointAddress),
			 gtco->buffer,
			 REPORT_MAX_SIZE,
			 gtco_urb_callback,
			 gtco,
			 endpoint->bInterval);
	gtco->urbinfo->transfer_dma = gtco->buf_dma;
	gtco->urbinfo->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	usb_set_intfdata(usbinterface, gtco);
	error = input_register_device(input_dev);
	if (error)
		goto err_free_urb;
	return 0;
 err_free_urb:
	usb_free_urb(gtco->urbinfo);
 err_free_buf:
	usb_free_coherent(gtco->usbdev, REPORT_MAX_SIZE,
			  gtco->buffer, gtco->buf_dma);
 err_free_devs:
	input_free_device(input_dev);
	kfree(gtco);
	return error;
}