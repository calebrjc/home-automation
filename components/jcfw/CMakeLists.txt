set(JCFW_SRCS
    src/cli.c
    src/util/checksum.c
    src/util/string.c)

idf_component_register(
  SRCS
  ${JCFW_SRCS}
  INCLUDE_DIRS
  "include"
  PRIV_REQUIRES
  bt
  nvs_flash)
