#include "index_iterator.h"

#include <catch.hpp>

using namespace foundation;

TEST_CASE("Index Iterator") {
  const int length = 10;
  int array[length];

  for (int i = 0; i < length; ++i) {
    array[i] = i;
  }

  auto begin = index_iterator::begin(array);
  auto end = index_iterator::end(array);

  CHECK(begin == begin);
  CHECK(begin != end);

  CHECK(length == end - begin);
  CHECK(end == begin + (end - begin));

  CHECK(!(begin < begin));
  CHECK(!(end < begin));
  CHECK(end > begin);

  CHECK(begin[0] == 0);
  CHECK(begin[2] == 2);

  CHECK(*begin == 0);

  CHECK(*begin++ == 0);
  CHECK(*begin == 1);

  ++begin;
  CHECK(*begin == 2);

  begin += 2;
  CHECK(*begin == 4);

  --end;
  CHECK(*end == length - 1);

  CHECK(*end-- == length - 1);
  CHECK(*end == length - 2);

  end -= 2;
  CHECK(*end == length - 4);
}
