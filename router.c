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
  | Author: Joe Watkins <krakjoe@php.net>                                |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "SAPI.h"
#include "ext/pcre/php_pcre.h"
#include "ext/standard/head.h"
#include "ext/standard/info.h"
#include "php_router.h"
#include "zend_exceptions.h"

#define ROUTER_REDIRECT_PERM 301
#define ROUTER_REDIRECT_TEMP 302

zend_class_entry *Router;
zend_class_entry *RoutingException;

typedef struct _router_t {
	zend_object std;
	HashTable   routes;
	zend_bool   console;
} router_t;

typedef struct _route_t {
	zend_bool	       console;
	zval               method;
	zval			   uri;
	zval		      *callable;
} route_t;

const char *Console_Route = "Console";
static size_t Console_Route_Size = sizeof(Console_Route)-1;

/* {{{ */
ZEND_BEGIN_ARG_INFO_EX(Router_no_args, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Router_addRoute, 0, 0, 3)
	ZEND_ARG_INFO(0, method)
	ZEND_ARG_INFO(0, uri)
	ZEND_ARG_INFO(0, callable)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Router_setConsole, 0, 0, 1)
	ZEND_ARG_INFO(0, callable)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Router_redirect, 0, 0, 1)
	ZEND_ARG_INFO(0, location)
ZEND_END_ARG_INFO() /* }}} */

#define RETURN_CHAIN() do {\
	Z_TYPE_P(return_value) = IS_OBJECT; \
	Z_OBJVAL_P(return_value) = Z_OBJVAL_P(getThis()); \
	zval_copy_ctor(return_value);\
} while(0)

/* {{{ proto Router Router::__construct(void) 
	Will return a new Router instance */
static PHP_METHOD(Router, __construct) { 
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
} /* }}} */

/* {{{ proto Router Router::addRoute(string method, string uri, Callable handler) */
static PHP_METHOD(Router, addRoute) {
	zval *method,
		 *uri,
		 *callable;
	 
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzz", &method, &uri, &callable) != SUCCESS) {
		return;
	}

	if (Z_TYPE_P(method) == IS_STRING) {
		if (Z_TYPE_P(uri) == IS_STRING) {
			char *callable_name = NULL;
			
			if (zend_is_callable(callable, 0, &callable_name TSRMLS_CC)) {
				route_t route = {0};
				
				ZVAL_STRINGL(&route.method, Z_STRVAL_P(method), Z_STRLEN_P(method), 1);
				ZVAL_STRINGL(&route.uri, Z_STRVAL_P(uri), Z_STRLEN_P(uri), 1);
				
				route.callable = callable;
				
				Z_ADDREF_P(route.callable);
				
				{
					router_t *router = zend_object_store_get_object(getThis() TSRMLS_CC);
					
					zend_hash_next_index_insert(
						&router->routes, (void**) &route, sizeof(route_t), NULL);
				}
			}
			
			if (callable_name) {
				efree(callable_name);
			}
		}
	}
	
	RETURN_CHAIN();
} /* }}} */

/* {{{ proto Router Router::setConsole(Callable handler) */
static PHP_METHOD(Router, setConsole) {
	zval *callable;
	 
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &callable) != SUCCESS) {
		return;
	}

	{
		char *callable_name = NULL;
			
		if (zend_is_callable(callable, 0, &callable_name TSRMLS_CC)) {
			route_t route = {1};
			
			route.callable = callable;
		
			Z_ADDREF_P(route.callable);
		
			{
				router_t *router = zend_object_store_get_object(getThis() TSRMLS_CC);
			
				zend_hash_update(
					&router->routes, Console_Route, Console_Route_Size, (void**)&route, sizeof(route_t), NULL);

				router->console = 1;
			}
		}
	
		if (callable_name) {
			efree(callable_name);
		}
	}
	
	RETURN_CHAIN();
} /* }}} */

