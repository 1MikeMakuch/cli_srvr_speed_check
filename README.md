# cli_srvr_speed_check

Measure network throughput between 2 endpoints. Just run the server on some host then
hit it with the client on some other host. Use to confirm speed of connections, narrow
down bottlenecks etc.

./srvr
usage srvr -p port [-d]

./cli
usage: cli -h host -p port -n nrequests -s size(max 1000000) -d


$ ./srvr -p 10101 -d

waiting for connection
server: got connection from 72.182.122.41
sizeofb2r 4
got 1e+06 bytes

waiting for connection
server: got connection from 72.182.122.41
sizeofb2r 4
got 1e+07 bytes

waiting for connection



$ ./cli  -h 54.193.56.105 -p 10101 -n 1000 -s 1000 -d
reqs: 1000, bytes sent: 1e+06, bytes srvr received: 1e+06
round trip secs: 1, bytes/sec:1e+06

$ ./cli  -h 54.193.56.105 -p 10101 -n 1000 -s 10000 -d
reqs: 1000, bytes sent: 1e+07, bytes srvr received: 1e+07
round trip secs: 14, bytes/sec:714286

$




