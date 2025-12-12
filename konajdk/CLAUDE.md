# KonaJDK Docker Images - Maintenance Guide

This document provides instructions and best practices for maintaining KonaJDK Docker images.

## Version Update Process

### 1. Check Latest Versions

Use GitHub CLI to fetch the latest release versions from Tencent's repositories:

```bash
# KonaJDK 8
gh release list --repo Tencent/TencentKona-8 --limit 5

# KonaJDK 11
gh release list --repo Tencent/TencentKona-11 --limit 5

# KonaJDK 17
gh release list --repo Tencent/TencentKona-17 --limit 5

# KonaJDK 21
gh release list --repo Tencent/TencentKona-21 --limit 5
```

### 2. Verify Asset Names

**IMPORTANT**: Always verify the actual asset filenames in the new release before updating Dockerfiles.

```bash
# Check KonaJDK 8 assets (note: version-specific suffix may change)
gh release view <TAG> --repo Tencent/TencentKona-8 --json assets --jq '.assets[] | select(.name | contains("jdk_linux")) | .name'

# Check other versions
gh release view <TAG> --repo Tencent/TencentKona-11 --json assets --jq '.assets[] | select(.name | contains("jdk_linux")) | select(.name | contains("x86_64")) | select(.name | endswith(".tar.gz")) | .name'
```

### 3. Update Dockerfile Versions

Update the `ARG IMAGE_VERSION` and `ENV JAVA_VERSION` in each Dockerfile:

- `konajdk/8/Dockerfile`
- `konajdk/11/Dockerfile`
- `konajdk/17/Dockerfile`
- `konajdk/21/Dockerfile`

### 4. Special Considerations for KonaJDK 8

**WARNING**: KonaJDK 8 uses a different naming pattern that includes a build-specific suffix (e.g., `8u462`, `8u472`).

- The suffix **may change** between versions
- Always check the actual asset names for the new version
- Update both the `x86_64` and `aarch64` download URLs

Example pattern for KonaJDK 8:
```dockerfile
# Version 8.0.23 used: 8u462
TencentKona8.0.23.b1_jdk_linux-x86_64_8u462.tar.gz

# Version 8.0.24 uses: 8u472
TencentKona8.0.24.b1_jdk_linux-x86_64_8u472.tar.gz
```

### 5. Local Build Testing

Before pushing changes, always test builds locally:

```bash
# Build all versions in parallel
cd konajdk/8 && docker build --platform linux/amd64 -t konajdk:8.0.24-test . &
cd konajdk/11 && docker build --platform linux/amd64 -t konajdk:11.0.29-test . &
cd konajdk/17 && docker build --platform linux/amd64 -t konajdk:17.0.17-test . &
cd konajdk/21 && docker build --platform linux/amd64 -t konajdk:21.0.9-test . &
wait

# Test all images
docker run --rm konajdk:8.0.24-test java -version
docker run --rm konajdk:11.0.29-test java --version
docker run --rm konajdk:17.0.17-test java --version
docker run --rm konajdk:21.0.9-test java --version
```

### 6. Commit and Push

Create descriptive commits:

```bash
# Commit version updates
git add konajdk/*/Dockerfile
git commit -m "chore(konajdk): 更新所有 KonaJDK 版本到最新 GA 版本

- KonaJDK 8: X.X.X → Y.Y.Y
- KonaJDK 11: X.X.X → Y.Y.Y
- KonaJDK 17: X.X.X → Y.Y.Y
- KonaJDK 21: X.X.X → Y.Y.Y
"

# If KonaJDK 8 download URL changed
git add konajdk/8/Dockerfile
git commit -m "fix(konajdk): 修复 KonaJDK 8.X.X 下载链接

更新 8uXXX 到 8uYYY 以匹配新版本的文件命名
"

git push origin main
```

## Troubleshooting

### Build Fails with 404 Error

**Symptom**: `wget` returns `404 Not Found` when downloading JDK archive.

**Cause**: The asset filename in the Dockerfile doesn't match the actual release asset name.

**Solution**:
1. Check the actual asset names using `gh release view`
2. Update the download URL pattern in the Dockerfile
3. Pay special attention to KonaJDK 8's version-specific suffix

### Example Fix

```dockerfile
# Before (incorrect)
downloadUrl="https://github.com/Tencent/TencentKona-8/releases/download/${JAVA_VERSION}-GA/TencentKona${JAVA_VERSION}.b1_jdk_linux-x86_64_8u462.tar.gz"

# After (correct)
downloadUrl="https://github.com/Tencent/TencentKona-8/releases/download/${JAVA_VERSION}-GA/TencentKona${JAVA_VERSION}.b1_jdk_linux-x86_64_8u472.tar.gz"
```

## Version History

| Date | KonaJDK 8 | KonaJDK 11 | KonaJDK 17 | KonaJDK 21 | Notes |
|------|-----------|------------|------------|------------|-------|
| 2025-11-09 | 8.0.24 (8u472) | 11.0.29 | 17.0.17 | 21.0.9 | Updated all versions to latest GA |
| 2025-07-28 | 8.0.23 (8u462) | 11.0.28 | 17.0.16 | 21.0.8 | Previous stable versions |

## Release Tags

Each Tencent KonaJDK repository uses different tag formats:

- **KonaJDK 8**: `8.0.24-GA`
- **KonaJDK 11**: `kona11.0.29`
- **KonaJDK 17**: `TencentKona-17.0.17`
- **KonaJDK 21**: `TencentKona-21.0.9`

## Useful Commands

```bash
# List all local KonaJDK images
docker images | grep konajdk

# Remove test images
docker rmi $(docker images | grep konajdk | grep test | awk '{print $3}')

# Check GitHub Actions build status
gh run list --workflow konajdk-8.yaml
gh run list --workflow konajdk-11.yaml
gh run list --workflow konajdk-17.yaml
gh run list --workflow konajdk-21.yaml
```

## References

- [Tencent Kona JDK 8](https://github.com/Tencent/TencentKona-8)
- [Tencent Kona JDK 11](https://github.com/Tencent/TencentKona-11)
- [Tencent Kona JDK 17](https://github.com/Tencent/TencentKona-17)
- [Tencent Kona JDK 21](https://github.com/Tencent/TencentKona-21)
