Router
======

This extension serves to provide a sane, easy router for modern PHP applications:

```php
<?php
abstract Router {
	/*
	* The console program is executed whenever appropriate
	* @param callback - the console program
	*/
	public function setConsole(Callable callback);
	
	/*
	* The default route is invoked when a web request matches no other route
	* @param callback - the console program
	*/
	public function setDefault(Callable callback);
	
	/*
	* @param method   - get/post/put/patch etc
	* @param uri	  - regexp to match REQUEST_URI
	* @param callback - the routes callable
	*/
	public function addRoute(string method, string uri, Callable callback);
	
	/*
	* Sends a Location: header and finishes the request
	* @param location - the location to redirect to
	* @param code     - the response code for the redirect
	*/
	public function redirect(string location [, integer code = ROUTER_REDIRECT_TEMP]);
	
	/*
	* Invoke the most suitable route for the current request
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
