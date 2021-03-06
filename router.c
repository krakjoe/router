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
#include "zend_interfaces.h"

#define ROUTER_REDIRECT_PERM 301
#define ROUTER_REDIRECT_TEMP 302

#define ROUTER_ROUTE_NORMAL 0
#define ROUTER_ROUTE_CONSOLE 1
#define ROUTER_ROUTE_DEFAULT 2

#define getRouter() zend_object_store_get_object(getThis() TSRMLS_CC)

zend_class_entry *Router;
zend_class_entry *Route;
zend_class_entry *RoutingException;

typedef struct _route_t {
	zend_uchar	       type;
	zval               method;
	zval			   uri;
	zval		      *callable;
} route_t;

typedef struct _router_t {
	zend_object std;
	HashTable   routes;
	route_t     *console;
	route_t     *fallback;
	route_t		*set;
} router_t;

const char *Console_Route = "\0\0$$Console$$\0\0";
static size_t Console_Route_Size = sizeof(Console_Route)-1;

const char *Default_Route = "\0\0$$Default$$\0\0";
static size_t Default_Route_Size = sizeof(Default_Route)-1;

/* {{{ */
ZEND_BEGIN_ARG_INFO_EX(Router_no_args, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Router_two_args, 0, 0, 2)
	ZEND_ARG_INFO(0, method)
	ZEND_ARG_INFO(0, uri)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Router_array_arg, 0, 0, 1)
	ZEND_ARG_INFO(0, array)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Router_addRoute, 0, 0, 3)
	ZEND_ARG_INFO(0, method)
	ZEND_ARG_INFO(0, uri)
	ZEND_ARG_INFO(0, callable)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Router_setCallable, 0, 0, 1)
	ZEND_ARG_INFO(0, callable)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Router_redirect, 0, 0, 1)
	ZEND_ARG_INFO(0, location)
ZEND_END_ARG_INFO() /* }}} */

#define RETURN_CHAIN() do {\
	Z_SET_ISREF_P(return_value); \
	Z_TYPE_P(return_value) = IS_OBJECT; \
	ZVAL_ZVAL(return_value, getThis(), 1, 0);\
} while(0)

/* {{{ proto Router Router::__construct(void) 
	Will return a new Router instance */
static PHP_METHOD(Router, __construct) { 
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
} /* }}} */

/* {{{ proto Router Router::setRequest(string method, string uri)
	Set the request, overriding detection on ::route() */
static PHP_METHOD(Router, setRequest) {
	const char *request_method = NULL;
	size_t request_method_length = 0L;
	const char *request_uri = NULL;
	size_t request_uri_length = 0L;
	router_t *router = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &request_method, &request_method_length, &request_uri, &request_uri_length) != SUCCESS) {
		return;
	}
	
	router = getRouter();

	{
		if (!router->set) {
			router->set = (route_t*) ecalloc(1, sizeof(route_t));
		} else {
			zval_dtor(&router->set->method);
			zval_dtor(&router->set->uri);
		}
		
		ZVAL_STRINGL(&router->set->method,
			request_method, request_method_length, 1);
		ZVAL_STRINGL(&router->set->uri, 
			request_uri, request_uri_length, 1);
	}

	RETURN_CHAIN();
}

/* {{{ proto Router Router::addRoute(string method, string uri, Callable handler)
	   proto Router Router::addRoute(Route route)
	Throws RoutingException on failure */
