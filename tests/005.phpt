--TEST--
Check rerouting
--SKIPIF--
<?php include_once(sprintf("%s/skip-if.inc", __DIR__)) ?>
--FILE--
<?php 
$router = new Router();
$router
	->addHandler("get", "~/reroute\-me~", function() use (&$router) {
		$router->reroute("get", "/reroute-to");
		return true;
	})
	->addHandler("get", "~/reroute\-to~", function() {
		printf("rerouted");
		return true;
	})
	->setRequest("get", "/reroute-me")
	->route();
?>
--EXPECT--
rerouted


