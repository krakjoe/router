--TEST--
Check Route API behaviour
--SKIPIF--
<?php include_once(sprintf("%s/skip-if.inc", __DIR__)) ?>
--FILE--
<?php
try {
	class DefaultRoute implements Route {
		public function getMethod() { return "get"; }
		public function getURI()    { return "~/(.*)~"; }
		public function handle($patterns) { return true; }
	}
	
	$router = new Router();
	$router
		->addRoute(new DefaultRoute())
		->setRequest("get", "/whatever")
		->route();
} catch(RoutingException $ex) {
	printf(
		"<pre>Exception: %s\n</pre>", (string)$ex);
}
?>
--EXPECT--
