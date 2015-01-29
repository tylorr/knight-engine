#pragma once

#include <collection_types.h>

namespace knight {
namespace string {

void append(foundation::Array<char> &string, const char *c_string);
void append(foundation::Array<char> &string, char c);

size_t length(const foundation::Array<char> &string);

const char *c_str(const foundation::Array<char> &string);

uint64_t hash(const char *string, uint64_t seed = 0);

} // namespace string
} // namespace knight
