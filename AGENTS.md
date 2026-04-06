# AGENTS.md

## Cursor Cloud specific instructions

### Overview

This repository stores Dockerfiles for building base Docker images published to `ghcr.io/metowolf/`. There are three image families:

| Image Family | Dockerfiles | Description |
|---|---|---|
| **KonaJDK** | `konajdk/{8,11,17,21,25}/Dockerfile` | Tencent KonaJDK base images (multiple Java LTS versions) |
| **Snell** | `snell/Dockerfile` | Snell proxy server in distroless container |
| **Warp** | `warp/Dockerfile` | Cloudflare WARP client proxy |

### Prerequisites

Docker with Buildx is required. In Cloud Agent VMs, Docker needs special configuration (fuse-overlayfs storage driver, iptables-legacy) — these are set up by the environment snapshot.

### Building images

Build any image with `docker build`:

```bash
# Example: KonaJDK 21
cd konajdk/21 && docker build --platform linux/amd64 -t konajdk:21-test .

# Example: Snell
cd snell && docker build --platform linux/amd64 -t snell:test .

# Example: Warp
cd warp && docker build --platform linux/amd64 -t warp:test .
```

### Testing images

After building, verify with a quick smoke test:

```bash
docker run --rm konajdk:21-test java --version
docker run --rm --entrypoint "" snell:test /usr/local/bin/snell-server --version
```

### Gotchas

- The Docker daemon must be started before builds: `sudo dockerd &>/dev/null &` then wait a few seconds.
- Use `--platform linux/amd64` for local builds (CI uses `linux/amd64,linux/arm64` with QEMU).
- KonaJDK version maintenance guide is in `konajdk/CLAUDE.md`.
- There is no lint, test suite, or application server — "testing" means building images and running smoke tests.
- No `package.json`, `Makefile`, or other dependency managers exist; Docker is the only tool needed.
