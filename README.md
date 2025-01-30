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
  -u <user>     HTTP basic auth user
  -p <password> HTTP basic auth password
  -h, --help    Show this help message
````
