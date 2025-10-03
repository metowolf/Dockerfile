#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define CONFIG_PATH "/etc/snell/snell-server.conf"
#define DEFAULT_LISTEN "0.0.0.0:9000"
#define DEFAULT_PSK "uYQwNqZbaIOMiZ6Zni8v5x0M09Y8bSK"

static const char *resolve_env(const char *name, const char *fallback) {
  const char *value = getenv(name);
  if (!value || value[0] == '\0') {
    return fallback;
  }
  return value;
}

int main(int argc, char *argv[]) {
  const char *listen = resolve_env("LISTEN", DEFAULT_LISTEN);
  const char *psk = resolve_env("PSK", DEFAULT_PSK);

  if (mkdir("/etc/snell", 0755) != 0 && errno != EEXIST) {
    perror("entrypoint: mkdir");
    return 1;
  }

  FILE *config = fopen(CONFIG_PATH, "w");
  if (!config) {
    perror("entrypoint: fopen");
    return 1;
  }

  if (fprintf(config,
              "[snell-server]\n"
              "listen = %s\n"
              "psk = %s\n"
              "ipv6 = false\n",
              listen,
              psk) < 0) {
    perror("entrypoint: fprintf");
    fclose(config);
    return 1;
  }

  if (fclose(config) != 0) {
    perror("entrypoint: fclose");
    return 1;
  }

  if (argc < 2) {
    fprintf(stderr, "entrypoint: no command provided\n");
    return 1;
  }

  execvp(argv[1], &argv[1]);
  perror("entrypoint: execvp");
  return 1;
}
