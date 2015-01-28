#include "knight_string.h"

#include <array.h>
#include <catch.hpp>

#include <cstring>

TEST_CASE("String") {
  using namespace knight;
  using namespace foundation;

  auto &allocator = memory_globals::default_allocator();
  auto string = Array<char>{allocator};

  SECTION("A string starts as empty") {
    auto c_string = string::c_str(string);

    CHECK(strcmp(c_string, "") == 0);
    CHECK(strlen(c_string) == 0);
  }

  SECTION("Pushing chars onto array creates proper string") {
    array::push_back(string, 'a');
    array::push_back(string, 'b');
    array::push_back(string, '\0');

    auto c_string = string::c_str(string);

    CHECK(strcmp(c_string, "ab") == 0);
    CHECK(strlen(c_string) == 2);
  }

  SECTION("Appending char literal creates proper string") {
    string::append(string, 'e');
    string::append(string, 'f');

    auto c_string = string::c_str(string);

    CHECK(strcmp(c_string, "ef") == 0);
    CHECK(strlen(c_string) == 2);
  }

  SECTION("Appending string literal creates proper string") {
    string::append(string, "abc");

    auto c_string = string::c_str(string);

    CHECK(strcmp(c_string, "abc") == 0);
    CHECK(strlen(c_string) == 3);
  }

  SECTION("Length returns the correct length without \\0") {
    CHECK(string::length(string) == 0);

    string::append(string, "abc");
    CHECK(string::length(string) == 3);

    auto c_string = string::c_str(string);
    CHECK(strlen(c_string) == string::length(string));
  }

  SECTION("Reserving space in array does not break string") {
    array::reserve(string, 5);
    string::append(string, "abcdefghijklmnop");

    auto c_string = string::c_str(string);

    CHECK(strcmp(c_string, "abcdefghijklmnop") == 0);
    CHECK(strlen(c_string) == 16);
  }
}
