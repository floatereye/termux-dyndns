#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <getopt.h>

#define DEFAULT_FILE "./dyndns.json"
#define DEFAULT_DELAY 2
#define DEFAULT_NAME "dyndns"

typedef struct {
    char *file;
    int delay;
    char *name;
    char *url;
} Config;

void print_help() {
    printf("Usage: dyndns <url> [options]\n");
    printf("Options:\n");
    printf("  -f <file>     Path to JSON file (default: ./dyndns.json)\n");
    printf("  -d <delay>    Delay in seconds before execution (default: 2)\n");
    printf("  -n, --name    Name to display in output (default: dyndns)\n");
    printf("  -h, --help    Show this help message\n");
}

void print_error(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

int parse_args(int argc, char *argv[], Config *config) {
    int option;
    static struct option long_options[] = {
        {"name", required_argument, 0, 'n'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };


    config->file = DEFAULT_FILE;
    config->delay = DEFAULT_DELAY;
    config->name = DEFAULT_NAME;
    config->url = NULL;

    while ((option = getopt_long(argc, argv, "f:d:n:h", long_options, NULL)) != -1) {
        switch (option) {
            case 'f':
                config->file = optarg;
                break;
            case 'd':
                config->delay = atoi(optarg);
                break;
            case 'n':
                config->name = optarg;
                break;
            case 'h':
                print_help();
                return 1;
            default:
                print_help();
                return 1;
        }
    }

    if (optind >= argc) {
        print_error("URL is required.");
        print_help();
        return 1;
    }

    config->url = argv[optind];

    return 0;
}

size_t write_callback(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

void dyndns_req(Config config) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        print_error("Error initializing curl");
        return;
    }

    FILE *file = fopen(config.file, "w");
    if (!file) {
        print_error("Error opening file for writing");
        curl_easy_cleanup(curl);
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, config.url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    if (curl_easy_perform(curl) != CURLE_OK) {
        print_error("Error performing request");
    }

    fclose(file);
    curl_easy_cleanup(curl);
}

void dyndns_addr(Config config) {
    FILE *file = fopen(config.file, "r");
    if (!file) {
        print_error("Error opening JSON file");
        return;
    }

    struct json_object *parsed_json, *targets, *target, *thisip;
    char buffer[1024];
    fread(buffer, 1, sizeof(buffer), file);
    fclose(file);

    parsed_json = json_tokener_parse(buffer);
    if (!json_object_object_get_ex(parsed_json, "targets", &targets)) {
        print_error("Error parsing JSON: 'targets' not found");
        return;
    }

    target = json_object_array_get_idx(targets, 0);
    if (!json_object_object_get_ex(target, "thisip", &thisip)) {
        print_error("Error parsing JSON: 'thisip' not found");
        return;
    }

    printf("[%s] %s\n", config.name, json_object_get_string(thisip));
}

int main(int argc, char *argv[]) {
    Config config;

    if (parse_args(argc, argv, &config)) {
        return 1;
    }

    sleep(config.delay);
    dyndns_req(config);
    dyndns_addr(config);

    return 0;
}

