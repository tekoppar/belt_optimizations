#pragma once

#include <iterator>
#include <type_traits>
#include <utility>
#include <xmemory>

namespace mem
{
	namespace concepts
	{
		template<class T>
		concept vector_has_method = requires(T t)
		{
			{
				t.needs_resize()
			};
		};

		template<class Object>
		consteval bool nested_is_trivially_copyable_v() noexcept
		{
			if constexpr (mem::concepts::vector_has_method<Object> == true)
			{
				using vector_type = Object::iterator::value_type;
				return mem::concepts::nested_is_trivially_copyable_v<vector_type>();
			}
			else
				return std::is_trivially_copyable_v<Object>;
		};

		template<class Object>
		concept get_is_trivially_copyable_v = (mem::concepts::nested_is_trivially_copyable_v<Object>() == true);

		template<class _Alloc> // tests if allocator has simple addressing
		concept is_simple_alloc_v = std::is_same_v<typename std::allocator_traits<_Alloc>::size_type, std::size_t>&&
			std::is_same_v<typename std::allocator_traits<_Alloc>::difference_type, std::ptrdiff_t>&&
			std::is_same_v<typename std::allocator_traits<_Alloc>::pointer, typename _Alloc::value_type*>&&
			std::is_same_v<typename std::allocator_traits<_Alloc>::const_pointer, const typename _Alloc::value_type*>;

		template<typename Allocator, typename T>
		concept is_allocator_v = std::is_same_v<typename std::allocator_traits<Allocator>::value_type, T>&& requires (Allocator t, std::size_t n)
		{
			{
				std::is_same_v<typename std::allocator_traits<Allocator>::pointer, decltype(t.allocate(n))>
			};
			t.deallocate(t.allocate(n), n);
		};

		template <class _p>
		concept allocator_pointer_deref_member_v = requires(_p p, const _p cp)
		{
			p->m;
			(*p).m;
			cp->m;
			(*cp).m;
		};

		template <typename Allocator, typename Object>
		concept allocator_has_pointers_v = requires(Allocator t, Object * p, const Object * cp)
		{
			std::is_class_v<Object>;
			{
				mem::concepts::allocator_pointer_deref_member_v<Object>
			};
			std::is_same_v<Object&, decltype(*p)>;
			std::is_same_v<const Object&, decltype(*cp)>;
			std::is_same_v<decltype(static_cast<Allocator::pointer>(typename std::allocator_traits<Allocator>::void_pointer(p))), decltype(p)>;
			std::is_same_v<decltype(static_cast<Allocator::const_pointer>(typename std::allocator_traits<Allocator>::const_void_pointer(cp))), decltype(cp)>;
		};

		template<typename Allocator, typename T>
		concept allocator_rebind_v3 =
			std::is_same<typename Allocator::template rebind_alloc<T>, Allocator>::value &&
			!std::is_same<typename Allocator::template rebind_alloc<unsigned long long>, Allocator>::value;

		template<typename Allocator, typename T>
		concept allocator_rebind_v =
			std::is_same<typename Allocator::template rebind_alloc<T>, Allocator>::value;
		//!std::is_same<typename Allocator::template rebind_alloc<unsigned long long>, Allocator>::value;

		template <typename Allocator, typename T>
		concept is_allocator_requirements_v =
			std::is_same_v<typename std::allocator_traits<Allocator>::value_type, T>&&
			mem::concepts::allocator_has_pointers_v<Allocator, T>&&
			mem::concepts::allocator_rebind_v<Allocator, T>&&
			requires(Allocator t, std::size_t n)
		{
			{
				std::is_same_v<typename std::allocator_traits<Allocator>::pointer, decltype(t.allocate(n))>
			};
			t.deallocate(t.allocate(n), n);
			t == Allocator(t);
			t == Allocator(std::move(t));
			t == Allocator(std::move(Allocator(t)));
		};
	}
}