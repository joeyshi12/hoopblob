FROM devkitpro/devkitarm:20260610

RUN apt-get update && apt-get install -y --no-install-recommends python3 git && \
    rm -rf /var/lib/apt/lists/*

ARG BUTANO_TAG=21.7.0
RUN git clone --depth 1 --branch ${BUTANO_TAG} https://github.com/GValiente/butano.git /opt/butano

WORKDIR /hoopblob
