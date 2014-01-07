Router
======

This extension serves to provide a sane, easy router for modern PHP applications:

```php
<?php
abstract Router {
	/*
	* The console program is executed whenever appropriate
	* @param handler - the console program
	* Throws RoutingException if the handler is not callable
	*/
	public function setConsole(Callable handler);
	
	/*
	* The default route is invoked when a web request matches no other route
	* @param handler - the console program
	* Throws RoutingException if the handler is not callable
	*/
	public function setDefault(Callable handler);
	
	/*
	* @param method   - get/post/put/patch etc
	* @param uri	  - regexp to match REQUEST_URI
	* @param handler  - this routes handler
	* Throws RoutingException if the handler is not callable
	*/
	public function addRoute(string method, string uri, Callable handler);
	
	/*
	* Sends a Location: header and finishes the request
	* @param location - the location to redirect to
	* @param code     - the response code for the redirect
	*/
	public function redirect(string location [, integer code = ROUTER_REDIRECT_TEMP]);
	
	/*
	* Reroutes the current request without redirecting the browser
	* @param method - method for target route
	* @param uri    - uri for target route
	* Note superglobals are not touched, only the correct handler is invoked
	* Throws RoutingException upon failure to find suitable route
	*/
	public function reroute(string method, string uri);
	
	/*
	* Invoke the most suitable route for the current request
	* Throws RoutingException upon failure to find suitable route
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
