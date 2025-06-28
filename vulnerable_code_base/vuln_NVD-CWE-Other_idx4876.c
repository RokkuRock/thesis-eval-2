static int clie_5_attach(struct usb_serial *serial)
{
	struct usb_serial_port *port;
	unsigned int pipe;
	int j;
	if (serial->num_ports < 2)
		return -1;
	port = serial->port[0];
	port->bulk_out_endpointAddress =
				serial->port[1]->bulk_out_endpointAddress;
	pipe = usb_sndbulkpipe(serial->dev, port->bulk_out_endpointAddress);
	for (j = 0; j < ARRAY_SIZE(port->write_urbs); ++j)
		port->write_urbs[j]->pipe = pipe;
	return 0;
}