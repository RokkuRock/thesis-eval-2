static void ToPropertyDescriptor(js_State *J, js_Object *obj, const char *name, js_Object *desc)
{
	int haswritable = 0;
	int hasvalue = 0;
	int enumerable = 0;
	int configurable = 0;
	int writable = 0;
	int atts = 0;
	js_pushobject(J, obj);
	js_pushobject(J, desc);
	if (js_hasproperty(J, -1, "writable")) {
		haswritable = 1;
		writable = js_toboolean(J, -1);
		js_pop(J, 1);
	}
	if (js_hasproperty(J, -1, "enumerable")) {
		enumerable = js_toboolean(J, -1);
		js_pop(J, 1);
	}
	if (js_hasproperty(J, -1, "configurable")) {
		configurable = js_toboolean(J, -1);
		js_pop(J, 1);
	}
	if (js_hasproperty(J, -1, "value")) {
		hasvalue = 1;
		js_setproperty(J, -3, name);
	}
	if (!writable) atts |= JS_READONLY;
	if (!enumerable) atts |= JS_DONTENUM;
	if (!configurable) atts |= JS_DONTCONF;
	if (js_hasproperty(J, -1, "get")) {
		if (haswritable || hasvalue)
			js_typeerror(J, "value/writable and get/set attributes are exclusive");
	} else {
		js_pushundefined(J);
	}
	if (js_hasproperty(J, -2, "set")) {
		if (haswritable || hasvalue)
			js_typeerror(J, "value/writable and get/set attributes are exclusive");
	} else {
		js_pushundefined(J);
	}
	js_defaccessor(J, -4, name, atts);
	js_pop(J, 2);
}