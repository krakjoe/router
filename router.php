<?php
if (extension_loaded("router")) {

	class Console {
		public static function main($argv = array()) {
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
			->addRoute("get", "~/blog/.*~", function() use (&$router) { 
				printf("Welcome to the bloggingz :)\n");
				return true;
			})
			->addRoute("get", "~.*~", function(){
				printf("I am the default route ...\n");
				return true;
			})
			->route();
	} catch (RoutingException $re) {
		printf(
			"Something went wong routing request :(\n");
	}
}
?>
