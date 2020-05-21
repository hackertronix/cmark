#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "config.h"
#include "cmark.h"
#include "node.h"

#if defined(__OpenBSD__)
#  include <sys/param.h>
#  if OpenBSD >= 201605
#    define USE_PLEDGE
#    include <unistd.h>
#  endif
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <io.h>
#include <fcntl.h>
#endif

typedef enum {
    FORMAT_NONE,
    FORMAT_HTML,
    FORMAT_XML,
    FORMAT_MAN,
    FORMAT_COMMONMARK,
    FORMAT_LATEX
} writer_format;

void print_usage() {
    printf("Usage:   cmark [FILE*]\n");
    printf("Options:\n");
    printf("  --to, -t FORMAT  Specify output format (html, xml, man, "
           "commonmark, latex)\n");
    printf("  --width WIDTH    Specify wrap width (default 0 = nowrap)\n");
    printf("  --sourcepos      Include source position attribute\n");
    printf("  --hardbreaks     Treat newlines as hard line breaks\n");
    printf("  --nobreaks       Render soft line breaks as spaces\n");
    printf("  --unsafe         Render raw HTML and dangerous URLs\n");
    printf("  --smart          Use smart punctuation\n");
    printf("  --validate-utf8  Replace invalid UTF-8 sequences with U+FFFD\n");
    printf("  --help, -h       Print usage information\n");
    printf("  --version        Print version\n");
}

static void print_document(cmark_node *document, writer_format writer,
                           int options, int width) {
    char *result;

    switch (writer) {
        case FORMAT_HTML:
            result = cmark_render_html(document, options);
            break;
        case FORMAT_XML:
            result = cmark_render_xml(document, options);
            break;
        case FORMAT_MAN:
            result = cmark_render_man(document, options, width);
            break;
        case FORMAT_COMMONMARK:
            result = cmark_render_commonmark(document, options, width);
            break;
        case FORMAT_LATEX:
            result = cmark_render_latex(document, options, width);
            break;
        default:
            fprintf(stderr, "Unknown format %d\n", writer);
            exit(1);
    }

    //TODO return this result back
    printf("%s", result);
    document->mem->free(result);
}

int main(int argc, char *argv[]) {
    int i, numfps = 0;
    int *files;
    char buffer[4096];
    cmark_parser *parser;
    size_t bytes;
    cmark_node *document;
    int width = 0;
    char *unparsed;
    writer_format writer = FORMAT_HTML;
    int options = CMARK_OPT_DEFAULT;

#ifdef USE_PLEDGE
    if (pledge("stdio rpath", NULL) != 0) {
    perror("pledge");
    return 1;
  }
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
    _setmode(_fileno(stdin), _O_BINARY);
  _setmode(_fileno(stdout), _O_BINARY);
#endif
    for (i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-s") == 0)) {
            i += 1;
            if (i < argc) {
                strcpy(buffer, argv[i]);
            }
        }
    }

    parser = cmark_parser_new(options);
    cmark_parser_feed(parser, buffer, strlen(buffer));

#ifdef USE_PLEDGE
    if (pledge("stdio", NULL) != 0) {
    perror("pledge");
    return 1;
  }
#endif

    document = cmark_parser_finish(parser);
    cmark_parser_free(parser);

    print_document(document, writer, options, width);

    cmark_node_free(document);

//    free(files);

    return 0;
}