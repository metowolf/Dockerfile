# Shadowsocks (with optional Cloudflare WARP egress)

Debian 13, official `shadowsocks-rust` v1.25.0 release binaries for `amd64`/`arm64`,
and Cloudflare's `trixie` WARP client package. The build stage verifies the
release's SHA-256 checksum and copies `ssserver` and `sslocal` into the runtime
image.

## Modes

- **Standard mode (`WARP=false` or unset, default)**: runs plain `ssserver` in
  `tcp_and_udp` mode with direct egress.
- **WARP egress mode (`WARP=true`)**: starts `cloudflare-warp` in local SOCKS
  proxy mode and chains `ssserver` outbound through WARP via native
  `outbound_proxy` ([added in shadowsocks-rust v1.25.0](https://github.com/shadowsocks/shadowsocks-rust/releases/tag/v1.25.0),
  no `LD_PRELOAD` wrapper needed).
  - Traffic path: Shadowsocks client → `ssserver` → `outbound_proxy`
    → `127.0.0.1:1080` (`socat`) → `127.0.0.1:1081` (WARP SOCKS) → WARP egress.
  - Target hostnames are passed to the SOCKS proxy. Failed proxy connections
    fail the request; there is no direct fallback.
  - Enforces `tcp_only`: WARP proxy mode does not relay UDP, so UDP is disabled
    when `WARP=true`. Both SOCKS listeners bind only to `127.0.0.1`.

## Build

```sh
DOCKER_BUILDKIT=0 docker build --build-arg TARGETARCH=amd64 -t metowolf/shadowsocks:latest shadowsocks
```

## Run

### Plain Shadowsocks (`WARP=false` / default)

```sh
docker run -d --name shadowsocks \
  -p 8388:8388/tcp -p 8388:8388/udp \
  -e SS_PASSWORD='replace-this-password' \
  metowolf/shadowsocks:latest
docker logs -f shadowsocks
```

### With Cloudflare WARP egress (`WARP=true`)

```sh
docker run -d --name shadowsocks \
  -p 8388:8388/tcp \
  -e WARP=true \
  -e SS_PASSWORD='replace-this-password' \
  -v shadowsocks-warp-state:/var/lib/cloudflare-warp \
  metowolf/shadowsocks:latest
docker logs -f shadowsocks
```

For host networking, replace `-p ...` with `--network host`. When `WARP=true`,
ports `1080`, `1081`, and `SS_PORT` must be free on that host, and host
networking also allows other host processes to access the local `127.0.0.1`
WARP SOCKS listeners.

## Environment Variables

| Environment variable | Default | Meaning |
| --- | --- | --- |
| `WARP` | `false` | Set to `true` to start Cloudflare WARP in proxy mode and route `ssserver` outbound via local WARP SOCKS (`tcp_only`); when `false` or unset, runs plain Shadowsocks (`tcp_and_udp`) without WARP |
| `SS_PASSWORD` | `shadowsocks-lab` | Required at runtime (must not be empty). Default is a public **lab-only** password; replace for deployment |
| `SS_METHOD` | `chacha20-ietf-poly1305` | Shadowsocks cipher; client must match |
| `SS_PORT` | `8388` | Shadowsocks listen port (`1..65535`, excluding `1080`/`1081` when `WARP=true`); adjust Docker port publishing if changed |
| `SS_LISTEN` | `0.0.0.0` | Shadowsocks bind address |
| `WARP_LICENSE` | unset | Optional WARP+ license key (used when `WARP=true`) |
| `WARP_CUSTOM_ENDPOINT` | unset | Optional WARP tunnel endpoint `IP:port` (used when `WARP=true`) |
| `WARP_TIMEOUT` | `120` | WARP startup deadline in seconds (`1..9999`, used when `WARP=true`) |

## WARP Startup & Lifecycle (`WARP=true`)

When `WARP=true`, startup waits for `warp-svc`, registers if
`/var/lib/cloudflare-warp/reg.json` is absent, and always reapplies proxy mode,
port `1081`, and `connect` (including with existing persisted state). It waits
for `Connected` status and a successful SOCKS request to
`https://1.1.1.1/cdn-cgi/trace` returning `warp=on` (or `warp=plus`) before
starting `ssserver`. That trace endpoint must be reachable. Timeout or exit of
any supervised process (`warp-svc`, `socat`, or `ssserver`) stops the container
with an error and the tail of `/tmp/warp-svc.log`. `dumb-init` forwards
shutdown signals and reaps child processes. Mounting
`/var/lib/cloudflare-warp` preserves registration across container recreation;
do not share one state volume between concurrently running containers.

## Verification

Test with a real Shadowsocks client using `sslocal` included in the image (host
networking example):

```sh
docker run -d --name shadowsocks-client --network host --entrypoint sslocal \
  metowolf/shadowsocks:latest -b 127.0.0.1:1088 -s 127.0.0.1:8388 \
  -m chacha20-ietf-poly1305 -k 'replace-this-password'
curl --noproxy '' --socks5 127.0.0.1:1088 --max-time 30 \
  https://1.1.1.1/cdn-cgi/trace
docker rm -f shadowsocks-client
```

When `WARP=true`, expect `warp=on` (or `warp=plus`) and a Cloudflare WARP
`ip=` egress address. Using the literal IP destination avoids host fake-IP DNS
(such as `198.18.0.53`); using only `--socks5-hostname` with an arbitrary
domain is insufficient proof on such hosts.
