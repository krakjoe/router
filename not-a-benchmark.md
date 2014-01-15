Not a benchmark
===============

The following is the output from ApacheBenchmark, serving router.php using prefork mpm apache, PHP5.7 (master) + Opcache (warm).

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

Document Path:          /router.php
Document Length:        27 bytes

Concurrency Level:      28
Time taken for tests:   0.173 seconds
Complete requests:      2000
Failed requests:        0
Write errors:           0
Total transferred:      440000 bytes
HTML transferred:       54000 bytes
Requests per second:    11543.08 [#/sec] (mean)
Time per request:       2.426 [ms] (mean)
Time per request:       0.087 [ms] (mean, across all concurrent requests)
Transfer rate:          2479.96 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    1   0.1      1       1
Processing:     0    2   0.2      2       3
Waiting:        0    2   0.3      2       3
Total:          1    2   0.2      2       3

Percentage of the requests served within a certain time (ms)
  50%      2
  66%      2
  75%      2
  80%      2
  90%      3
  95%      3
  98%      3
  99%      3
 100%      3 (longest request)
```

**NOTE: It is important to remember that this shows you only the performance of the router, executing close to nothing, on an idle machine**