static PHP_METHOD(Router, addRoute) {
	const char *request_method = NULL;
	size_t request_method_length = 0L;
	const char *request_uri = NULL;
	size_t request_uri_length = 0L;
	zval *callable = NULL;
	zend_class_entry *ce = NULL;
	router_t *router = getRouter();
	
	switch (ZEND_NUM_ARGS()) {
		case 3: if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "ssz", 
				&request_method, &request_method_length, 
				&request_uri, &request_uri_length, &callable) != SUCCESS) {
			return;
		} break;
		
		case 1: if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "O", 
				&callable, ce) != SUCCESS) {
			return;
		} break;
		
		default:
			zend_throw_exception(
				RoutingException, "Router::addRoute expects one or three arguments, please see documentation", 0 TSRMLS_CC);
			return;
	}

	{
		route_t route = {ROUTER_ROUTE_NORMAL};
		
		route.callable = callable;
		
		if (ZEND_NUM_ARGS() == 1) {
			zval *pmethod = NULL, 
				 *puri = NULL;
			
			zend_call_method_with_0_params(
				&callable, ce, NULL, "getMethod", &pmethod);
			
			if (Z_TYPE_P(pmethod) != IS_STRING) {
				zend_throw_exception(
					RoutingException, "getMethod returned a non-string value", 0 TSRMLS_CC);
				zval_ptr_dtor(&pmethod);
				return;
			}
			
			zend_call_method_with_0_params(
				&callable, ce, NULL, "getURI", &puri);
				
			if (Z_TYPE_P(puri) != IS_STRING) {
				zend_throw_exception(
					RoutingException, "getURI returned a non-string value", 0 TSRMLS_CC);
				zval_ptr_dtor(&pmethod);
				zval_ptr_dtor(&puri);
				return;
			}
			
			ZVAL_STRINGL(&route.method, Z_STRVAL_P(pmethod), Z_STRLEN_P(pmethod), 1);
			ZVAL_STRINGL(&route.uri, Z_STRVAL_P(puri), Z_STRLEN_P(puri), 1);
			Z_ADDREF_P(route.callable);
			zend_hash_next_index_insert(
				&router->routes, 
				(void**) &route, sizeof(route_t), NULL);
			zval_ptr_dtor(&pmethod);
			zval_ptr_dtor(&puri);
		} else {
			char *callable_name = NULL;
			
			if (!zend_is_callable(callable, 0, &callable_name TSRMLS_CC)) {
				zend_throw_exception(
					RoutingException, "handler is not callable", 0 TSRMLS_CC);
				if (callable_name)
					efree(callable_name);
				return;
			}
			
			ZVAL_STRINGL(&route.method, request_method, request_method_length, 1);
			ZVAL_STRINGL(&route.uri, request_uri, request_uri_length, 1);
			Z_ADDREF_P(route.callable);
			zend_hash_next_index_insert(
				&router->routes, 
				(void**) &route, sizeof(route_t), NULL);
			if (callable_name)
				efree(callable_name);
		}
	}

	RETURN_CHAIN();
} /* }}} */

/* {{{ proto Router Router::setConsole(Callable handler) 
	Throws RoutingException on failure 
	The console route is invoked whenever executed from the command line */
static PHP_METHOD(Router, setConsole) {
	zval *callable;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &callable) != SUCCESS) {
		return;
	}

	{
		char *callable_name = NULL;

		if (zend_is_callable(callable, 0, &callable_name TSRMLS_CC)) {
			route_t route = {ROUTER_ROUTE_CONSOLE};

			route.callable = callable;
			
			Z_ADDREF_P(route.callable);

			{
				router_t *router = getRouter();

				zend_hash_update(
					&router->routes,
					Console_Route, Console_Route_Size, 
					(void**)&route, sizeof(route_t), (void**)&router->console);
			}
		} else {
			zend_throw_exception(
				RoutingException, "handler is not callable", 0 TSRMLS_CC);
		}

		if (callable_name) {
			efree(callable_name);
		}
	}

	RETURN_CHAIN();
} /* }}} */

/* {{{ proto Router Router::setDefault(Callable handler) 
	Throws RoutingException on failure 
	The default route is invoked for web requests that match no other route */
