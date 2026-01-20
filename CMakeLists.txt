cmake_minimum_required(VERSION 3.20)
project(ticketverify VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Dependencies: nlohmann/json (header-only) ----
include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)
add_library(ticketverify_core
  src/cli.cpp         # optionnel si tu veux tester cli aussi; sinon retire
  src/engine.cpp      # optionnel pour tests engine; sinon retire
  src/json.cpp        # optionnel
  src/version.cpp     # optionnel
  src/normalize.cpp
  src/parse_total.cpp
  src/parse_merchant.cpp
  src/signals.cpp
)

target_include_directories(ticketverify_core PUBLIC include)
target_link_libraries(ticketverify_core PUBLIC nlohmann_json::nlohmann_json)

add_executable(ticketverify
  src/main.cpp
)

target_link_libraries(ticketverify PRIVATE ticketverify_core)

# ---- Tests ----
include(CTest)
enable_testing()
add_subdirectory(tests)

