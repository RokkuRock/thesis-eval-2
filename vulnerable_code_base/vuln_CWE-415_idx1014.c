SPL_METHOD(SplDoublyLinkedList, offsetSet)
{
	zval                  *zindex, *value;
	spl_dllist_object     *intern;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "zz", &zindex, &value) == FAILURE) {
		return;
	}
	intern = Z_SPLDLLIST_P(getThis());
	if (Z_TYPE_P(zindex) == IS_NULL) {
		spl_ptr_llist_push(intern->llist, value);
	} else {
		zend_long                   index;
		spl_ptr_llist_element *element;
		index = spl_offset_convert_to_long(zindex);
		if (index < 0 || index >= intern->llist->count) {
			zval_ptr_dtor(value);
			zend_throw_exception(spl_ce_OutOfRangeException, "Offset invalid or out of range", 0);
			return;
		}
		element = spl_ptr_llist_offset(intern->llist, index, intern->flags & SPL_DLLIST_IT_LIFO);
		if (element != NULL) {
			if (intern->llist->dtor) {
				intern->llist->dtor(element);
			}
			zval_ptr_dtor(&element->data);
			ZVAL_COPY_VALUE(&element->data, value);
			if (intern->llist->ctor) {
				intern->llist->ctor(element);
			}
		} else {
			zval_ptr_dtor(value);
			zend_throw_exception(spl_ce_OutOfRangeException, "Offset invalid", 0);
			return;
		}
	}
}  