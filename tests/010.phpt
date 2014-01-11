--TEST--
Check argument handling (coerce vars)
--SKIPIF--
<?php include_once(sprintf("%s/skip-if.inc", __DIR__)) ?>
--FILE--
<?php 
$router = new Router();
$method = 1;
$uri = 1;
$router->addRoute($method, $uri, function(){});
var_dump($method);
var_dump($uri);
?>
--EXPECT--
int(1)
int(1)
