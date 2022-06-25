#! /bin/sh
docker build -t jetson-l4t-img -f img/Dockerfile --platform=linux/arm64 .
