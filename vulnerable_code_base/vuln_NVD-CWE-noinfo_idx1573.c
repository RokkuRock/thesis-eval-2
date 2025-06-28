static int dwc3_qcom_acpi_register_core(struct platform_device *pdev)
{
	struct dwc3_qcom	*qcom = platform_get_drvdata(pdev);
	struct device		*dev = &pdev->dev;
	struct resource		*res, *child_res = NULL;
	struct platform_device	*pdev_irq = qcom->urs_usb ? qcom->urs_usb :
							    pdev;
	int			irq;
	int			ret;
	qcom->dwc3 = platform_device_alloc("dwc3", PLATFORM_DEVID_AUTO);
	if (!qcom->dwc3)
		return -ENOMEM;
	qcom->dwc3->dev.parent = dev;
	qcom->dwc3->dev.type = dev->type;
	qcom->dwc3->dev.dma_mask = dev->dma_mask;
	qcom->dwc3->dev.dma_parms = dev->dma_parms;
	qcom->dwc3->dev.coherent_dma_mask = dev->coherent_dma_mask;
	child_res = kcalloc(2, sizeof(*child_res), GFP_KERNEL);
	if (!child_res)
		return -ENOMEM;
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "failed to get memory resource\n");
		ret = -ENODEV;
		goto out;
	}
	child_res[0].flags = res->flags;
	child_res[0].start = res->start;
	child_res[0].end = child_res[0].start +
		qcom->acpi_pdata->dwc3_core_base_size;
	irq = platform_get_irq(pdev_irq, 0);
	if (irq < 0) {
		ret = irq;
		goto out;
	}
	child_res[1].flags = IORESOURCE_IRQ;
	child_res[1].start = child_res[1].end = irq;
	ret = platform_device_add_resources(qcom->dwc3, child_res, 2);
	if (ret) {
		dev_err(&pdev->dev, "failed to add resources\n");
		goto out;
	}
	ret = device_add_software_node(&qcom->dwc3->dev, &dwc3_qcom_swnode);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to add properties\n");
		goto out;
	}
	ret = platform_device_add(qcom->dwc3);
	if (ret) {
		dev_err(&pdev->dev, "failed to add device\n");
		device_remove_software_node(&qcom->dwc3->dev);
	}
out:
	kfree(child_res);
	return ret;
}