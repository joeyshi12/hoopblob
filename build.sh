#!/usr/bin/env bash
set -e
docker compose run --rm build
echo "Build complete! .gba ROM is in the project root."
