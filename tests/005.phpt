--TEST--
Check rerouting
--SKIPIF--
<?php include_once(sprintf("%s/skip-if.inc", __DIR__)) ?>
--FILE--
<?php 
$router = new Router();
$router
	->addRoute("get", "~/reroute\-me~", function() use (&$router) {
		$router->reroute("get", "/reroute-to");
		return true;
	})
	->addRoute("get", "~/reroute\-to~", function() {
		printf("rerouted");
		return true;
	})
	->setRequest("get", "/reroute-me")
	->route();
?>
--EXPECT--
rerouted


