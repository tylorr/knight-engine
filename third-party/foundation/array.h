#pragma once

#include "collection_types.h"
#include "memory.h"

#include <memory>

namespace foundation {
	namespace array
	{
		/// The number of elements in the array.
		template<typename T, uint32_t align> uint32_t size(const Array<T, align> &a) ;
		/// Returns true if there are any elements in the array.
		template<typename T, uint32_t align> bool any(const Array<T, align> &a);
		/// Returns true if the array is empty.
		template<typename T, uint32_t align> bool empty(const Array<T, align> &a);
		
		/// Used to iterate over the array.
		template<typename T, uint32_t align> T* begin(Array<T, align> &a);
		template<typename T, uint32_t align> const T* begin(const Array<T, align> &a);
		template<typename T, uint32_t align> T* end(Array<T, align> &a);
		template<typename T, uint32_t align> const T* end(const Array<T, align> &a);
		
		/// Returns the first/last element of the array. Don't use these on an
		/// empty array.
		template<typename T, uint32_t align> T& front(Array<T, align> &a);
		template<typename T, uint32_t align> const T& front(const Array<T, align> &a);
		template<typename T, uint32_t align> T& back(Array<T, align> &a);
		template<typename T, uint32_t align> const T& back(const Array<T, align> &a);

		/// Changes the size of the array (does not reallocate memory unless necessary).
		template<typename T, uint32_t align> void resize(Array<T, align> &a, uint32_t new_size);
		/// Removes all items in the array (does not free memory).
		template<typename T, uint32_t align> void clear(Array<T, align> &a);
		/// Reallocates the array to the specified capacity.
		template<typename T, uint32_t align> void set_capacity(Array<T, align> &a, uint32_t new_capacity);
		/// Makes sure that the array has at least the specified capacity.
		/// (If not, the array is grown.)
		template<typename T, uint32_t align> void reserve(Array<T, align> &a, uint32_t new_capacity);
		/// Grows the array using a geometric progression formula, so that the ammortized
		/// cost of push_back() is O(1). If a min_capacity is specified, the array will
		/// grow to at least that capacity.
		template<typename T, uint32_t align> void grow(Array<T, align> &a, uint32_t min_capacity = 0);
		/// Trims the array so that its capacity matches its size.
		template<typename T, uint32_t align> void trim(Array<T, align> &a);

		/// Pushes the item to the end of the array.
		template<typename T, uint32_t align> void push_back(Array<T, align> &a, const T &item);
		/// Pops the last item from the array. The array cannot be empty.
		template<typename T, uint32_t align> void pop_back(Array<T, align> &a);

		/// Construct item in place on the end of the array.
		template<typename T, uint32_t align, typename... Args> void emplace_back(Array<T, align> &a, Args&&... args);
	}

	namespace array
	{
		template<typename T, uint32_t align> inline uint32_t size(const Array<T, align> &a) 		{return a._size;}
		template<typename T, uint32_t align> inline bool any(const Array<T, align> &a) 			{return a._size != 0;}
		template<typename T, uint32_t align> inline bool empty(const Array<T, align> &a) 			{return a._size == 0;}
		
		template<typename T, uint32_t align> inline T* begin(Array<T, align> &a) 					{return a._data;}
		template<typename T, uint32_t align> inline const T* begin(const Array<T, align> &a) 		{return a._data;}
		template<typename T, uint32_t align> inline T* end(Array<T, align> &a) 					{return a._data + a._size;}
		template<typename T, uint32_t align> inline const T* end(const Array<T, align> &a) 		{return a._data + a._size;}
		
		template<typename T, uint32_t align> inline T& front(Array<T, align> &a) 					{return a._data[0];}
		template<typename T, uint32_t align> inline const T& front(const Array<T, align> &a) 		{return a._data[0];}
		template<typename T, uint32_t align> inline T& back(Array<T, align> &a) 					{return a._data[a._size-1];}
		template<typename T, uint32_t align> inline const T& back(const Array<T, align> &a) 		{return a._data[a._size-1];}

