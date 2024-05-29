vcpkg install aricpp

  # this is heuristically generated, and may not be correct
  find_package(aricpp CONFIG REQUIRED)
  target_link_libraries(main PRIVATE aricpp::aricpp)

aricpp provides pkg-config modules:

  # Asterisk ARI interface bindings for modern C++
  aricpp
