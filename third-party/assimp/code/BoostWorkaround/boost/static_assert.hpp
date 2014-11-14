
#ifndef AI_static_assert_INCLUDED
#define AI_static_assert_INCLUDED

#ifndef static_assert

namespace boost {
	namespace detail {

		template <bool b>  class static_assertion_failure;
		template <>        class static_assertion_failure<true> {};
	}
}


#define static_assert(eval) \
{boost::detail::static_assertion_failure<(eval)> assert_dummy;(void)assert_dummy;}

#endif
#endif // !! AI_static_assert_INCLUDED
