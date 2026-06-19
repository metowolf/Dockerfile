ARG VERSION=6.0.0b3
ARG TARGETOS
ARG TARGETARCH

FROM ubuntu:20.04 AS builder
LABEL maintainer="metowolf <i@i-meto.com>"

ARG VERSION
ARG TARGETOS
ARG TARGETARCH

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
  && apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    dumb-init \
    libc-ares2 \
    libsodium23 \
    libssl1.1 \
    libuv1 \
    unzip \
    wget \
  && rm -rf /var/lib/apt/lists/*

RUN set -eux; \
  case "$TARGETARCH" in \
    "amd64") snell_arch="amd64" ;; \
    "386") snell_arch="i386" ;; \
    "arm64") snell_arch="aarch64" ;; \
    *) echo "Unsupported architecture: $TARGETARCH" >&2; exit 1 ;; \
  esac; \
  wget -O /tmp/snell-server.zip "https://dl.nssurge.com/snell/snell-server-v${VERSION}-${TARGETOS}-${snell_arch}.zip"; \
  unzip /tmp/snell-server.zip -d /tmp; \
  mv /tmp/snell-server /usr/local/bin/snell-server; \
  chmod +x /usr/local/bin/snell-server; \
  rm -rf /tmp/snell-server.zip; \
  snell-server --help >/dev/null

COPY entrypoint.c /tmp/entrypoint.c

RUN gcc -std=c11 -O2 -pipe /tmp/entrypoint.c -o /usr/local/bin/entrypoint \
  && strip /usr/local/bin/entrypoint \
  && rm -f /tmp/entrypoint.c

RUN set -eux; \
  mkdir -p /deps; \
  for lib in \
    libstdc++.so.6 \
    libgcc_s.so.1 \
    libm.so.6 \
    libcares.so.2 \
    libcrypto.so.1.1 \
    libssl.so.1.1 \
    libsodium.so.23 \
    libuv.so.1 \
  ; do \
    src=$(find /usr/lib /lib -name "$lib" 2>/dev/null | head -1); \
    [ -n "$src" ] && cp --parents "$src" /deps/ || echo "WARNING: $lib not found" >&2; \
  done

FROM gcr.io/distroless/base-debian12
LABEL maintainer="metowolf <i@i-meto.com>"

COPY --from=builder /usr/bin/dumb-init /usr/bin/dumb-init
COPY --from=builder /usr/local/bin/snell-server /usr/local/bin/snell-server
COPY --from=builder /usr/local/bin/entrypoint /usr/local/bin/entrypoint
COPY --from=builder /etc/ssl/certs/ca-certificates.crt /etc/ssl/certs/ca-certificates.crt
COPY --from=builder /deps/ /

ENV PSK=uYQwNqZbaIOMiZ6Zni8v5x0M09Y8bSK \
    LISTEN=0.0.0.0:9000 \
    TZ=Asia/Shanghai

VOLUME /etc/snell
ENTRYPOINT [ "dumb-init", "--", "/usr/local/bin/entrypoint" ]
CMD [ "snell-server", "-c", "/etc/snell/snell-server.conf" ]
