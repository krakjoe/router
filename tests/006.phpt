--TEST--
Check argument handling (default)
--SKIPIF--
<?php include_once(sprintf("%s/skip-if.inc", __DIR__)) ?>
--FILE--
<?php 
$router = new Router();
$router->setDefault(1);
?>
--EXPECTF--
Fatal error: Uncaught exception 'RoutingException' with message 'handler is not callable' in %s:%d
Stack trace:
#0 %s(%d): Router->setDefault(1)
#1 {main}
  thrown in %s on line %d


