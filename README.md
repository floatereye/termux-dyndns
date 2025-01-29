termux-ddns-freedns is a C program that uses FreeDNS's latest API with a token for manual update requests. It sends the request to https://freedns.afraid.org, saves the response in a JSON file, and supports an optional delay before execution.

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