		template<typename T, uint32_t align> inline void clear(Array<T, align> &a) {resize(a,0);}
		template<typename T, uint32_t align> inline void trim(Array<T, align> &a) {set_capacity(a,a._size);}

		template<typename T, uint32_t align> void resize(Array<T, align> &a, uint32_t new_size)
		{
			if (new_size > a._capacity)
				grow(a, new_size);
			a._size = new_size;
		}

		template<typename T, uint32_t align> inline void reserve(Array<T, align> &a, uint32_t new_capacity)
		{
			if (new_capacity > a._capacity)
				set_capacity(a, new_capacity);
		}

		template<typename T, uint32_t align> void set_capacity(Array<T, align> &a, uint32_t new_capacity)
		{
			if (new_capacity == a._capacity)
				return;
			if (new_capacity < a._size) {
				resize(a, new_capacity);
			}

			T *new_data = 0;
			if (new_capacity > 0) {
				new_data = (T *)a._allocator->allocate(sizeof(T)*new_capacity, align);
				memcpy(new_data, a._data, sizeof(T)*a._size);
			}
			a._allocator->deallocate(a._data);
			a._data = new_data;
			a._capacity = new_capacity;
		}

		template<typename T, uint32_t align> void grow(Array<T, align> &a, uint32_t min_capacity)
		{
			uint32_t new_capacity = a._capacity*2 + 8;
			if (new_capacity < min_capacity)
				new_capacity = min_capacity;
			set_capacity(a, new_capacity);
		}

		template<typename T, uint32_t align> inline void push_back(Array<T, align> &a, const T &item)
		{
			if (a._size + 1 > a._capacity) {
				grow(a);
			}
			a._data[a._size++] = item;
		}

		template<typename T, uint32_t align> inline void pop_back(Array<T, align> &a)
		{
			a._size--;
		}

		template<typename T, uint32_t align, typename... Args>
		void emplace_back(Array<T, align> &a, Args&&... args) {
			if (a._size + 1 > a._capacity)
				grow(a);
			new (a._data + a._size) T(std::forward<Args>(args)...);
			++a._size;
		}
	}

	template<typename T, uint32_t align>
	inline Array<T, align>::Array(Allocator &allocator) : _allocator(&allocator), _size(0), _capacity(0), _data(0) {}

	template<typename T, uint32_t align>
	inline Array<T, align>::~Array()
	{
		_allocator->deallocate(_data);
	}

	template<typename T, uint32_t align>
	Array<T, align>::Array(const Array<T, align> &other) : _allocator(other._allocator), _size(0), _capacity(0), _data(0)
	{
		const uint32_t n = other._size;
		array::set_capacity(*this, n);
		memcpy(_data, other._data, sizeof(T) * n);
		_size = n;
	}

	template<typename T, uint32_t align>
	Array<T, align>::Array(Array<T, align> &&other) : _allocator(nullptr), _size(0), _capacity(0), _data(nullptr)
	{
		*this = std::move(other);
	}

	template<typename T, uint32_t align>
	Array<T, align> &Array<T, align>::operator=(const Array<T, align> &other)
	{
		const uint32_t n = other._size;
		array::resize(*this, n);
		memcpy(_data, other._data, sizeof(T)*n);
		return *this;
	}

	template<typename T, uint32_t align>
	Array<T, align> &Array<T, align>::operator=(Array<T, align> &&other)
	{
		if (this != &other) {
			if (_allocator) {
				_allocator->deallocate(_data);
			}

			_allocator = other._allocator;
			_data = other._data;
			_size = other._size;
			_capacity = other._capacity;

			other._size = 0;
			other._capacity = 0;
			other._data = nullptr;
		}
		return *this;
	}

	template<typename T, uint32_t align>
	inline T & Array<T, align>::operator[](uint32_t i)
	{
		return _data[i];
	}

	template<typename T, uint32_t align>
	inline const T & Array<T, align>::operator[](uint32_t i) const
	{
		return _data[i];
	}
}
