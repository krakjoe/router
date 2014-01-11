--TEST--
Check default
--SKIPIF--
<?php include_once(sprintf("%s/skip-if.inc", __DIR__)) ?>
--FILE--
<?php 
$router = new Router();
$router
	->setDefault(function(){
		printf("I am the default");
		return true;
	})
	->setRequest("get", "/")
	->route();
?>
--EXPECT--
I am the default

