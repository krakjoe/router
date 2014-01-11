--TEST--
Check Route API return value exceptions (handle)
--SKIPIF--
<?php include_once(sprintf("%s/skip-if.inc", __DIR__)) ?>
--FILE--
<?php
class DefaultRoute implements Route {
	public function getMethod() { return "get"; }
	public function getURI()    { return "~(.*)~"; }
	public function handle($patterns) {}
}

$router = new Router();
$router
	->addRoute(new DefaultRoute())
	->setRequest("get", "/whatever")
	->route();
?>
--EXPECTF--
Fatal error: Uncaught exception 'RoutingException' with message 'route did not return an appropriate value' in %s:%d
Stack trace:
#0 %s(%d): Router->route()
#1 {main}
  thrown in %s on line %d

