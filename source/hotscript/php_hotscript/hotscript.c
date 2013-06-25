/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_hotscript.h"
#include "main/php_output.h"

#ifdef ZTS
#include "TSRM.h"
#endif

#include "ext/standard/php_smart_str.h"

/* If you declare any globals in php_hotscript.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(hotscript)
*/

/* True global resources - no need for thread safety here */
static int le_hotscript;

/* {{{ hotscript_functions[]
 *
 * Every user visible function must have an entry in hotscript_functions[].
 */
const zend_function_entry hotscript_functions[] = {
	PHP_FE(hot_encode,	NULL)
	PHP_FE_END	/* Must be the last line in hotscript_functions[] */
};
/* }}} */

/* {{{ hotscript_module_entry
 */
zend_module_entry hotscript_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"hotscript",
	hotscript_functions,
	PHP_MINIT(hotscript),
	PHP_MSHUTDOWN(hotscript),
	PHP_RINIT(hotscript),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(hotscript),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(hotscript),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_HOTSCRIPT
ZEND_GET_MODULE(hotscript)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("hotscript.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_hotscript_globals, hotscript_globals)
    STD_PHP_INI_ENTRY("hotscript.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_hotscript_globals, hotscript_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_hotscript_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_hotscript_init_globals(zend_hotscript_globals *hotscript_globals)
{
	hotscript_globals->global_value = 0;
	hotscript_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(hotscript)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(hotscript)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(hotscript)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(hotscript)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(hotscript)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "hotscript support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

#include "hotpot/hp_reader.h"
#include "hotpot/hp_error.h"

#define MAX_DEEP 1024
typedef struct tagSTACK_NODE
{
	zval *val;
}STACK_NODE;

typedef struct _ZVALReader
{
	HPAbstractReader super;
	hpuint32 stack_num;
	STACK_NODE stack[MAX_DEEP];
}ZVALReader;

zval* zval_reade_get_zval(ZVALReader *self)
{
	return self->stack[self->stack_num - 1].val;
}

hpint32 zval_reader_begin(HPAbstractReader *super, const HPVar *name)
{
	ZVALReader *self = HP_CONTAINER_OF(super, ZVALReader, super);
	int i, r;
	HashTable *myht;
	zval *zv = zval_reade_get_zval(self);
	HashPosition pos;
	char *key;
	ulong index;
	uint key_len;
	zval **data;

	if (Z_TYPE_P(zv) != IS_ARRAY)
	{		
		return E_HP_ERROR;
	}
	myht = zv->value.ht;
	if(name == NULL)
	{
		i = zend_hash_get_current_key_ex(myht, &key, &key_len, &index, 0, &pos);
		if (i == HASH_KEY_NON_EXISTANT)
		{
			return E_HP_ERROR;
		}

		if (zend_hash_get_current_data_ex(myht, (void **) &data, &pos) != SUCCESS)
		{
			return E_HP_ERROR;
		}

		self->stack[self->stack_num].val = *data;
		++(self->stack_num);

		zend_hash_move_forward_ex(myht, &pos);
	}
	else
	{
		if(zend_hash_find(myht, name->val.str.ptr, name->val.str.len, (void **)&data) != SUCCESS)
		{			
			printf("error!\n");
			return E_HP_ERROR;
		}
		

		self->stack[self->stack_num].val = *data;
		++(self->stack_num);
	}

	return E_HP_NOERROR;
}

hpint32 zval_reader_read(HPAbstractReader* super, HPVar *var)
{
	ZVALReader *self = HP_CONTAINER_OF(super, ZVALReader, super);
	zval *zv = zval_reade_get_zval(self);
	switch (Z_TYPE_P(zv))
	{
	case IS_NULL:
		var->type = E_HP_STRING;
		var->val.str.ptr = "null";
		var->val.str.len = 4;
		break;

	case IS_BOOL:
		if (Z_BVAL_P(zv))
		{
			var->type = E_HP_STRING;
			var->val.str.ptr = "true";
			var->val.str.len = 4;
		}
		else
		{
			var->type = E_HP_STRING;
			var->val.str.ptr = "false";
			var->val.str.len = 5;
		}
		break;

	case IS_LONG:
		{
			char *d = NULL;
			var->type = E_HP_STRING;
			var->val.str.len = spprintf(&d, 0, "%lld", Z_LVAL_P(zv));
			var->val.str.ptr = d;
		}		
		break;

	case IS_DOUBLE:
		{
			char *d = NULL;
			double dbl = Z_DVAL_P(zv);
			var->type = E_HP_STRING;
			if (!zend_isinf(dbl) && !zend_isnan(dbl))
			{
				//var->val.str.len = spprintf(&d, 0, "%.*k", (int) EG(precision), dbl);
				var->val.str.len = spprintf(&d, 0, "%.*k", 14, dbl);
				var->val.str.ptr = d;
			}
			else 
			{
				var->val.str.len = 1;
				var->val.str.ptr = "0";
			}
		}
		break;

	case IS_STRING:
		{
			var->type = E_HP_STRING;
			var->val.str.ptr = Z_STRVAL_P(zv);
			var->val.str.len = Z_STRLEN_P(zv);
			break;
		}
	default:
		return E_HP_ERROR;
	}

	return E_HP_NOERROR;
}

hpint32 zval_reader_end(HPAbstractReader *super)
{
	ZVALReader *self = HP_CONTAINER_OF(super, ZVALReader, super);
	--(self->stack_num);
}


void hot_encode_ex(smart_str *buf, zval *val TSRMLS_DC);

void hot_encode_array(smart_str *buf, zval **val TSRMLS_DC)
{
	int i, r;
	HashTable *myht;

	if (Z_TYPE_PP(val) == IS_ARRAY) {
		myht = HASH_OF(*val);
	} else {
		myht = Z_OBJPROP_PP(val);
	}

	if (myht && myht->nApplyCount > 1) {
		smart_str_appendl(buf, "null", 4);
		return;
	}


	i = myht ? zend_hash_num_elements(myht) : 0;

	if (i > 0)
	{
		char *key;
		zval **data;
		ulong index;
		uint key_len;
		HashPosition pos;
		HashTable *tmp_ht;
		int need_comma = 0;

		zend_hash_internal_pointer_reset_ex(myht, &pos);
		for (;; zend_hash_move_forward_ex(myht, &pos))
		{
			i = zend_hash_get_current_key_ex(myht, &key, &key_len, &index, 0, &pos);
			if (i == HASH_KEY_NON_EXISTANT)
				break;

			if (zend_hash_get_current_data_ex(myht, (void **) &data, &pos) == SUCCESS)
			{
				tmp_ht = HASH_OF(*data);
				if (tmp_ht)
				{
					tmp_ht->nApplyCount++;
				}
				hot_encode_ex(buf, *data TSRMLS_CC);
				

				if (tmp_ht)
				{
					tmp_ht->nApplyCount--;
				}
			}
		}
	}
}

static void hot_escape_string(smart_str *buf, char *s, int len TSRMLS_DC) /* {{{ */
{
	int pos = 0, ulen = 0;
	unsigned short us;
	unsigned short *utf16;
	size_t newlen;

	while (pos < len)
	{
		us = s[pos++];

		smart_str_appendc(buf, (unsigned char) us);
	}
}
/* }}} */

void hot_encode_ex(smart_str *buf, zval *val TSRMLS_DC)
{
	switch (Z_TYPE_P(val))
	{
	case IS_NULL:
		smart_str_appendl(buf, "null", 4);
		break;

	case IS_BOOL:
		if (Z_BVAL_P(val)) {
			smart_str_appendl(buf, "true", 4);
		} else {
			smart_str_appendl(buf, "false", 5);
		}
		break;

	case IS_LONG:
		smart_str_append_long(buf, Z_LVAL_P(val));
		break;

	case IS_DOUBLE:
		{
			char *d = NULL;
			int len;
			double dbl = Z_DVAL_P(val);

			if (!zend_isinf(dbl) && !zend_isnan(dbl))
			{
				len = spprintf(&d, 0, "%.*k", (int) EG(precision), dbl);
				smart_str_appendl(buf, d, len);
				efree(d);
			}
			else 
			{
				smart_str_appendc(buf, '0');
			}
		}
		break;

	case IS_STRING:
		{
			hot_escape_string(buf, Z_STRVAL_P(val), Z_STRLEN_P(val) TSRMLS_CC);
		
			break;
		}

	case IS_ARRAY:
		hot_encode_array(buf, &val TSRMLS_CC);
		break;

	default:
		smart_str_appendl(buf, "null", 4);
		break;
	}

	return;
}

#include "hotscript/script_parser.h"

PHP_FUNCTION(hot_encode)
{
	zval *parameter;
	zval **data;
	smart_str buf = {0};
	ZVALReader reader;
	SCRIPT_PARSER sp;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|ll", &parameter) == FAILURE)
	{
		return;
	}
	reader.super.begin = zval_reader_begin;
	reader.super.read = zval_reader_read;
	reader.super.end = zval_reader_end;
	reader.stack[0].val = parameter;
	reader.stack_num = 1;
	

	//hot_encode_ex(&buf, parameter TSRMLS_CC);
	/*
	if(zend_hash_find(parameter->value.ht, "a", 2, (void **)&data) != SUCCESS)
	{
		printf("error\n");
	}
	*/
	if(script_parser(&sp, "d:/2.xml", &reader.super) == 0)
	{
		printf("output succeed\n");
	}
	else
	{
		printf("output failed\n");
	}
	ZVAL_STRINGL(return_value, buf.c, buf.len, 1);

	smart_str_free(&buf);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
