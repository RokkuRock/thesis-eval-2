static int jsR_hasproperty(js_State *J, js_Object *obj, const char *name)
{
	js_Property *ref;
	int k;
	if (obj->type == JS_CARRAY) {
		if (!strcmp(name, "length")) {
			js_pushnumber(J, obj->u.a.length);
			return 1;
		}
	}
	else if (obj->type == JS_CSTRING) {
		if (!strcmp(name, "length")) {
			js_pushnumber(J, obj->u.s.length);
			return 1;
		}
		if (js_isarrayindex(J, name, &k)) {
			if (k >= 0 && k < obj->u.s.length) {
				js_pushrune(J, js_runeat(J, obj->u.s.string, k));
				return 1;
			}
		}
	}
	else if (obj->type == JS_CREGEXP) {
		if (!strcmp(name, "source")) {
			js_pushliteral(J, obj->u.r.source);
			return 1;
		}
		if (!strcmp(name, "global")) {
			js_pushboolean(J, obj->u.r.flags & JS_REGEXP_G);
			return 1;
		}
		if (!strcmp(name, "ignoreCase")) {
			js_pushboolean(J, obj->u.r.flags & JS_REGEXP_I);
			return 1;
		}
		if (!strcmp(name, "multiline")) {
			js_pushboolean(J, obj->u.r.flags & JS_REGEXP_M);
			return 1;
		}
		if (!strcmp(name, "lastIndex")) {
			js_pushnumber(J, obj->u.r.last);
			return 1;
		}
	}
	else if (obj->type == JS_CUSERDATA) {
		if (obj->u.user.has && obj->u.user.has(J, obj->u.user.data, name))
			return 1;
	}
	ref = jsV_getproperty(J, obj, name);
	if (ref) {
		if (ref->getter) {
			js_pushobject(J, ref->getter);
			js_pushobject(J, obj);
			js_call(J, 0);
		} else {
			js_pushvalue(J, ref->value);
		}
		return 1;
	}
	return 0;
}