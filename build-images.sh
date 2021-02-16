DOCKER_BUILDKIT=1 docker build -f ipc.dockerfile --target ipc-servers --tag ipc:servers .
DOCKER_BUILDKIT=1 docker build -f ipc.dockerfile --target ipc-clients --tag ipc:clients .
DOCKER_BUILDKIT=1 docker build -f ipc.dockerfile --target debug --tag ipc:debug .
docker image prune
