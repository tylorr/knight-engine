#include "string_stream.h"

#include <stdarg.h>

namespace foundation 
{
	namespace string_stream
	{
		Buffer & tab(Buffer &b, uint32_t column)
		{
			uint32_t current_column = 0;
			uint32_t i = array::size(b) - 1;
			while (i != 0xffffffffu && b[i] != '\n' && b[i] != '\r') {
				++current_column;
				--i;
			}
			if (current_column < column)
				repeat(b, column - current_column, ' ');
			return b;
		}

		Buffer & repeat(Buffer &b, uint32_t count, char c)
		{
			for (uint32_t i=0; i<count; ++i)
				array::push_back(b, c);
			return b;
		}
	}
}
