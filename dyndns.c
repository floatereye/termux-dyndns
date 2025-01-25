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

void print_help() {
    printf("Usage: dyndns <url> [options]\n");
    printf("Options:\n");
    printf("  -f <file>     Path to JSON file (default: ./dyndns.json)\n");
    printf("  -d <delay>    Delay in seconds before execution (default: 2)\n");
    printf("  -n, --name    Name to display in output (default: dyndns)\n");
    printf("  -h, --help    Show this help message\n");
}

int parse_args(int argc, char *argv[], char **file, int *delay, char **name) {
    int option;
    static struct option long_options[] = {
        {"name", required_argument, 0, 'n'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    // Default values
    *file = DEFAULT_FILE;
    *delay = DEFAULT_DELAY;
    *name = DEFAULT_NAME;

    // Parse command-line options
    while ((option = getopt_long(argc, argv, "f:d:n:h", long_options, NULL)) != -1) {
        switch (option) {
            case 'f':
                *file = optarg;
                break;
            case 'd':
                *delay = atoi(optarg);
                break;
            case 'n':
                *name = optarg;
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
        fprintf(stderr, "Error: URL is required.\n");
        print_help();
        return 1;
    }

    return 0;
}

size_t write_callback(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

void dyndns_req(const char *url, const char *file_path) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error initializing curl\n");
        exit(1);
    }

    FILE *file = fopen(file_path, "w");
    if (!file) {
        fprintf(stderr, "Error opening file for writing\n");
        curl_easy_cleanup(curl);
        exit(1);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    if (curl_easy_perform(curl) != CURLE_OK) {
        fprintf(stderr, "Error performing request\n");
    }

    fclose(file);
    curl_easy_cleanup(curl);
}

void dyndns_addr(const char *file_path, const char *name) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        fprintf(stderr, "Error opening JSON file\n");
        exit(1);
    }

    struct json_object *parsed_json, *targets, *target, *thisip;
    char buffer[1024];
    fread(buffer, 1, sizeof(buffer), file);
    fclose(file);

    parsed_json = json_tokener_parse(buffer);
    if (!json_object_object_get_ex(parsed_json, "targets", &targets)) {
        fprintf(stderr, "Error parsing JSON: 'targets' not found\n");
        exit(1);
    }

    target = json_object_array_get_idx(targets, 0);
    if (!json_object_object_get_ex(target, "thisip", &thisip)) {
        fprintf(stderr, "Error parsing JSON: 'thisip' not found\n");
        exit(1);
    }

    printf("[%s] %s\n", name, json_object_get_string(thisip));
}

int main(int argc, char *argv[]) {
    char *file;
    int delay;
    char *name;

    if (parse_args(argc, argv, &file, &delay, &name)) {
        return 1;
    }

    char *url = argv[optind];

    sleep(delay);
    dyndns_req(url, file);
    dyndns_addr(file, name);


    return 0;
}

