set(SOURCES
    main.c
    platform.c
    util.c)

idf_component_register(
    SRCS
    ${SOURCES}
    INCLUDE_DIRS
    ".")
