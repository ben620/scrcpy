#include "getopt.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

char *optarg;
int optind = 1;
int opterr = 1;
int optopt;

static const char *short_cursor;

static const char *
find_short_option(const char *optstring, int option) {
    const char *p = optstring;
    while (*p) {
        if (*p == option) {
            return p;
        }
        ++p;
        while (*p == ':') {
            ++p;
        }
    }
    return NULL;
}

static int
parse_long_option(int argc, char *const argv[], const struct option *longopts,
                  int *longindex) {
    const char *argument = argv[optind] + 2;
    const char *separator = strchr(argument, '=');
    size_t name_len = separator ? (size_t) (separator - argument)
                                : strlen(argument);

    int match = -1;
    for (int i = 0; longopts[i].name; ++i) {
        if (strlen(longopts[i].name) == name_len
                && !strncmp(longopts[i].name, argument, name_len)) {
            match = i;
            break;
        }
    }

    if (match < 0) {
        if (opterr) {
            fprintf(stderr, "unrecognized option '--%.*s'\n",
                    (int) name_len, argument);
        }
        ++optind;
        optopt = 0;
        return '?';
    }

    const struct option *option = &longopts[match];
    optarg = NULL;
    if (option->has_arg == no_argument) {
        if (separator) {
            if (opterr) {
                fprintf(stderr, "option '--%s' does not allow an argument\n",
                        option->name);
            }
            ++optind;
            return '?';
        }
    } else if (separator) {
        optarg = (char *) separator + 1;
    } else if (option->has_arg == required_argument) {
        if (optind + 1 >= argc) {
            if (opterr) {
                fprintf(stderr, "option '--%s' requires an argument\n",
                        option->name);
            }
            ++optind;
            optopt = option->val;
            return '?';
        }
        optarg = argv[++optind];
    }

    ++optind;
    if (longindex) {
        *longindex = match;
    }
    if (option->flag) {
        *option->flag = option->val;
        return 0;
    }
    return option->val;
}

int
getopt_long(int argc, char *const argv[], const char *optstring,
            const struct option *longopts, int *longindex) {
    if (optind == 0) {
        optind = 1;
        short_cursor = NULL;
    }
    optarg = NULL;

    if (!short_cursor || !*short_cursor) {
        short_cursor = NULL;
        if (optind >= argc) {
            return -1;
        }

        const char *argument = argv[optind];
        if (!strcmp(argument, "--")) {
            ++optind;
            return -1;
        }
        if (argument[0] != '-' || argument[1] == '\0') {
            return -1;
        }
        if (argument[1] == '-') {
            return parse_long_option(argc, argv, longopts, longindex);
        }
        short_cursor = argument + 1;
    }

    int option = (unsigned char) *short_cursor++;
    const char *spec = find_short_option(optstring, option);
    if (!spec) {
        if (opterr) {
            fprintf(stderr, "invalid option -- '%c'\n", option);
        }
        optopt = option;
        if (!*short_cursor) {
            ++optind;
            short_cursor = NULL;
        }
        return '?';
    }

    bool requires_argument = spec[1] == ':';
    bool optional = requires_argument && spec[2] == ':';
    if (!requires_argument) {
        if (!*short_cursor) {
            ++optind;
            short_cursor = NULL;
        }
        return option;
    }

    if (*short_cursor) {
        optarg = (char *) short_cursor;
        ++optind;
        short_cursor = NULL;
        return option;
    }

    ++optind;
    short_cursor = NULL;
    if (!optional && optind < argc) {
        optarg = argv[optind++];
        return option;
    }
    if (optional) {
        return option;
    }

    if (opterr) {
        fprintf(stderr, "option requires an argument -- '%c'\n", option);
    }
    optopt = option;
    return '?';
}
