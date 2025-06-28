static int createFromTiffRgba(TIFF * tif, gdImagePtr im)
{
	int a;
	int x, y;
	int alphaBlendingFlag = 0;
	int color;
	int width = im->sx;
	int height = im->sy;
	uint32 *buffer;
	uint32 rgba;
	alphaBlendingFlag = im->alphaBlendingFlag;
	gdImageAlphaBlending(im, 0);
	buffer = (uint32 *) gdCalloc(sizeof(uint32), width * height);
	if (!buffer) {
		return GD_FAILURE;
	}
	TIFFReadRGBAImage(tif, width, height, buffer, 0);
	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			rgba = buffer[(y * width + x)];
			a = (0xff - TIFFGetA(rgba)) / 2;
			color = gdTrueColorAlpha(TIFFGetR(rgba), TIFFGetG(rgba), TIFFGetB(rgba), a);
			gdImageSetPixel(im, x, height - y - 1, color);
		}
	}
	gdFree(buffer);
	gdImageAlphaBlending(im, alphaBlendingFlag);
	return GD_SUCCESS;
}