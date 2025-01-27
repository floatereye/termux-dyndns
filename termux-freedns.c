#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <getopt.h>

typedef struct {
    const char *file;
    int delay;
    const char *url;
} Config;

size_t write_callback(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

void dyndns_req(Config config) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        perror("Error initializing curl");
        curl_easy_cleanup(curl);
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(config.file, "w");
    if (!file) {
        perror("Error opening file for writing");
        curl_easy_cleanup(curl);
        exit(EXIT_FAILURE);
    }

    char errbuf[CURL_ERROR_SIZE];

    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curl, CURLOPT_URL, config.url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    CURLcode res;
    errbuf[0] = 0;
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      perror(curl_easy_strerror(res));
      curl_easy_cleanup(curl);
      fclose(file);
      exit(EXIT_FAILURE);
    }

    fclose(file);
    curl_easy_cleanup(curl);
}

void dyndns_addr(Config config) {
    FILE *file = fopen(config.file, "r");
    if (!file) {
        perror("Error opening JSON file");
        return;
    }

    struct json_object *parsed_json, *targets, *target, *thisip;
    char buffer[1024];
    fread(buffer, 1, sizeof(buffer), file);
    fclose(file);

    parsed_json = json_tokener_parse(buffer);
    if (!json_object_object_get_ex(parsed_json, "targets", &targets)) {
        perror("Error parsing JSON: 'targets' not found");
        exit(EXIT_FAILURE);
    }

    target = json_object_array_get_idx(targets, 0);
    if (!json_object_object_get_ex(target, "thisip", &thisip)) {
        perror("Error parsing JSON: 'thisip' not found");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", json_object_get_string(thisip));
}

void print_help(Config config) {
    printf("Usage: dyndns <url> [options]\n");
    printf("Options:\n");
    printf("  -f <file>     Path to JSON file (default: %s)\n", config.file);
    printf("  -d <delay>    Delay in seconds before execution (default: 2)\n");
    printf("  -h, --help    Show this help message\n");
}

int main(int argc, char *argv[]) {
    Config config = {0};

    int option;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    config.file = "freedns.json";
    config.delay = 3;
    config.url = NULL;

    while ((option = getopt_long(argc, argv, "f:d:n:h", long_options, NULL)) != -1) {
        switch (option) {
            case 'f':
                config.file = optarg;
                break;
            case 'd':
                config.delay = atoi(optarg);
                break;
            case 'h':
                print_help(config);
                return 1;
            default:
                print_help(config);
                return 1;
        }
    }

    if (optind >= argc) {
        perror("URL is required.");
        print_help(config);
        return 1;
    }

    config.url = argv[optind];

    sleep(config.delay);
    dyndns_req(config);
    dyndns_addr(config);

    return EXIT_SUCCESS;
}

