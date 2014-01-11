<?php
if (extension_loaded("router")) {

	class Console {
		/*
		* @param config - parsed arguments from command line
		* 
		* exec (this script)
		* flags (-flag)
		* options (--option=value or --option value)
		* args (anything else)
		*/
		public static function main($config = array()) {
			var_dump($config);
			
			printf(
				"I am the console program :)\n");
			return true;
		}
	}
	
	try {
		$router = new Router();
		$router
			->setConsole(array("Console", "main"))
			->addRoute("get", "~^/redirect-me~", function() use (&$router) {
				$router->redirect(
					"/redirect-to", ROUTER_REDIRECT_PERM);
				return true;
			})
			->addRoute("get", "~^/redirect-to~", function(){
				printf("You were redirected here ...\n");
				return true;
			})
			->addRoute("get", "~^/reroute-me/?([^/]+)?/?~", function($patterns) use (&$router) {
				@list($uri, $reroute) = $patterns;
				
				if ($reroute) {
					$router->reroute("get", "/{$reroute}");
				} else $router->reroute("get", "/blog");
				return true;
			})
			->addRoute("get", "~^/reroute-to~", function(){
				printf("rerouted here ...\n");
				return true;
			})
			->addRoute("get", "~^/blog/?([a-z]+)?/?([a-z\-]+)?~", function($patterns) use (&$router) { 
				@list($uri, $action, $article) = $patterns;
				
				if ($action) {
					if ($article) {
						printf("Welcome to the bloggingz: %s %s :)\n", $action, $article);
					} else printf("Welcome to the bloggingz: %s :)\n", $action);
				} else printf("Welcome to the bloggingz :)\n");
				
				return true;
			})
			->setDefault(function(){
				printf("I am the default route ...\n");
				return true;
			})
			->route();
	} catch (RoutingException $re) {
		printf(
			"Fatal Error:\n".
			"<pre>%s\n</pre>\n", 
			(string) $re);
	}
}
?>
