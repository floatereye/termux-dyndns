termux-ddns-freedns is a C program that updates your IP on FreeDNS (https://freedns.afraid.org) using their API and token, saves the response in a JSON file, and supports an optional delay.

```
Install:
./configure && make && make install
```

````
Usage: ./termux-ddns-freedns <url> [options]
Options:
  -f <file>     Path to JSON file (default: ./dyndns.json)
  -d <delay>    Delay in seconds before execution (default: 2)
  -n, --name    Name to display in output (default: dyndns)
  -h, --help    Show this help message
````
