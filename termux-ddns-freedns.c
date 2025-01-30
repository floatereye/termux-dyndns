#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <getopt.h>

typedef struct {
    int delay;
    const char *output_file;
    const char *url;
} config_t;

void parse_dns_response(config_t config) {
    FILE *file = fopen(config.output_file, "r");
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

void update_dns_record(config_t config) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "[ERROR] curl_easy_init() failed\n");
        return;
    }

    FILE *fp = fopen(config.output_file, "wb");
    if (!fp) {
        fprintf(stderr, "[ERROR] Failed to open output file\n");
        curl_easy_cleanup(curl);
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, config.url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "[ERROR] curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    fclose(fp);
    curl_easy_cleanup(curl);
}


int validate_url_format(const char *url) {
    if (strncmp(url, "http://", 7) == 0 || strncmp(url, "https://", 8) == 0) {
        return 1;
    }
    return 0;
}

void print_help(char progname[], config_t config) {
    printf("Usage: %s <url> [options]\n", progname);
    printf("Options:\n");
    printf("  -f <file>     Path to JSON file\n");
    printf("  -d <delay>    Delay in seconds before execution (default: %d)\n", config.delay);
    printf("  -h <ip>       Specify a ip instead of automatic server side IP detection\n");
    printf("  -h <host>     Hostname to update\n");
    printf("  -u <user>     HTTP user\n");
    printf("  -p <password> HTTP password\n");
    printf("  -a, --auth    Use HTTP Authentication\n");
    printf("  -h, --help    Show this help message\n");
}
int main(int argc, char *argv[]) {

    const char *progname = argv[0];
    config_t config = {0};
    config.delay = 2;

    int option;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    while ((option = getopt_long(argc, argv, "f:d:n:h", long_options, NULL)) != -1) {
        switch (option) {
            case 'f':
                if (1 + strlen(optarg) >= 255) {
                    fprintf(stderr, "%s: Path too long\n", progname);
                    exit(EXIT_FAILURE);
                }
                config.output_file = optarg;
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

    if (optind >= argc || !validate_url_format(argv[optind])) {
        printf("%s: URL is required.\n", progname);
        print_help(argv[0], config);
        return EXIT_FAILURE;
    }
    config.url = argv[optind];

    if (! config.output_file) {
        static char output_file[255];

        if (1 + strlen(progname + 5) >= 255) {
            fprintf(stderr, "%s: Path too long\n", progname);
            exit(EXIT_FAILURE);
        }

        strcat(output_file, progname);
        strcat(output_file, ".json");

        config.output_file = output_file;
    }

    sleep(config.delay);
    update_dns_record(config);
    parse_dns_response(config);

    return EXIT_SUCCESS;
}

