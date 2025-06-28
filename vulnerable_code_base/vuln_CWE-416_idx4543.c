_public_ int sd_bus_enqeue_for_read(sd_bus *bus, sd_bus_message *m) {
        int r;
        assert_return(bus, -EINVAL);
        assert_return(bus = bus_resolve(bus), -ENOPKG);
        assert_return(m, -EINVAL);
        assert_return(m->sealed, -EINVAL);
        assert_return(!bus_pid_changed(bus), -ECHILD);
        if (!BUS_IS_OPEN(bus->state))
                return -ENOTCONN;
        r = bus_rqueue_make_room(bus);
        if (r < 0)
                return r;
        bus->rqueue[bus->rqueue_size++] = bus_message_ref_queued(m, bus);
        return 0;
}