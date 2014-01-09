Router
======

This extension serves to provide a sane, easy router for modern PHP applications:

```php
<?php
abstract Router {
	/*
	* The console handler is invoked whenever appropriate
	* @param handler - the console handler
	* Throws RoutingException if the handler is not callable
	*/
	public function setConsole(Callable handler);
	
	/*
	* The default handler is invoked when a web request matches no other handler
	* @param handler - the default web request handler
	* Throws RoutingException if the handler is not callable
	*/
	public function setDefault(Callable handler);
	
	/*
	* @param method   - case insensitive request method
	* @param uri	  - regexp for request uri
	* @param handler  - request handler
	* Throws RoutingException if the handler is not callable
	*/
	public function addHandler(string method, string uri, Callable handler);
	
	/*
	* Sets the request, overriding detection on ::route
	* @param method - method for the request
	* @param uri    - uri for the request
	* Throws RoutingException upon failure to find suitable handler
	*/
	public function setRequest(string method, string uri);
	
	/*
	* Sends a Location: header and finishes the request
	* @param location - the location uri
	* @param code     - ROUTER_DIRECT_TEMP or ROUTER_REDIRECT_PERM
	*/
	public function redirect(string location [, integer code = ROUTER_REDIRECT_TEMP]);
	
	/*
	* Reroutes the current request without redirecting the browser
	* @param method - target method
	* @param uri    - target uri
	* Note superglobals are not touched, only the correct handler is invoked
	* Throws RoutingException upon failure to find suitable handler
	*/
	public function reroute(string method, string uri);
	
	/*
	* Invoke the most suitable handler for the current request
	* Throws RoutingException upon failure to find suitable handler
	*/
	public function route(void);
}
?>
```

*Note: above is documentation to display API; Router is not an abstract.*

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

