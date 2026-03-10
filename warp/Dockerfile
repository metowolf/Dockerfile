FROM debian:12

ARG TARGETARCH

RUN apt-get update && apt-get install -y curl gnupg unzip \
    && rm -rf /var/lib/apt/lists/*

RUN curl https://pkg.cloudflareclient.com/pubkey.gpg | gpg --yes --dearmor --output /usr/share/keyrings/cloudflare-warp-archive-keyring.gpg \
    && echo "deb [arch=${TARGETARCH} signed-by=/usr/share/keyrings/cloudflare-warp-archive-keyring.gpg] https://pkg.cloudflareclient.com/ bookworm main" | tee /etc/apt/sources.list.d/cloudflare-client.list \
    && apt-get update \
    && apt-get install -y cloudflare-warp \
    && rm -rf /var/lib/apt/lists/* \
    && mkdir -p /root/.local/share/warp \
    && echo -n 'yes' > /root/.local/share/warp/accepted-tos.txt

COPY entry /usr/local/bin/entry

ENTRYPOINT [ "entry" ]