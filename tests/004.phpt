--TEST--
Check routing
--SKIPIF--
<?php include_once(sprintf("%s/skip-if.inc", __DIR__)) ?>
--FILE--
<?php 
$router = new Router();
$router
	->addHandler("get", "~/first/?([^/]+)?/?([^/]+)/?~", function($patterns){
		var_dump($patterns);
	})
	->setRequest("get", "/first/second/third")
	->route();
?>
--EXPECT--
array(3) {
  [0]=>
  string(19) "/first/second/third"
  [1]=>
  string(6) "second"
  [2]=>
  string(5) "third"
}


