Silex Example
=============

Here's a silex benchmark test, the silex application is a standard bootstrap/twig test, apache prefork mpm, PHP5.7 (master) + Opcache (warm).

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
Server Port:            30000

Document Path:          /
Document Length:        2558 bytes

Concurrency Level:      28
Time taken for tests:   2.002 seconds
Complete requests:      2000
Failed requests:        0
Write errors:           0
Total transferred:      5636000 bytes
HTML transferred:       5116000 bytes
Requests per second:    999.18 [#/sec] (mean)
Time per request:       28.023 [ms] (mean)
Time per request:       1.001 [ms] (mean, across all concurrent requests)
Transfer rate:          2749.68 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.1      0       1
Processing:     8   28   4.0     27      57
Waiting:        8   27   3.9     26      44
Total:          9   28   3.9     27      57

Percentage of the requests served within a certain time (ms)
  50%     27
  66%     29
  75%     30
  80%     31
  90%     33
  95%     35
  98%     37
  99%     38
 100%     57 (longest request)
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
Server Port:            30000

Document Path:          /router-silex.php
Document Length:        2558 bytes

Concurrency Level:      28
Time taken for tests:   1.270 seconds
Complete requests:      2000
Failed requests:        0
Write errors:           0
Total transferred:      5506000 bytes
HTML transferred:       5116000 bytes
Requests per second:    1574.76 [#/sec] (mean)
Time per request:       17.780 [ms] (mean)
Time per request:       0.635 [ms] (mean, across all concurrent requests)
Transfer rate:          4233.71 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.1      0       1
Processing:     4   18   2.4     17      30
Waiting:        4   17   2.3     17      30
Total:          5   18   2.4     17      30

Percentage of the requests served within a certain time (ms)
  50%     17
  66%     18
  75%     18
  80%     19
  90%     21
  95%     22
  98%     24
  99%     26
 100%     30 (longest request)
```

A nice improvement, but I'd wager *nothing that couldn't be done in userland* ...
