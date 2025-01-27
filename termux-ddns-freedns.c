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
      perror(config.url);
      perror(curl_easy_strerror(res));
      curl_easy_cleanup(curl);
      fclose(file);
      exit(EXIT_FAILURE);
    }

    fclose(file);
    curl_easy_cleanup(curl);
}

void print_help(char* progname, Config config) {
    printf("Usage: %s <url> [options]\n", progname);
    printf("Options:\n");
    printf("  -f <file>     Path to JSON file (default: %s)\n", config.file);
    printf("  -d <delay>    Delay in seconds before execution (default: %d)\n", config.delay);
    printf("  -h, --help    Show this help message\n");
}



int is_valid_url(const char *url) {
    if (strncmp(url, "http://", 7) == 0 || strncmp(url, "https://", 8) == 0) {
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {

    const char *progname = argv[0];

    size_t progname_len = strlen(progname);
    char *file = malloc(progname_len + 6); // +6 for ".json" and null terminator
    snprintf(file, progname_len + 6, "%s.json", progname);
    Config config = {file, 2, NULL};
    free(file);

    int option;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    while ((option = getopt_long(argc, argv, "f:d:n:h", long_options, NULL)) != -1) {
        switch (option) {
            case 'f':
                config.file = optarg;
                break;
            case 'd':
                config.delay = atoi(optarg);
                break;
            case 'h':
                print_help(argv[0], config);
                return EXIT_SUCCESS;
            default:
            		print_help(argv[0], config);
            		return EXIT_SUCCESS;

        }
    }

    if (optind >= argc || !is_valid_url(argv[optind])) {
        printf("URL is required.\n");
        print_help(argv[0], config);
		    return EXIT_FAILURE;
    }
    config.url = argv[optind];

    sleep(config.delay);
    dyndns_req(config);
    dyndns_addr(config);

    return EXIT_SUCCESS;
}

