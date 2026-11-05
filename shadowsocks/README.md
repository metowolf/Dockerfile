# Shadowsocks

Debian 13 with official shadowsocks-rust v1.25.0 binaries for amd64/arm64 and
optional Cloudflare WARP egress. The download stage verifies the release's
SHA-256 checksum and copies `ssserver` and `sslocal` into the runtime image.
Cloudflare WARP is installed from its trixie repository in the same image, but
only starts when enabled.

```sh
docker build -t metowolf/shadowsocks:local shadowsocks
docker run -d --name shadowsocks \
  -p 8388:8388/tcp -p 8388:8388/udp \
  -e SS_PASSWORD='replace-this-password' metowolf/shadowsocks:local
```

The default is plain Shadowsocks with direct TCP/UDP egress; it needs no extra
capabilities, TUN device, or WARP registration. For the legacy Docker builder,
pass `--build-arg TARGETARCH=amd64` (or `arm64`) explicitly.

To route outbound connections through WARP:

```sh
docker run -d --name shadowsocks-warp \
  --cap-add NET_ADMIN --device /dev/net/tun \
  -p 8388:8388/tcp -e WARP=true \
  -e SS_PASSWORD='replace-this-password' \
  -v shadowsocks-warp-state:/var/lib/cloudflare-warp \
  metowolf/shadowsocks:local
docker logs -f shadowsocks-warp
```

Traffic path: Shadowsocks client → `ssserver` → native `outbound_proxy`
→ `socks5://127.0.0.1:1080` (WARP proxy mode) → WARP egress.
[shadowsocks-rust v1.25.0 added native server outbound chaining](https://github.com/shadowsocks/shadowsocks-rust/releases/tag/v1.25.0).
WARP mode forces `tcp_only`; do not publish UDP for this mode. Both WARP and
SS stop if either service exits. Proxy failures have no direct fallback.
Only the SS port is published; the WARP SOCKS listener binds to loopback.

| Environment variable | Default | Meaning |
| --- | --- | --- |
| `WARP` | `false` | `true`, `1`, or `yes` enables WARP; unset, empty, `false`, `0`, or `no` uses direct egress (case insensitive); other values are rejected |
| `SS_METHOD` | `chacha20-ietf-poly1305` | Shadowsocks cipher; client must match |
| `SS_PASSWORD` | `shadowsocks-lab` | Public **lab-only** password; replace for deployment; empty passwords are rejected |
| `SS_PORT` | `8388` | SS port, 1–65535; cannot be 1080 with WARP enabled; adjust Docker publishing if changed |
| `SS_LISTEN` | `0.0.0.0` | Shadowsocks bind address |
| `WARP_LICENSE` | unset | Optional WARP+ license; used only with WARP enabled |
| `WARP_CUSTOM_ENDPOINT` | unset | Optional WARP tunnel endpoint (`IP:port`); used only with WARP enabled |
| `WARP_TIMEOUT` | `120` | WARP startup deadline in seconds (1–9999); ignored in direct mode |

WARP startup registers if needed, reapplies proxy mode, and connects even with
existing state. It waits for `Connected` and a SOCKS request to
`https://1.1.1.1/cdn-cgi/trace` returning `warp=on` or `warp=plus` before starting
SS. That endpoint must be reachable. Timeout or a failed service stops the
container with an error and the tail of `/tmp/warp-svc.log`. `dumb-init` forwards
shutdown signals and reaps children. The state volume preserves registration;
do not share one volume between running WARP daemons.

To verify through a real Shadowsocks client, run the following on a Linux
Docker host with either server example above listening on port 8388:

```sh
docker run -d --name shadowsocks-client --network host --entrypoint sslocal \
  metowolf/shadowsocks:local -b 127.0.0.1:1088 -s 127.0.0.1:8388 \
  -m chacha20-ietf-poly1305 -k 'replace-this-password'
curl --noproxy '' --socks5 127.0.0.1:1088 --max-time 30 \
  https://1.1.1.1/cdn-cgi/trace
docker rm -f shadowsocks-client
```

With WARP enabled, expect `warp=on` or `warp=plus` and a WARP egress `ip=`.
In direct mode, the result reflects the Docker host's existing egress.
