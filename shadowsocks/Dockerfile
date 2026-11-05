ARG VERSION=1.25.0

FROM debian:13 AS builder
LABEL maintainer="metowolf <i@i-meto.com>"

ARG VERSION
ARG TARGETARCH

RUN apt-get update \
  && apt-get install -y --no-install-recommends ca-certificates curl xz-utils \
  && rm -rf /var/lib/apt/lists/*

RUN set -eux; \
  case "$TARGETARCH" in \
    amd64) ss_arch=x86_64 ;; \
    arm64) ss_arch=aarch64 ;; \
    *) echo "Unsupported architecture: $TARGETARCH" >&2; exit 1 ;; \
  esac; \
  archive="shadowsocks-v${VERSION}.${ss_arch}-unknown-linux-musl.tar.xz"; \
  url="https://github.com/shadowsocks/shadowsocks-rust/releases/download/v${VERSION}"; \
  cd /tmp; \
  curl -fSL --retry 3 -O "${url}/${archive}"; \
  curl -fSL --retry 3 -O "${url}/${archive}.sha256"; \
  sha256sum -c "${archive}.sha256"; \
  tar -xJf "$archive" -C /usr/local/bin ssserver sslocal; \
  ssserver --version; \
  ssserver --help | grep -q -- --outbound-proxy

FROM debian:13
LABEL maintainer="metowolf <i@i-meto.com>"

ARG TARGETARCH

RUN apt-get update \
  && apt-get install -y --no-install-recommends \
    ca-certificates curl dumb-init gnupg jq \
  && curl -fsSL --retry 3 https://pkg.cloudflareclient.com/pubkey.gpg \
    | gpg --batch --yes --dearmor --output /usr/share/keyrings/cloudflare-warp-archive-keyring.gpg \
  && echo "deb [arch=${TARGETARCH} signed-by=/usr/share/keyrings/cloudflare-warp-archive-keyring.gpg] https://pkg.cloudflareclient.com/ trixie main" \
    > /etc/apt/sources.list.d/cloudflare-client.list \
  && apt-get update \
  && apt-get install -y --no-install-recommends cloudflare-warp \
  && rm -rf /var/lib/apt/lists/* \
  && mkdir -p /root/.local/share/warp \
  && printf yes > /root/.local/share/warp/accepted-tos.txt

COPY --from=builder /usr/local/bin/ssserver /usr/local/bin/sslocal /usr/local/bin/
COPY entry /usr/local/bin/entry

# Public lab password; override SS_PASSWORD for deployment.
ENV WARP=false \
    SS_PASSWORD=shadowsocks-lab \
    SS_METHOD=chacha20-ietf-poly1305 \
    SS_PORT=8388 \
    SS_LISTEN=0.0.0.0 \
    WARP_TIMEOUT=120

EXPOSE 8388/tcp 8388/udp

ENTRYPOINT [ "dumb-init", "--", "/usr/local/bin/entry" ]
