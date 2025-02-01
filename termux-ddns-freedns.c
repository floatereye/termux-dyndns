#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <jq.h>
#include <getopt.h>

typedef struct {
    int delay;
    const char *output_file;
    const char *url;
    const char *jq_filter;
    int is_json;
} config_t;

typedef struct {
		const char *system;
		const char *host;
		const char *thisip;
		const char *lastip;
		const char *status_text;

		const int status_code;
		const int updated;
		const int ts;
} dns_record_t

void parse_response_json(const char buffer[], config_t config) {
    jq_state *jq = jq_init();
    if (!jq) {
        return;
    }

    if (!jq_compile(jq, config.jq_filter)) {
        fprintf(stderr, "Failed to compile jq filter: %s\n", config.jq_filter);
        jq_teardown(&jq);
        return;
    }

    jv input = jv_parse(buffer);
    if (!jv_is_valid(input)) {
        fprintf(stderr, "Invalid JSON input\n");
        jq_teardown(&jq);
        return;
    }

    jq_start(jq, input, 0);
    jv result;
    while (jv_is_valid(result = jq_next(jq))) {
        jv output = jv_dump_string(result, 0);
        printf("Result: %s\n", jv_string_value(output));
        jv_free(output);
    }

    jq_teardown(&jq);
}

void parse_dns_response(config_t config) {
    FILE *file = fopen(config.output_file, "r");
    if (!file) {
        perror("Error opening JSON file");
        return;
    }

    char buffer[1024];
    fread(buffer, 1, sizeof(buffer), file);
    fclose(file);
		printf("%s", buffer);

		if (config.is_json) {
				parse_response_json(buffer, config);
				return;
		}
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

    while ((option = getopt_long(argc, argv, "f:d:q:n:h", long_options, NULL)) != -1) {
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
            case 'q':
                config.jq_filter = optarg;
                break;
            case 'h':
                print_help(argv[0], config);
                return EXIT_SUCCESS;
            default:
                print_help(argv[0], config);
                return EXIT_SUCCESS;
        }
    }

    if (optind >= argc) {
		    printf("%s: URL is required.\n", progname);
		    return EXIT_FAILURE;
    }
    else {
    		char *url;
		    url = argv[optind];

  			CURLUcode rc;
  			CURLU *h = curl_url();
				rc = curl_url_set(h, CURLUPART_URL, url, CURLU_DEFAULT_SCHEME);
				if (rc != CURLUE_OK) {
	        printf("%s: URL is malformed: %s\n", progname, url);
	        print_help(argv[0], config);
					curl_url_cleanup(h);
	        return EXIT_FAILURE;
        }
        else {
					char *query;
					rc = curl_url_get(h, CURLUPART_QUERY, &query, 0);
					if (rc == CURLUE_OK && strcmp(query, "content-type=json") == 0) {
							config.is_json = 1;
					}
					char *path;
					rc = curl_url_get(h, CURLUPART_PATH, &path, 0);
					if (rc == CURLUE_OK && path[-1] != '/') {
							strcat(url, "/");
					}
        }
				curl_url_cleanup(h);
				config.url = url;
    }

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

    //sleep(config.delay);
    update_dns_record(config);
		if (config.is_json && !config.jq_filter) {
				config.jq_filter = ".targets[0].thisip";
		}
    parse_dns_response(config);

    return EXIT_SUCCESS;
}