static PHP_METHOD(Router, setDefault) {
	zval *callable;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &callable) != SUCCESS) {
		return;
	}

	{
		char *callable_name = NULL;

		if (zend_is_callable(callable, 0, &callable_name TSRMLS_CC)) {
			route_t route = {ROUTER_ROUTE_DEFAULT};

			route.callable = callable;
			
			Z_ADDREF_P(route.callable);

			{
				router_t *router = getRouter();
				
				zend_hash_update(
					&router->routes, 
					Default_Route, Default_Route_Size,
					(void**)&route, sizeof(route_t), (void**)&router->fallback);
			}
		} else {
			zend_throw_exception(
				RoutingException, "handler is not callable", 0 TSRMLS_CC);
		}

		if (callable_name) {
			efree(callable_name);
		}
	}

	RETURN_CHAIN();
} /* }}} */

static inline void Router_do_route(route_t *route, zval *groups, zval *return_value_ptr TSRMLS_DC) { /* {{{ */
	zval *local_retval_ptr = NULL;
	zend_class_entry *ce = NULL;
	
	if (!route)
		return;
	
	if (Z_TYPE_P(route->callable) == IS_OBJECT)
		ce = Z_OBJCE_P(route->callable);
	
	if (ce && instanceof_function(ce, Route TSRMLS_CC)) {
		if (zend_call_method_with_1_params(&route->callable, ce, NULL, "handle", &local_retval_ptr, groups) == NULL || 
			Z_TYPE_P(local_retval_ptr) == IS_NULL) {
			zend_throw_exception(
				RoutingException, "route did not return an appropriate value", 0 TSRMLS_CC);
			return;
		}
		
		ZVAL_ZVAL(return_value_ptr, local_retval_ptr, 0, 1);
	} else {
		zval *callback_args = NULL;
		zend_fcall_info callback_info;
		zend_fcall_info_cache callback_cache;
		char *callback_name, *callback_error;

		if (zend_fcall_info_init(route->callable, 0, &callback_info, &callback_cache, &callback_name, &callback_error TSRMLS_CC) == SUCCESS) {
			callback_info.retval_ptr_ptr = &local_retval_ptr;
			if (groups) {
				callback_info.params = (zval***) ecalloc(1, sizeof(zval**));
				callback_info.params[0] = &groups;
				callback_info.param_count = 1;
			} else {
				callback_info.params = 0;
				callback_info.param_count = 0;
			}

			if ((zend_call_function(&callback_info, &callback_cache TSRMLS_CC) != SUCCESS) || 
				(local_retval_ptr == NULL || Z_TYPE_P(local_retval_ptr) == IS_NULL)) {
				zend_throw_exception(
					RoutingException, "route did not return an appropriate value", 0 TSRMLS_CC);
				goto cleanup;
			}
		
			ZVAL_ZVAL(return_value_ptr, local_retval_ptr, 0, 1);

cleanup:
			if (callback_info.params)
				efree(callback_info.params);
			if (callback_name)
				efree(callback_name);
			if (callback_error)
				efree(callback_error);
		}

		
	}
} /* }}} */

