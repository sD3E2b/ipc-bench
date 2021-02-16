#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#include "common/common.h"

#define FIFO_CLIENT_PATH "/tmp/ipc_bench_fifo_client"
#define FIFO_SERVER_PATH "/tmp/ipc_bench_fifo_server"

void cleanup(FILE *stream, void *buffer) {
	free(buffer);
	fclose(stream);
	if (remove(FIFO_CLIENT_PATH) == -1) {
		throw("Error removing FIFO");
	}
}

void communicate(FILE *server_stream,
								 FILE *client_stream,
								 struct Arguments *args) {
	int message;
	struct Benchmarks bench;
	void *buffer = malloc(args->size);

	setup_benchmarks(&bench);

	for (message = 0; message < args->count; ++message) {
		bench.single_start = now();

        if (fwrite(buffer, args->size, 1, server_stream) == 0) {
			throw("Error writing server stream");
		}

        fflush(server_stream);

        size_t elements = 0;
        while (elements == 0) {
            elements = fread(buffer, args->size, 1, client_stream);
        }

		benchmark(&bench);
	}

	evaluate(&bench, args);
	cleanup(client_stream, buffer);
}

FILE *open_server_fifo() {
	FILE *stream;

	// Because a FIFO is really just a file, we can
	// open a normal FILE* stream to it (in read mode)
	// Note that this call will block until the write-end
	// is opened by the server
	if ((stream = fopen(FIFO_SERVER_PATH, "w")) == NULL) {
		throw("Error opening stream to FIFO on client-side");
	}

	return stream;
}

FILE *open_client_fifo() {
	FILE *stream;

	// Wait for the server to create the FIFO
	// wait_for_signal(signal_action);

	// Create a FIFO object. Note that a FIFO is
	// really just a special file, which must be
	// opened by one process and to which then
	// both server and client can write using standard
	// c i/o functions. 0666 specifies read+write
	// access permissions for the user, group and world
	if (mkfifo(FIFO_CLIENT_PATH, 0666) > 0) {
		throw("Error creating FIFO");
	}
	// Because a FIFO is really just a file, we can
	// open a normal FILE* stream to it (in read mode)
	// Note that this call will block until the write-end
	// is opened by the server
	if ((stream = fopen(FIFO_CLIENT_PATH, "r")) == NULL) {
		throw("Error opening stream to FIFO on client-side");
	}

	return stream;
}

int main(int argc, char *argv[]) {
	// The file pointer we will associate with the FIFO
	FILE *server_stream;
	FILE *client_stream;

	struct Arguments args;
	parse_arguments(&args, argc, argv);

	server_stream = open_server_fifo();
	client_stream = open_client_fifo();

	communicate(server_stream, client_stream, &args);

	return EXIT_SUCCESS;
}
