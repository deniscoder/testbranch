// logtail certified secure challenge

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

extern const char *__progname;

#define CFG_RELEASE_FILENAME "/etc/issue.net"

#define CFG_FILENAME "/etc/logtail.cfg"

#define CFG_MAX_LINE 256

#define CFG_MAX_FILE 16

#define CFG_MAX_MESSAGE 8192

#define CFG_LAST_LINES 10

#define CFG_MAX_LAST_LINES 64

struct configuration {

	unsigned int filecount;

	char hostname[CFG_MAX_LINE];

	char release[CFG_MAX_LINE];

	char filenames[CFG_MAX_FILE][CFG_MAX_LINE];
};

// variable argument log routine with implicit flush

void logmsg(char * fmt, ...) {

	char message[CFG_MAX_MESSAGE]; va_list ap;

	va_start(ap, fmt);

	vsnprintf(message, sizeof(message) - 1, fmt, ap);

	message[sizeof(message) - 1] = 0x0;

	va_end(ap);

	fprintf(stdout, "%s", message);

	fflush(stdout);
}

// wrapper around fgets that removes the optional newline

char * fgetsane(char * buffer, size_t size, FILE * file) {

	char * ptr, * result = fgets(buffer, size, file);

	if(!result) return result;

	if((ptr = strchr(result, 0xA))) *ptr = 0x00; 

	return result;
}

// load configuration parameters from config-file

void load_configuration(struct configuration * cfg) {

	FILE * file = 0, * release_file = 0; 

	if(!(release_file = fopen(CFG_RELEASE_FILENAME, "r"))) goto cleanup;

	if(!(file = fopen(CFG_FILENAME, "r"))) goto cleanup;

	bzero((char*)cfg, sizeof(struct configuration));

	if(gethostname(cfg->hostname, CFG_MAX_LINE - 1)) goto cleanup;

	if(!fgetsane(cfg->release, CFG_MAX_LINE - 1, release_file)) goto cleanup;

	while(cfg->filecount < CFG_MAX_FILE && 
			
			fgetsane(cfg->filenames[cfg->filecount], CFG_MAX_LINE - 1, file)) { 
		
		cfg->filecount++;
	}

cleanup:

	if(release_file) fclose(release_file);

	if(file) fclose(file);
}

// print the last lines from the specified filename 

void print_last_lines(char * filename, unsigned int count) {

	FILE * file; char ** buffer, * line = NULL;

	size_t line_size, offset, buffer_index = 0;

	if(!(file = fopen(filename, "r"))) return;

	if(!(buffer = calloc(sizeof(char*), count))) return;

	// read all the lines and keep "circular" array reference

	while(getline(&line, &line_size, file) != -1) {

		buffer_index = buffer_index % count; 

		if(buffer[buffer_index]) free(buffer[buffer_index]);
		
		buffer[buffer_index] = line; line = NULL; 

		buffer_index++; 
	}

	// output all the lines read (start at buffer_index)

	for(offset = 0; offset < count; offset++) { 

		buffer_index = buffer_index % count; 
	
		if(buffer[buffer_index]) {

			fprintf(stderr, "%s", buffer[buffer_index]);

			free(buffer[buffer_index]);
		}

		buffer_index++;
	}
}

// show the specified number of last lines from logfiles

void show_last_lines(int line_count) {

	struct configuration cfg;

	unsigned int index = 0;

	load_configuration(&cfg);

	logmsg("%s: %s - %s\n", __progname, cfg.release, cfg.hostname);

	while(index < cfg.filecount) {

		logmsg("%s: %s\n", __progname, cfg.filenames[index]);

		print_last_lines(cfg.filenames[index++], 16);
	};
}

// parse argument and show the last lines from logfiles

int main(int argc, char ** argv) {

	unsigned int line_count = CFG_LAST_LINES;

	if(argc > 1) {

		line_count = atoi(argv[1]);

		if(line_count > CFG_MAX_LAST_LINES) exit(1);
	}

	logmsg("%s: showing last %d lines\n", __progname, line_count);

	show_last_lines(line_count);

	return 0;
}
	