static inline void Router_do_route(route_t *route, zval *return_value_ptr TSRMLS_DC) { /* {{{ */
	if (route) {
		zval *callback_retval_ptr = NULL;
		zend_fcall_info callback_info;
		zend_fcall_info_cache callback_cache;
		char *callback_name, *callback_error;

		if (zend_fcall_info_init(route->callable, 0, &callback_info, &callback_cache, &callback_name, &callback_error TSRMLS_CC) == SUCCESS) {
			callback_info.retval_ptr_ptr = &callback_retval_ptr;
		
			if (zend_call_function(&callback_info, &callback_cache TSRMLS_CC) != SUCCESS) {
				zend_throw_exception(
					RoutingException, "failed to call selected route", 0 TSRMLS_CC);
			}
		
			if (callback_retval_ptr) {
				ZVAL_ZVAL(return_value_ptr, callback_retval_ptr, 0, 1);
			}
		}
	
		if (callback_name)
			efree(callback_name);
		if (callback_error)
			efree(callback_error);
	}
} /* }}} */

/* {{{ proto mixed Router::route(void) 
	returns the result of invoking the selected routes callable 
	Throws RoutingException on failure */
static PHP_METHOD(Router, route) {
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	router_t *router = zend_object_store_get_object(getThis() TSRMLS_CC);
	
	if (zend_hash_num_elements(&router->routes)) {
		HashPosition position;
		route_t *route = NULL;
		zend_bool routed = 0;
		
		if (SG(request_info).request_method) {
			size_t method_length = strlen(SG(request_info).request_method);
			char *method = zend_str_tolower_dup(SG(request_info).request_method, method_length);
			
			char *request_uri = SG(request_info).request_uri;
			size_t request_uri_length = strlen(request_uri);
			
			for (zend_hash_internal_pointer_reset_ex(&router->routes, &position);
				zend_hash_get_current_data_ex(&router->routes, (void**) &route, &position) == SUCCESS;
				zend_hash_move_forward_ex(&router->routes, &position)) {
				if (method_length == Z_STRLEN(route->method)) {
					if (strncasecmp(method, Z_STRVAL(route->method), method_length) == SUCCESS) {
						zval zmatch = {0},
						     zsubs = {0};
						pcre_cache_entry *pcre = pcre_get_compiled_regex_cache(Z_STRVAL(route->uri), Z_STRLEN(route->uri) TSRMLS_CC);
						
						if (pcre != NULL) {
							php_pcre_match_impl(
								pcre, 
								request_uri, request_uri_length, 
								&zmatch, &zsubs, 0, 0, 0, 0 TSRMLS_CC);
							
							zval_dtor(&zsubs);
							
							if (zend_is_true(&zmatch)) {
								Router_do_route(route, return_value TSRMLS_CC);
								efree(method);
								return;
							}
						} else {
							zend_throw_exception(
								RoutingException, "invalid route found", 0 TSRMLS_CC);
						}
					}
				}
			}
			
			efree(method);
		} else {
			if (router->console) {
				if (zend_hash_find(&router->routes, Console_Route, Console_Route_Size, (void**)&route) == SUCCESS) {
					Router_do_route(route, return_value TSRMLS_CC);
					return;
				}
			}
		}
	}
	
	zend_throw_exception(RoutingException, "no route found", 0 TSRMLS_CC);
} /* }}} */

/* {{{ proto void Router::redirect(string location [, integer code = ROUTER_REDIRECT_TEMP])
	Redirect the browser to the specified location */
static PHP_METHOD(Router, redirect) {
	char *location = NULL;
	zend_uint location_length = 0;
	sapi_header_line redirect = {0};
	
	redirect.response_code = ROUTER_REDIRECT_TEMP;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &location, &location_length, &redirect.response_code) == FAILURE) {
		return;			
	}
	
	asprintf(
		&redirect.line, "Location: %s", location);
	redirect.line_len = strlen(redirect.line);
	
	sapi_header_op(
		SAPI_HEADER_REPLACE, &redirect TSRMLS_CC);
	php_header(TSRMLS_C);
	
	free(redirect.line);
}

