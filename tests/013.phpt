--TEST--
Check Route API return value exceptions (getURI)
--SKIPIF--
<?php include_once(sprintf("%s/skip-if.inc", __DIR__)) ?>
--FILE--
<?php
class DefaultRoute implements Route {
	public function getMethod() { return "get"; }
	public function getURI()    { return 1; }
	public function handle($patterns) { return true; }
}

$router = new Router();
$router
	->addRoute(new DefaultRoute())
	->setRequest("get", "/whatever")
	->route();
?>
--EXPECTF--
Fatal error: Uncaught exception 'RoutingException' with message 'getURI returned a non-string value' in %s:%d
Stack trace:
#0 %s(%d): Router->addRoute(Object(DefaultRoute))
#1 {main}
  thrown in %s on line %d

