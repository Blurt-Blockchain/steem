file(GLOB HEADERS "include/blurt/plugins/auth_util/*.hpp")

add_library( steem_auth_util
             ${HEADERS}
             auth_util_plugin.cpp
             auth_util_api.cpp
           )

target_link_libraries( steem_auth_util blurt_app blurt_chain blurt_protocol fc )
target_include_directories( steem_auth_util
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
