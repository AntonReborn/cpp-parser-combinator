option(ENABLE_ASAN "Enable AddressSanitizer" ON)
option(BUILD_GMOCK "Build GMock" OFF)

if(ENABLE_ASAN)
    set(ASAN_COMPILE_FLAGS -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined)
    set(ASAN_LINK_FLAGS -fsanitize=address)

    add_compile_options(${ASAN_COMPILE_FLAGS})
    add_link_options(${ASAN_LINK_FLAGS})
endif ()

add_compile_options(-Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Werror)