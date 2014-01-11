--TEST--
Check console
--SKIPIF--
<?php include_once(sprintf("%s/skip-if.inc", __DIR__)) ?>
--FILE--
<?php 
$router = new Router();
$router->setConsole(function(){
	printf("I am the console");
	return true;
})->route();
?>
--EXPECT--
I am the console

