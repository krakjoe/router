--TEST--
Check argument handling (coerce)
--SKIPIF--
<?php include_once(sprintf("%s/skip-if.inc", __DIR__)) ?>
--FILE--
<?php 
$router = new Router();
$router->addRoute(1, 1, function(){});
echo "yay";
?>
--EXPECT--
yay
