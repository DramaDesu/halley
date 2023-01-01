#pragma once

#include <cassert>
#include <memory>
#include <iterator>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <stdexcept>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace Halley {
	template <typename T, typename Pointer>
	class VectorIterator {
	public:
	#ifdef __cpp_lib_concepts
	    using iterator_concept = std::contiguous_iterator_tag;
	#endif // __cpp_lib_concepts
	    using iterator_category = std::random_access_iterator_tag;
	    using value_type        = T;
	    using difference_type   = std::ptrdiff_t;
	    using pointer           = Pointer;
	    using reference         = decltype(*Pointer());

		VectorIterator() : v(nullptr) {}
		VectorIterator(pointer v) : v(v) {}

		template<typename OtherPointer>
		VectorIterator(const VectorIterator<T, OtherPointer>& o) : v(o.v) {}
		
		reference operator*() const { return *v; }
		pointer operator->() const { return v; }
		reference operator[](size_t n) const { return v[n]; }
		
		VectorIterator& operator++() { ++v; return *this; }
		VectorIterator& operator--() { --v; return *this; }
		VectorIterator operator++(int) const { return VectorIterator(v + 1); }
		VectorIterator operator--(int) const { return VectorIterator(v - 1); }
		VectorIterator operator+(ptrdiff_t o) const { return VectorIterator(v + o); }
		VectorIterator operator-(ptrdiff_t o) const { return VectorIterator(v - o); }
		VectorIterator operator+=(ptrdiff_t o) { v += o; return *this; }
		VectorIterator operator-=(ptrdiff_t o) { v -= o; return *this; }
		
		template<typename OtherPointer>
		ptrdiff_t operator-(const VectorIterator<T, OtherPointer>& other) const { return v - other.v; }

		template<typename OtherPointer>
		bool operator==(const VectorIterator<T, OtherPointer>& other) const { return v == other.v; }

		template<typename OtherPointer>
		bool operator!=(const VectorIterator<T, OtherPointer>& other) const { return v != other.v; }

		template<typename OtherPointer>
		bool operator<(const VectorIterator<T, OtherPointer>& other) const { return v < other.v; }

		template<typename OtherPointer>
		bool operator>(const VectorIterator<T, OtherPointer>& other) const { return v > other.v; }

		template<typename OtherPointer>
		bool operator<=(const VectorIterator<T, OtherPointer>& other) const { return v <= other.v; }

		template<typename OtherPointer>
		bool operator>=(const VectorIterator<T, OtherPointer>& other) const { return v >= other.v; }

		friend void swap(VectorIterator& a, VectorIterator& b) noexcept { std::swap(a.v, b.v); }

		pointer v;
	};

	template <class T, class = void>
	constexpr bool is_iterator_v = false;

	template <class T>
	constexpr bool is_iterator_v<T, std::void_t<typename std::iterator_traits<T>::iterator_category>> = true;

	namespace VectorDetail {
		template <typename T>
		[[nodiscard]] constexpr T alignUp(T val, T align)
		{
			return val + (align - (val % align)) % align;
		}
	}

	template <typename T, class Allocator = std::allocator<T>>
	class VectorSize32 : Allocator {
	public:
		using value_type = T;
		using size_type = uint32_t;
		using difference_type = int32_t;
		using reference = T&;
		using const_reference = const T&;
		using pointer = typename std::allocator_traits<Allocator>::pointer;
		using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;

		using iterator = VectorIterator<T, T*>;
		using const_iterator = VectorIterator<T, const T*>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		constexpr static float growth_factor = 2.0f; // When resizing, what number to scale by

		VectorSize32() noexcept = default;

		explicit VectorSize32(const Allocator& allocator) noexcept
			: Allocator(allocator)
		{
		}

		VectorSize32(size_t count, T defaultValue, const Allocator& alloc = Allocator())
			: Allocator(alloc)
		{
			resize(count, std::move(defaultValue));
		}

		explicit VectorSize32(size_t count, const Allocator& alloc = Allocator())
			: Allocator(alloc)
		{
			resize(count);
		}

		template <class InputIt, std::enable_if_t<is_iterator_v<InputIt>, int> Test = 0>
		VectorSize32(InputIt first, InputIt last, const Allocator& alloc = Allocator())
			: Allocator(alloc)
		{
			assign(std::move(first), std::move(last));
		}
		
		VectorSize32(const VectorSize32& other)
		{
			change_capacity(other.m_capacity);
			m_s = other.m_s;
			for (size_type i = 0; i < st_size(); ++i) {
				std::allocator_traits<Allocator>::construct(*this, data() + i, other[i]);
			}
		}
		
		VectorSize32(const VectorSize32& other, const Allocator& alloc)
			: Allocator(alloc)
		{
			change_capacity(other.m_capacity);
			m_s = other.m_s;
			for (size_type i = 0; i < st_size(); ++i) {
				std::allocator_traits<Allocator>::construct(*this, data() + i, other[i]);
			}
		}
		
		VectorSize32(VectorSize32&& other) noexcept
			: Allocator(std::move(other))
			, m_s(other.m_s)
			, m_capacity(other.m_capacity)
			, m_data(other.m_data)
		{
			other.m_data = nullptr;
			other.m_s = {};
			other.m_capacity = 0;
		}

		VectorSize32(VectorSize32&& other, const Allocator& alloc)
			: Allocator(alloc)
			, m_s(other.m_s)
			, m_capacity(other.m_capacity)
			, m_data(other.m_data)
		{
			other.m_data = nullptr;
			other.m_s = {};
			other.m_capacity = 0;
		}

		VectorSize32(std::initializer_list<T> list, const Allocator& alloc = Allocator())
			: Allocator(alloc)
		{
			reserve(list.size());
			for (const auto& e: list) {
				push_back(T(e));
			}
		}
		
		~VectorSize32() noexcept
		{
			clear();
			std::allocator_traits<Allocator>::deallocate(*this, m_data, m_capacity);
		}

		VectorSize32& operator=(const VectorSize32& other)
		{
			if (this == &other) {
				return *this;
			}

			assign(other.begin(), other.end());
			
			return *this;
		}

		VectorSize32& operator=(VectorSize32&& other) noexcept
		{
			Allocator::operator=(std::move(other));
			m_data = other.m_data;
			m_s = other.m_s;
			m_capacity = other.m_capacity;
			other.m_data = nullptr;
			other.m_s = {};
			other.m_capacity = 0;
			return *this;
		}

		VectorSize32& operator=(std::initializer_list<T> list)
		{
			// TODO: could be faster
			assign(list.begin(), list.end());
			return *this;
		}

		void assign(size_t count, const T& value)
		{
			// TODO: could be faster
			clear();
			resize(count, value);
		}

		template <class InputIt, std::enable_if_t<is_iterator_v<InputIt>, int> Test = 0>
		void assign(InputIt begin, InputIt end)
		{
			// TODO: could be faster
			clear();
			if constexpr (std::is_same_v<typename std::iterator_traits<InputIt>::iterator_category, std::random_access_iterator_tag>) {
				reserve(end - begin);
			}
			for (auto iter = begin; iter != end; ++iter) {
				push_back(*iter);
			}
		}

		void assign(std::initializer_list<T> list)
		{
			assign(list.begin(), list.end());
		}

		[[nodiscard]] Allocator get_allocator() const noexcept
		{
			return *this;
		}

		[[nodiscard]] constexpr pointer data()
		{
			return m_data;
		}

		[[nodiscard]] constexpr const_pointer data() const
		{
			return m_data;
		}

		[[nodiscard]] constexpr size_t size() const
		{
			return st_size();
		}

		[[nodiscard]] constexpr size_t max_size() const
		{
			return std::numeric_limits<size_type>::max();
		}

		[[nodiscard]] constexpr bool empty() const
		{
			return st_size() == 0;
		}

		[[nodiscard]] constexpr size_t capacity() const
		{
			return m_capacity;
		}

		[[nodiscard]] constexpr reference operator[](size_t index)
		{
			return data()[index];
		}

		[[nodiscard]] constexpr const_reference operator[](size_t index) const
		{
			return data()[index];			
		}

		[[nodiscard]] reference at(size_t index)
		{
			if (index >= size()) {
				throw std::out_of_range("Index out of vector range");
			}
			return data()[index];
		}

		[[nodiscard]] const_reference at(size_t index) const
		{
			if (index >= size()) {
				throw std::out_of_range("Index out of vector range");
			}
			return data()[index];			
		}

		[[nodiscard]] reference front()
		{
			assert(!empty());
			return data()[0];
		}

		[[nodiscard]] reference back()
		{
			assert(!empty());
			return data()[size() - 1];
		}

		[[nodiscard]] const_reference front() const
		{
			assert(!empty());
			return data()[0];			
		}

		[[nodiscard]] const_reference back() const
		{
			assert(!empty());
			return data()[size() - 1];			
		}

		void resize(size_t size)
		{
			do_resize(size, [this] (pointer bytes)
			{
				std::allocator_traits<Allocator>::construct(as_allocator(), bytes, T());
			});
		}

		void resize(size_t size, T defaultValue)
		{
			do_resize(size, [&] (pointer bytes)
			{
				std::allocator_traits<Allocator>::construct(as_allocator(), bytes, defaultValue);
			});
		}

		void reserve(size_t size)
		{
			change_capacity(static_cast<size_type>(std::max(size, capacity())));
		}

		void shrink_to_fit()
		{
			change_capacity(st_size());
		}

		void clear() noexcept
		{
			for (size_type i = 0; i < st_size(); ++i) {
				std::allocator_traits<Allocator>::destroy(as_allocator(), data() + i);
			}
			set_size(0);
		}

		iterator insert(const_iterator pos, const T& value)
		{
			return do_insert(pos, [&] (size_t prevSize) {
				push_back(value);
			});
		}

		iterator insert(const_iterator pos, T&& value)
		{
			return do_insert(pos, [&] (size_t prevSize) {
				push_back(std::move(value));
			});
		}

		iterator insert(const_iterator pos, size_t count, const T& value)
		{
			return do_insert(pos, [&](size_t prevSize) {
				reserve(size() + count);
				for (size_t i = 0; i < count; ++i) {
					std::allocator_traits<Allocator>::construct(as_allocator(), data() + (i + prevSize), value);
				}
				set_size(static_cast<size_type>(prevSize + count));
			});
		}

		template <class InputIt, std::enable_if_t<is_iterator_v<InputIt>, int> Test = 0>
		iterator insert(const_iterator pos, InputIt first, InputIt last)
		{
			return do_insert(pos, [&](size_t prevSize) {
				if constexpr (std::is_same_v<typename std::iterator_traits<InputIt>::iterator_category, std::random_access_iterator_tag>) {
					const auto count = last - first;
					reserve(size() + count);
					size_t i = 0;
					for (auto iter = first; iter != last; ++iter) {
						std::allocator_traits<Allocator>::construct(as_allocator(), data() + (i + prevSize), *iter);
						++i;
					}
					set_size(static_cast<size_type>(prevSize + count));
				} else {
					for (auto iter = first; iter != last; ++iter) {
						push_back(*iter);
					}
				}
			});
		}
		
		iterator insert(const_iterator pos, std::initializer_list<T> initializerList)
		{
			return insert(pos, initializerList.begin(), initializerList.end());
		}

		template <class... Args>
		iterator emplace(const_iterator pos, Args&&... args)
		{
			const auto idx = pos - begin();
			emplace_back(std::forward<Args>(args)...);
			std::rotate(begin() + idx, end() - 1, end());
			return begin() + idx;
		}

		iterator erase(const_iterator first, const_iterator last)
		{
			const auto idx = first - begin();
			std::rotate(de_const_iter(first), de_const_iter(last), end());
			resize_down(static_cast<size_type>(size() - (last - first)));
			return begin() + idx;
		}

		iterator erase(const_iterator pos)
		{
			return erase(pos, pos + 1);
		}

		template <class... Args>
		reference emplace_back(Args&&... args)
		{
			const auto idx = size();
			construct_with_ensure_capacity(st_size() + 1, [&] (pointer data)
			{
				std::allocator_traits<Allocator>::construct(as_allocator(), data + idx, std::forward<Args>(args)...);
			});
			set_size(st_size() + 1);
			return elem(idx);
		}

		void push_back(const T& value)
		{
			construct_with_ensure_capacity(st_size() + 1, [&](pointer data)
			{
				std::allocator_traits<Allocator>::construct(as_allocator(), data + size(), value);
			});
			set_size(st_size() + 1);
		}

		void push_back(T&& value)
		{
			construct_with_ensure_capacity(st_size() + 1, [&](pointer data)
			{
				std::allocator_traits<Allocator>::construct(as_allocator(), data + size(), std::move(value));
			});
			set_size(st_size() + 1);
		}

		void pop_back()
		{
			assert(!empty());
			std::allocator_traits<Allocator>::destroy(as_allocator(), &back());
			set_size(st_size() - 1);
		}

		void swap(VectorSize32& other) noexcept
		{
			std::swap(m_data, other.m_data);
			std::swap(m_s, other.m_s);
			std::swap(m_capacity, other.m_capacity);
		}

		[[nodiscard]] iterator begin()
		{
			return iterator(&elem(0));
		}

		[[nodiscard]] iterator end()
		{
			return iterator(&elem(size()));
		}

		[[nodiscard]] const_iterator begin() const
		{
			return const_iterator(&elem(0));
		}

		[[nodiscard]] const_iterator end() const
		{
			return const_iterator(&elem(size()));
		}

		[[nodiscard]] reverse_iterator rbegin()
		{
			return reverse_iterator(end())++;
		}

		[[nodiscard]] reverse_iterator rend()
		{
			return reverse_iterator(begin())++;
		}

		[[nodiscard]] const_reverse_iterator rbegin() const
		{
			return const_reverse_iterator(end())++;
		}

		[[nodiscard]] const_reverse_iterator rend() const
		{
			return const_reverse_iterator(begin())++;
		} 

	private:
		struct SBO {
			bool sbo_enabled : 1;
			size_type m_size : sizeof(size_type) * 8 - 1;

			SBO()
				: sbo_enabled(false)
				, m_size(0)
			{}
		} m_s;
		size_type m_capacity = 0;
		pointer m_data = nullptr;

		void change_capacity(size_type newCapacity)
		{
			change_capacity(newCapacity, [](pointer) {});
		}

		template <typename F>
		void change_capacity(size_type newCapacity, const F& construct)
		{
			assert(newCapacity >= st_size());
			if (newCapacity != m_capacity) {
				pointer newData = newCapacity > 0 ? std::allocator_traits<Allocator>::allocate(as_allocator(), newCapacity) : nullptr;

				construct(newData);
				
				if (m_data) {
					for (size_type i = 0; i < st_size(); ++i) {
						std::allocator_traits<Allocator>::construct(as_allocator(), newData + i, std::move(data()[i]));
						std::allocator_traits<Allocator>::destroy(as_allocator(), data() + i);
					}
					std::allocator_traits<Allocator>::deallocate(as_allocator(), m_data, m_capacity);
				}
				
				m_data = newData;
				m_capacity = newCapacity;
			}
		}

		template<typename F>
		void do_resize(size_t nSize, const F& construct)
		{
			const auto newSize = static_cast<size_type>(nSize);
			if (newSize > st_size()) {
				if (newSize > m_capacity) {
					change_capacity(newSize);
				}
				for (size_type i = st_size(); i < newSize; ++i) {
					construct(data() + i);
				}
				set_size(newSize);
			} else if (newSize < st_size()) {
				resize_down(newSize);
			}
		}

		void resize_down(size_type newSize)
		{
			assert(newSize <= st_size());
			for (size_type i = newSize; i < st_size(); ++i) {
				std::allocator_traits<Allocator>::destroy(as_allocator(), data() + i);
			}
			set_size(newSize);
		}

		void set_size(size_type sz)
		{
			m_s.m_size = sz;
			m_s.sbo_enabled = false;
		}

		template <typename F>
		void construct_with_ensure_capacity(size_type minCapacity, const F& construct)
		{
			if (m_capacity >= minCapacity) {
				construct(data());
			} else {
				change_capacity(std::max(minCapacity, static_cast<size_type>(m_capacity * growth_factor)), construct);
			}
		}

		template <typename F>
		iterator do_insert(const_iterator pos, F f)
		{
			const auto idx = pos - begin();
			const auto prevSize = size();

			f(prevSize);

			std::rotate(begin() + idx, begin() + prevSize, end());
			return begin() + idx;
		}

		[[nodiscard]] reference elem(size_t pos)
		{
			return data()[pos];
		}

		[[nodiscard]] const_reference elem(size_t pos) const
		{
			return data()[pos];
		}

		[[nodiscard]] iterator de_const_iter(const_iterator iter)
		{
			return iterator(begin() + (iter - begin()));
		}

		[[nodiscard]] Allocator& as_allocator() noexcept
		{
			return *this;
		}

		[[nodiscard]] constexpr size_type st_size() const
		{
			assert(!m_s.sbo_enabled);
			return m_s.sbo_enabled ? (m_s.m_size & 0x7F) : m_s.m_size;
		}

		// N.B.: sbo = small buffer optimization
		[[nodiscard]] constexpr static std::size_t get_max_sbo_objects()
		{
			constexpr auto size_bytes = sizeof(VectorSize32);
			constexpr auto sbo_align_enabled = alignof(VectorSize32) >= alignof(T);
			constexpr auto first_sbo_offset = VectorDetail::alignUp<size_t>(1ull, alignof(T));
			const auto result = sbo_align_enabled && first_sbo_offset < size_bytes ? (size_bytes - first_sbo_offset) / sizeof(T) : 0ull;
			static_assert(result <= 127); // More than 127 wouldn't fit in the 7-bit size field
			return result;
		}
	};

	template <typename T, class Allocator>
	bool operator==(const VectorSize32<T, Allocator>& a, const VectorSize32<T, Allocator>& b)
	{
		return std::equal(a.begin(), a.end(), b.begin(), b.end());
	}

	template <typename T, class Allocator>
	bool operator!=(const VectorSize32<T, Allocator>& a, const VectorSize32<T, Allocator>& b)
	{
		return !std::equal(a.begin(), a.end(), b.begin(), b.end());
	}

	template <typename T, class Allocator>
	bool operator<(const VectorSize32<T, Allocator>& a, const VectorSize32<T, Allocator>& b)
	{
		return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](const T& a, const T& b) { return a < b; });
	}

	template <typename T, class Allocator>
	bool operator>(const VectorSize32<T, Allocator>& a, const VectorSize32<T, Allocator>& b)
	{
		return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](const T& a, const T& b) { return a > b; });
	}

	template <typename T, class Allocator>
	bool operator<=(const VectorSize32<T, Allocator>& a, const VectorSize32<T, Allocator>& b)
	{
		return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](const T& a, const T& b) { return a <= b; });
	}

	template <typename T, class Allocator>
	bool operator>=(const VectorSize32<T, Allocator>& a, const VectorSize32<T, Allocator>& b)
	{
		return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](const T& a, const T& b) { return a >= b; });
	}

	template<typename T, class Allocator>
	void swap(VectorSize32<T, Allocator>& a, VectorSize32<T, Allocator>& b) noexcept
	{
		a.swap(b);
	}
}
