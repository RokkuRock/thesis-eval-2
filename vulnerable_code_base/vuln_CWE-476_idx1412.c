static void i8042_stop(struct serio *serio)
{
	struct i8042_port *port = serio->port_data;
	port->exists = false;
	synchronize_irq(I8042_AUX_IRQ);
	synchronize_irq(I8042_KBD_IRQ);
	port->serio = NULL;
}