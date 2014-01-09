--TEST--
Check no route found exception
--SKIPIF--
<?php include_once(sprintf("%s/skip-if.inc", __DIR__)) ?>
--FILE--
<?php 
$router = new Router();
$router->route();
?>
--EXPECTF--
Fatal error: Uncaught exception 'RoutingException' with message 'no route found' in %s:%d
Stack trace:
#0 %s(%d): Router->route()
#1 {main}
  thrown in %s on line %d

