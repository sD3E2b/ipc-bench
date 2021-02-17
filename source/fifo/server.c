#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unistd.h>

#include "common/common.h"

#define FIFO_CLIENT_PATH "/tmp/ipc_bench_fifo_client"
#define FIFO_SERVER_PATH "/tmp/ipc_bench_fifo_server"

void cleanup(FILE* stream, void* buffer) {
	free(buffer);
	fclose(stream);
	if (remove(FIFO_SERVER_PATH) == -1) {
		throw("Error removing FIFO");
	}
}

void communicate(FILE* server_stream,
                                 FILE* client_stream,
								 struct Arguments* args) {
	void* buffer;

	buffer = malloc(args->size);

	for (; args->count > 0; --args->count) {


        if (fread(buffer, args->size, 1, server_stream) == 0) {
			throw("Error writing server stream");
        }

		if (fwrite(buffer, args->size, 1, client_stream) == 0) {
			throw("Error writing client stream");
		}
		// Send off immediately (for small buffers)
		fflush(client_stream);
	}

	cleanup(server_stream, buffer);
}

FILE* open_server_fifo() {
	FILE* stream;

	// Just in case it already exists
	//(void)remove(FIFO_PATH);

	// Create a FIFO object. Note that a FIFO is
	// really just a special file, which must be
	// opened by one process and to which then
	// both server and client can write using standard
	// c i/o functions. 0666 specifies read+write
	// access permissions for the user, group and world
	if (mkfifo(FIFO_SERVER_PATH, 0666) > 0) {
		throw("Error creating FIFO");
	}

	// Because a fifo is really just a file, we can
	// open a normal FILE* stream to it (in write mode)
	// Note that this call will block until the read-end
	// is opened by the client
	if ((stream = fopen(FIFO_SERVER_PATH, "r")) == NULL) {
		throw("Error opening descriptor to FIFO on server side");
	}

	return stream;
}

FILE* open_client_fifo() {
	FILE* stream = NULL;

	// Just in case it already exists
	//(void)remove(FIFO_PATH);

	// Because a fifo is really just a file, we can
	// open a normal FILE* stream to it (in write mode)
	// Note that this call will block until the read-end
	// is opened by the client
	if ((stream = fopen(FIFO_CLIENT_PATH, "w")) == NULL) {
		throw("Error opening client stream server-side");
	}

	return stream;
}

int main(int argc, char* argv[]) {
	// The file pointer we will associate with the FIFO
	FILE* server_stream;
	FILE* client_stream;

	struct Arguments args;
	parse_arguments(&args, argc, argv);

	server_stream = open_server_fifo();
	client_stream = open_client_fifo();

	communicate(server_stream, client_stream, &args);

	return EXIT_SUCCESS;
}
