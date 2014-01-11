Router
======

```php
<?php
class Router {
	/**
	* The console handler is invoked whenever appropriate
	* @param handler - the console handler
	* Throws RoutingException if the handler is not callable
	*/
	public function setConsole(Callable handler);
	
	/**
	* The default handler is invoked when a web request matches no other handler
	* @param handler - the default web request handler
	* Throws RoutingException if the handler is not callable
	*/
	public function setDefault(Callable handler);
	
	/**
	* @param method   - case insensitive request method
	* @param uri	  - regexp for request uri
	* @param handler  - route handler
	* Throws RoutingException if the handler is not callable
	*/
	public function addRoute(string method, string uri, Callable handler);
	
	/**
	* @param method   - route
	* Throws RoutingException upon failure to manipulate the route appropriately
	*/
	public function addRoute(Route route);
	
	/**
	* Sets the request, overriding detection on ::route
	* @param method - method for the request
	* @param uri    - uri for the request
	* Throws RoutingException upon failure to find suitable handler
	*/
	public function setRequest(string method, string uri);
	
	/**
	* Sends a Location: header and finishes the request
	* @param location - the location uri
	* @param code     - ROUTER_DIRECT_TEMP or ROUTER_REDIRECT_PERM
	*/
	public function redirect(string location [, integer code = ROUTER_REDIRECT_TEMP]);
	
	/**
	* Reroutes the current request without redirecting the browser
	* @param method - target method
	* @param uri    - target uri
	* Note superglobals are not touched, only the correct handler is invoked
	* Throws RoutingException upon failure to find suitable handler
	*/
	public function reroute(string method, string uri);
	
	/**
	* Invoke the most suitable handler for the current request
	* Throws RoutingException upon failure to find suitable handler
	*/
	public function route(void);
}
?>
```

Route
=====

```php
<?php
interface Route {
	/**
	* Should return get/put/patch/post etc
	* @return string
	*/
	public function getMethod();
	
	/**
	* Should return a valid regex pattern describing request
	* @return string
	*/
	public function getURI();
	
	/**
	* Will be invoked for appropriate requests
	* @param patterns - groups array
	* @return boolean
	*/
	public function handle($patterns);
}
?>
```

Constants
=========

For redirection:

  - ROUTER_REDIRECT_TEMP
  - ROUTER_REDIRECT_PERM
  
Hello World
===========

```php
<?php
try {
	$router = new Router();
	$router->setDefault(function(){
		printf("Hello World\n");
	})
	->route();
} catch(RoutingException $ex) {
	printf(
		"<pre>Exception: %s\n</pre>", (string)$ex);
}
?>
```

Hello Console & Web
===================

```php
<?php
try {
	$router = new Router();
	$router
		->setDefault(function(){
			printf("Hello Web User\n");
		})
		->setConsole(function(){
			printf("Hello Console User\n");
		})
		->route();
} catch(RoutingException $ex) {
	printf(
		"<pre>Exception: %s\n</pre>", (string)$ex);
}
?>
```

Hello Objects
=============

```php
<?php
class BlogRoute implements Route {
	/* always return a valid method string */
	public function getMethod() 		{ return "get"; }
	/* always return a valid regex pattern string */
	public function getURI()    		{ return "~/blog/?(.*)?~"; }
	/* always return a boolean value */
	public function handle($patterns)	{ return true; }
}

class DefaultRoute implements Route {
	/* always return a valid method string */
	public function getMethod() 		{ return "get"; }
	/* always return a valid regex pattern string */
	public function getURI()    		{ return "~/(.*)~"; }
	/* always return a boolean value */
	public function handle($patterns)	{ return true; }
}

try {
	$router = new Router();
	$router
		->addRoute(new BlogRoute())
		->addRoute(new DefaultRoute())
		->setRequest("get", "/whatever")
		->route();
} catch(RoutingException $ex) {
	printf(
		"<pre>Exception: %s\n</pre>", (string)$ex);
}
?>
```