static inline zend_bool Router_call_route(router_t *router, zval *method, zval *uri, zval *return_value TSRMLS_DC) { /* {{{ */
	zval *zsubs;
	HashPosition position;
	route_t *route = NULL;
	
	ALLOC_INIT_ZVAL(zsubs);
	array_init(zsubs);

	for (zend_hash_internal_pointer_reset_ex(&router->routes, &position);
		zend_hash_get_current_data_ex(&router->routes, (void**) &route, &position) == SUCCESS;
		zend_hash_move_forward_ex(&router->routes, &position)) {
		
		if (route->type)
			continue;
		
		if ((Z_STRLEN_P(method) == Z_STRLEN(route->method)) && 
			(strncasecmp(Z_STRVAL_P(method), Z_STRVAL(route->method), Z_STRLEN_P(method)) == SUCCESS)) {
			zval zmatch = {0};

			pcre_cache_entry *pcre = pcre_get_compiled_regex_cache(Z_STRVAL(route->uri), Z_STRLEN(route->uri) TSRMLS_CC);

			if (pcre != NULL) {
				php_pcre_match_impl(
					pcre,
					Z_STRVAL_P(uri), Z_STRLEN_P(uri), 
					&zmatch, zsubs, 0, 0, 0, 0 TSRMLS_CC);

#if PHP_VERSION_ID >= 50700
				if (zend_is_true(&zmatch TSRMLS_CC)) {
#else
				if (zend_is_true(&zmatch)) {
#endif
					Router_do_route(route, zsubs, return_value TSRMLS_CC);
					zval_ptr_dtor(&zsubs);
					return 1;
				}

				zend_hash_clean(Z_ARRVAL_P(zsubs));
			} else {
				zend_throw_exception(
					RoutingException, "invalid route found", 0 TSRMLS_CC);
			}
		}
	}

	zval_ptr_dtor(&zsubs);
	
	return 0;
} /* }}} */

static inline void Router_call_console(router_t *router, zval *return_value TSRMLS_DC) { /* {{{ */
	zval *zargv, *zflags, *zoptions, *zargs;

	MAKE_STD_ZVAL(zargv);
	MAKE_STD_ZVAL(zflags);
	MAKE_STD_ZVAL(zoptions);
	MAKE_STD_ZVAL(zargs);

	array_init(zargv);
	array_init(zflags);
	array_init(zoptions);
	array_init(zargs);

	add_assoc_zval(zargv, "flags", zflags);
	add_assoc_zval(zargv, "options", zoptions);
	add_assoc_zval(zargv, "args", zargs);

	if (SG(request_info).argc) {
		char **argv = SG(request_info).argv;
		int    argc = SG(request_info).argc;
		int    arg  = 1;

		add_assoc_string(zargv, "exec", argv[0], strlen(argv[0]));

		while (arg < argc) {
			size_t len = strlen(argv[arg]);

			switch (argv[arg][0]) {
				case '-': switch (argv[arg][1]) {
					case '-': {
						if (len > 2) {
							char *option = strdup(&argv[arg][2]);
							char *value = strstr(option, "=");

							if (value) {
								option[value - option] = 0;
								value++;
							} else {
								if (argc > arg+1) {
									arg++;
									value = argv[arg];
								}
							}

							if (option && value) {
								add_assoc_string(
									zoptions, option, value, strlen(value));
							}

							free(option);
						}
					} break;

					default: {
						if (len > 1) {
							add_next_index_string(
								zflags, &argv[arg][1], len-1);
						}
					}
				} break;

				default: {
					add_next_index_string(zargs, argv[arg], len);
				}
			}
			arg++;
		}
	}

	Router_do_route(router->console, zargv, return_value TSRMLS_CC);

	zval_ptr_dtor(&zargv);
	zval_ptr_dtor(&zflags);
	zval_ptr_dtor(&zoptions);
	zval_ptr_dtor(&zargs);
}

/* {{{ proto mixed Router::route(void)
	returns the result of invoking the first suitable routes handler 
	Throws RoutingException on failure */
static PHP_METHOD(Router, route) {
	router_t *router = NULL;

	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}

	router = getRouter();

	if (zend_hash_num_elements(&router->routes)) {
		const char *request_method = NULL;
		size_t request_method_length = 0L;
		const char *request_uri = NULL;
		size_t request_uri_length = 0L;
		
		if (router->set) {
			request_method = Z_STRVAL(router->set->method);
			request_method_length = Z_STRLEN(router->set->method);
			request_uri = Z_STRVAL(router->set->uri);
			request_uri_length = Z_STRLEN(router->set->uri);
		} else {
			if (SG(request_info).request_method) {
				request_method = SG(request_info).request_method;
				request_method_length = strlen(request_method);
				request_uri = SG(request_info).request_uri;
				request_uri_length = strlen(request_uri);
			}
		}
		
		if (request_method && request_method_length) {
			zval method;
			zval uri;
			
			ZVAL_STRINGL(&method, request_method, request_method_length, 0);
			ZVAL_STRINGL(&uri, request_uri, request_uri_length, 0);
			
			if (Router_call_route(
				router, &method, &uri, return_value TSRMLS_CC)) {
				return;
			}
			
			if (router->fallback) {
				Router_do_route(router->fallback, NULL, return_value TSRMLS_CC);
				return;
			}
		} else {
			if (router->console) {
				Router_call_console(router, return_value TSRMLS_CC);
				return;
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
} /* }}} */

/* {{{ proto mixed Router::reroute(string method, string uri) 
	Reroutes the current request to the specified location without redirecting browser */
static PHP_METHOD(Router, reroute) {
	char *request_method = NULL;
	size_t request_method_length = 0L;
	char *request_uri = NULL;
	size_t request_uri_length = 0L;
	zval method, uri;
	router_t *router = getRouter();
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", 
		&request_method, &request_method_length, &request_uri, &request_uri_length) == FAILURE) {
		return;
	}
	
	ZVAL_STRINGL(&method, request_method, request_method_length, 0);
	ZVAL_STRINGL(&uri, request_uri, request_uri_length, 0);
	
	if (!Router_call_route(router, &method, &uri, return_value TSRMLS_CC)) {
		zend_throw_exception(
			RoutingException, "the request could not be rerouted", 0 TSRMLS_CC);
	}
} /* }}} */

static zend_function_entry router_class_methods[] = { /* {{{ */
	PHP_ME(Router, __construct, Router_no_args, ZEND_ACC_PUBLIC)
	PHP_ME(Router, addRoute,	Router_addRoute, ZEND_ACC_PUBLIC)
	PHP_ME(Router, setRequest,	Router_two_args, ZEND_ACC_PUBLIC)
	PHP_ME(Router, setConsole,	Router_setCallable, ZEND_ACC_PUBLIC)
	PHP_ME(Router, setDefault,	Router_setCallable, ZEND_ACC_PUBLIC)
	PHP_ME(Router, route,		Router_no_args, ZEND_ACC_PUBLIC)
	PHP_ME(Router, redirect,	Router_redirect, ZEND_ACC_PUBLIC)
	PHP_ME(Router, reroute,		Router_two_args, ZEND_ACC_PUBLIC)
	PHP_FE_END
}; /* }}} */

static zend_function_entry route_interface_methods[] = { /* {{{ */
	PHP_ABSTRACT_ME(Route, getMethod, Router_no_args)
	PHP_ABSTRACT_ME(Route, getURI,	  Router_no_args)
	PHP_ABSTRACT_ME(Route, handle,	  Router_array_arg)
	PHP_FE_END
}; /* }}} */

static void Router_destroy_route(route_t *route) { /* {{{ */
	zval_dtor(&route->method);
	zval_dtor(&route->uri);
	zval_ptr_dtor(&route->callable);
} /* }}} */

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

	if (router->set) {
		zval_dtor(&router->set->method);
		zval_dtor(&router->set->uri);
		efree(router->set);
	}

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
					 Route_entry,
					 RoutingException_entry;

	INIT_CLASS_ENTRY(Router_entry, "Router", router_class_methods);
	Router_entry.create_object = Router_create_object;
	Router = zend_register_internal_class(	
		&Router_entry TSRMLS_CC);

	INIT_CLASS_ENTRY(RoutingException_entry, "RoutingException", NULL);
	RoutingException = zend_register_internal_class_ex(
		&RoutingException_entry, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
		
	INIT_CLASS_ENTRY(Route_entry, "Route", route_interface_methods);
	Route = zend_register_internal_class(
		&Route_entry TSRMLS_CC);
	Route->ce_flags |= ZEND_ACC_INTERFACE;

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