static zend_function_entry router_class_methods[] = { /* {{{ */
	PHP_ME(Router, __construct, Router_no_args, ZEND_ACC_PUBLIC)
	PHP_ME(Router, addRoute,	Router_addRoute, ZEND_ACC_PUBLIC)
	PHP_ME(Router, setConsole,	Router_setConsole, ZEND_ACC_PUBLIC)
	PHP_ME(Router, route,		Router_no_args, ZEND_ACC_PUBLIC)
	PHP_ME(Router, redirect,	Router_redirect, ZEND_ACC_PUBLIC)
	PHP_FE_END
}; /* }}} */

static void Router_destroy_route(route_t *route) {
	zval_dtor(&route->method);
	zval_dtor(&route->uri);
	zval_ptr_dtor(&route->callable);
}

static void Router_destroy_object(router_t *router, zend_object_handle handle TSRMLS_DC) { /* {{{ */

#if PHP_VERSION_ID > 50399
	zend_object *object = &router->std;

	if (object->properties_table) {
		int i;
		for (i = 0; i < object->ce->default_properties_count; i++) {
			if (object->properties_table[i]) {
				zval_ptr_dtor(&object->properties_table[i]);
			}
		}
		efree(object->properties_table);
	}

	if (object->properties) {
		zend_hash_destroy(object->properties);
		FREE_HASHTABLE(object->properties);
	}
#else
	zend_object_std_dtor(&(router->std) TSRMLS_CC);
#endif

	zend_hash_destroy(&router->routes);

	efree(router);
} /* }}} */

static zend_object_value Router_create_object(zend_class_entry *zce TSRMLS_DC) { /* {{{ */
	zend_object_value attach;
	router_t *router = (router_t*) ecalloc(1, sizeof(router_t));
	
	zend_object_std_init(&router->std, zce TSRMLS_CC);
#if PHP_VERSION_ID < 50400
	{
		zval *temp;

		zend_hash_copy(														
			router->std.properties,
			&zce->default_properties,
			(copy_ctor_func_t) zval_add_ref,
			&temp, sizeof(zval*)
		);
	}
#else
	object_properties_init(&(router->std), zce);
#endif	

	zend_hash_init(&router->routes, 8, NULL, (dtor_func_t) Router_destroy_route, 0);

	attach.handle = zend_objects_store_put(
		router,
		(zend_objects_store_dtor_t) Router_destroy_object,
		NULL, NULL TSRMLS_CC
	);
	attach.handlers = zend_get_std_object_handlers();
	
	return attach;
} /* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(router)
{
	zend_class_entry Router_entry,
					 RoutingException_entry;
	
	INIT_CLASS_ENTRY(Router_entry, "Router", router_class_methods);
	Router_entry.create_object = Router_create_object;
	Router = zend_register_internal_class(	
		&Router_entry TSRMLS_CC);
	
	INIT_CLASS_ENTRY(RoutingException_entry, "RoutingException", NULL);
	RoutingException = zend_register_internal_class_ex(
		&RoutingException_entry, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);

	REGISTER_LONG_CONSTANT("ROUTER_REDIRECT_PERM", ROUTER_REDIRECT_PERM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ROUTER_REDIRECT_TEMP", ROUTER_REDIRECT_TEMP, CONST_CS | CONST_PERSISTENT);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */	
PHP_MINFO_FUNCTION(router)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "Router support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ router_module_entry
 */
zend_module_entry router_module_entry = {
	STANDARD_MODULE_HEADER,
	PHP_ROUTER_EXTNAME,
	NULL,
	PHP_MINIT(router),
	NULL,
	NULL,
	NULL,
	PHP_MINFO(router),
	PHP_ROUTER_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_ROUTER
ZEND_GET_MODULE(router)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
