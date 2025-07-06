FROM debian:bullseye-slim
LABEL maintainer="metowolf <i@i-meto.com>"

ARG TARGETOS
ARG TARGETARCH
ARG VERSION=5.0.0b2

RUN apt-get update \
  && apt-get install -y \
    wget \
    unzip \
    dumb-init \
  \
  && case "$TARGETARCH" in \
       "amd64") SNELL_ARCH="amd64" ;; \
       "arm64") SNELL_ARCH="aarch64" ;; \
       *) echo "Unsupported architecture: $TARGETARCH" && exit 1 ;; \
     esac \
  \
  && wget -O snell-server.zip https://dl.nssurge.com/snell/snell-server-v$VERSION-$TARGETOS-$SNELL_ARCH.zip \
  && unzip snell-server.zip \
  && mv snell-server /usr/local/bin \
  && rm -rf snell-server.zip \
  && snell-server --help \
  \
  && apt-get remove -y \
    wget \
    unzip \
  && rm -rf /var/lib/apt/lists/*

# 添加默认环境变量
ENV PSK=uYQwNqZbaIOMiZ6Zni8v5x0M09Y8bSK \
    LISTEN=0.0.0.0:9000

# 更新入口脚本，支持环境变量
RUN echo '#!/bin/sh\n\
mkdir -p /etc/snell\n\
echo "[snell-server]\n\
listen = $LISTEN\n\
psk = $PSK\n\
ipv6 = false" > /etc/snell/snell-server.conf\n\
exec "$@"' > /entrypoint.sh \
  && chmod +x /entrypoint.sh

VOLUME /etc/snell
ENV TZ=Asia/Shanghai
CMD [ "snell-server", "-c", "/etc/snell/snell-server.conf" ]
ENTRYPOINT [ "dumb-init", "--", "/entrypoint.sh" ]