# 包含当前目录下所有的源文件
aux_source_directory(. SRC_LIST)

add_executable(ChatClient ${SRC_LIST})

target_link_libraries(ChatClient pthread)