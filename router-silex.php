<?php
include ("vendor/autoload.php");

$app = new Silex\Application();
$router = new Router();

$app->register(new Silex\Provider\TwigServiceProvider(), array(
    "twig.path" => sprintf("%s/twigs", __DIR__)
));

$router->setDefault(function() use (&$app) {
    echo $app["twig"]->render("index.twig", array(
        "date" => date("c")
    ));
    return true;
});

$router->route();
?>
