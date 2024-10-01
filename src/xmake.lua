
target("client")
    set_kind("binary")
    add_files("client.cpp")
    add_packages("cli11")

target("server")
    set_kind("binary")
    add_files("server.cpp")
    add_packages("cli11")
