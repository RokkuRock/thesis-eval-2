struct qmp *qmp_get(struct device *dev)
{
	struct platform_device *pdev;
	struct device_node *np;
	struct qmp *qmp;
	if (!dev || !dev->of_node)
		return ERR_PTR(-EINVAL);
	np = of_parse_phandle(dev->of_node, "qcom,qmp", 0);
	if (!np)
		return ERR_PTR(-ENODEV);
	pdev = of_find_device_by_node(np);
	of_node_put(np);
	if (!pdev)
		return ERR_PTR(-EINVAL);
	qmp = platform_get_drvdata(pdev);
	return qmp ? qmp : ERR_PTR(-EPROBE_DEFER);
}