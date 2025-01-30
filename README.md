termux-ddns-freedns updates your IP on FreeDNS (https://freedns.afraid.org) using their API and token, saves the response in a JSON file, and supports an optional delay.

```
Install:
./configure && make && make install
```

````
Usage: ./termux-ddns-freedns <url> [options]
Options:
  -f <file>     Path to JSON file
  -d <delay>    Delay in seconds before execution (default: 2)
  -h <ip>       Specify a ip instead of automatic server side IP detection
  -h <host>     Hostname to update
  -u <user>     HTTP user
  -p <password> HTTP password
  -a, --auth    Use HTTP Authentication
  -h, --help    Show this help message
````
