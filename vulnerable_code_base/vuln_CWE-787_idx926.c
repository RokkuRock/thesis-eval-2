static void O_getOwnPropertyDescriptor(js_State *J)
{
	js_Object *obj;
	js_Property *ref;
	if (!js_isobject(J, 1))
		js_typeerror(J, "not an object");
	obj = js_toobject(J, 1);
	ref = jsV_getproperty(J, obj, js_tostring(J, 2));
	if (!ref) {
		js_pushundefined(J);
	} else {
		js_newobject(J);
		if (!ref->getter && !ref->setter) {
			js_pushvalue(J, ref->value);
			js_setproperty(J, -2, "value");
			js_pushboolean(J, !(ref->atts & JS_READONLY));
			js_setproperty(J, -2, "writable");
		} else {
			if (ref->getter)
				js_pushobject(J, ref->getter);
			else
				js_pushundefined(J);
			js_setproperty(J, -2, "get");
			if (ref->setter)
				js_pushobject(J, ref->setter);
			else
				js_pushundefined(J);
			js_setproperty(J, -2, "set");
		}
		js_pushboolean(J, !(ref->atts & JS_DONTENUM));
		js_setproperty(J, -2, "enumerable");
		js_pushboolean(J, !(ref->atts & JS_DONTCONF));
		js_setproperty(J, -2, "configurable");
	}
}