Not a benchmark
===============

The following is the output from ApacheBenchmark, serving router.php using worker-mpm apache, PHP5.7 (master) + Opcache (warm).

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

Document Path:          /router.php
Document Length:        27 bytes

Concurrency Level:      28
Time taken for tests:   0.195 seconds
Complete requests:      2000
Failed requests:        0
Write errors:           0
Total transferred:      440000 bytes
HTML transferred:       54000 bytes
Requests per second:    10261.15 [#/sec] (mean)
Time per request:       2.729 [ms] (mean)
Time per request:       0.097 [ms] (mean, across all concurrent requests)
Transfer rate:          2204.54 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    1   0.3      1       2
Processing:     0    2   0.6      2       5
Waiting:        0    1   0.5      1       4
Total:          1    3   0.5      3       6

Percentage of the requests served within a certain time (ms)
  50%      3
  66%      3
  75%      3
  80%      3
  90%      3
  95%      4
  98%      4
  99%      5
 100%      6 (longest request)
```

**NOTE: It is important to remember that this shows you only the performance of the router, executing close to nothing, on an idle machine**
