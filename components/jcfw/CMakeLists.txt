set(JCFW_SRCS
    src/cli.c
    src/trace.c
    src/util/checksum.c
    src/util/string.c
    src/driver/als/ltr303.c)

idf_component_register(
  SRCS
  ${JCFW_SRCS}
  INCLUDE_DIRS
  "include"
  PRIV_REQUIRES
  bt
  nvs_flash)
