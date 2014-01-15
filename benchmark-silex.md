Silex Example
=============

Here's a silex benchmark test, the silex application is a standard bootstrap application, PHP5.7 (master) + Opcache (warm).

Standard Silex
==============

```php
<?php
include ("vendor/autoload.php");

$app = new Silex\Application();

$app->register(new Silex\Provider\TwigServiceProvider(), array(
    "twig.path" => sprintf("%s/twigs", __DIR__)
));

$app->get("/", function() use (&$app) {
    return $app["twig"]->render("index.twig", array(
        "date" => date("c")
    ));
});

$app->run();
?>
```

```
This is ApacheBench, Version 2.3 <$Revision: 1430300 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking localhost (be patient)
Completed 200 requests
Completed 400 requests
Completed 600 requests
Completed 800 requests
Completed 1000 requests
Completed 1200 requests
Completed 1400 requests
Completed 1600 requests
Completed 1800 requests
Completed 2000 requests
Finished 2000 requests


Server Software:        Apache/2.2.23
Server Hostname:        localhost
Server Port:            80

Document Path:          /
Document Length:        2558 bytes

Concurrency Level:      28
Time taken for tests:   2.619 seconds
Complete requests:      2000
Failed requests:        0
Write errors:           0
Total transferred:      5636000 bytes
HTML transferred:       5116000 bytes
Requests per second:    763.75 [#/sec] (mean)
Time per request:       36.661 [ms] (mean)
Time per request:       1.309 [ms] (mean, across all concurrent requests)
Transfer rate:          2101.81 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.1      0       1
Processing:     6   36  27.9     29     208
Waiting:        6   32  26.7     24     207
Total:          7   36  28.0     29     208

Percentage of the requests served within a certain time (ms)
  50%     29
  66%     40
  75%     50
  80%     56
  90%     74
  95%     88
  98%    107
  99%    130
 100%    208 (longest request)
```

Router Silex
============
Identical in functionality to the silex code, same content served.

```php
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
```

```
This is ApacheBench, Version 2.3 <$Revision: 1430300 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking localhost (be patient)
Completed 200 requests
Completed 400 requests
Completed 600 requests
Completed 800 requests
Completed 1000 requests
Completed 1200 requests
Completed 1400 requests
Completed 1600 requests
Completed 1800 requests
Completed 2000 requests
Finished 2000 requests


Server Software:        Apache/2.2.23
Server Hostname:        localhost
Server Port:            80

Document Path:          /router-silex.php
Document Length:        2558 bytes

Concurrency Level:      28
Time taken for tests:   1.643 seconds
Complete requests:      2000
Failed requests:        0
Write errors:           0
Total transferred:      5506000 bytes
HTML transferred:       5116000 bytes
Requests per second:    1217.18 [#/sec] (mean)
Time per request:       23.004 [ms] (mean)
Time per request:       0.822 [ms] (mean, across all concurrent requests)
Transfer rate:          3272.35 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.1      0       1
Processing:     5   23  15.2     20     113
Waiting:        5   22  15.0     20     113
Total:          5   23  15.2     20     113

Percentage of the requests served within a certain time (ms)
  50%     20
  66%     24
  75%     29
  80%     32
  90%     44
  95%     54
  98%     66
  99%     76
 100%    113 (longest request)
```

A nice improvement, but I'd wager *nothing that couldn't be done in userland* ...
