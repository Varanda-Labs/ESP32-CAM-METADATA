
idf_component_register(SRCS "dummy.c"
    "metadata.c"
    "./libexif/libexif/exif-tag.c"
    "./libexif/libexif/exif-ifd.c"
    "./libexif/libexif/exif-content.c"
    "./libexif/libexif/exif-entry.c"
    "./libexif/libexif/exif-data.c"
    "./libexif/libexif/exif-mnote-data.c"
    "./libexif/libexif/exif-log.c"
    "./libexif/libexif/exif-utils.c"
    "./libexif/libexif/exif-format.c"
    "./libexif/libexif/exif-byte-order.c"
    "./libexif/libexif/exif-gps-ifd.c"
    "./libexif/libexif/exif-loader.c"
    "./libexif/libexif/exif-mem.c"
    INCLUDE_DIRS ${COMPONENT_DIR} 
        ${COMPONENT_DIR}/libexif 
        ${COMPONENT_DIR}/libexif/libexif 
        ${COMPONENT_DIR}/stub
)

idf_build_set_property(COMPILE_OPTIONS
    "-Wno-error=switch"
    APPEND)

idf_build_set_property(COMPILE_OPTIONS
    "-Wno-unused-variable"
    APPEND)

