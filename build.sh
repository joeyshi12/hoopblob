#!/usr/bin/env bash
set -e

# Use the legacy builder so the build works without the buildx plugin.
export DOCKER_BUILDKIT=0
export COMPOSE_DOCKER_CLI_BUILD=0

docker-compose run --rm build
echo "Build complete! .gba ROM is in the project root."
