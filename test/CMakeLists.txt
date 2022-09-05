# cups-notifier.h
# cups-notifier.c
include (GdbusCodegen)
add_gdbus_codegen_with_namespace (CUPS_NOTIFIER cups-notifier org.cups.cupsd Cups "${CMAKE_SOURCE_DIR}/src/org.cups.cupsd.Notifier.xml")

# mock-cups-notifier
add_executable (mock-cups-notifier mock-cups-notifier.c ${CUPS_NOTIFIER})
target_include_directories (mock-cups-notifier PUBLIC ${SERVICE_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries (mock-cups-notifier ${SERVICE_LIBRARIES})
add_test (mock-cups-notifier mock-cups-notifier)


