#ifndef MEMORY_POOL_HPP
#define MEMORY_POOL_HPP
#include "util_memory_pool.h"
#include <functional>

namespace util 
{
	template <typename U>
	void memory_pool_c::_free(U* obj)
	{
		static_assert(std::is_base_of<base_node_c, U>::value, "U must inherit from base_node_c");

		base_node_c* base_obj = static_cast<base_node_c*>(obj);
		if(0 != base_obj->_grp_name.compare(_grp_name))
		{
			// error
			return;
		}

		// call destructor
		obj->~U();
	
		std::lock_guard<std::mutex> pool_lock(_mpool_lock);

		size_t obj_size = sizeof(U);
		_mpool.insert(std::make_pair(obj_size, base_obj));

		_mpool_alloc_cnt--;
		if (_mpool_alloc_cnt < 0)
		{
			// weird
		}
	}

	template <typename U, typename... Args>
	std::shared_ptr<U> memory_pool_c::alloc(const std::string& grp_name, Args... args)
	{
		// check inheritance
		static_assert(std::is_base_of<base_node_c, U>::value, "U must be derived from base_node_c");

		if(0 != _grp_name.compare(grp_name))
		{
			// error
			return nullptr;
		}

		if(nullptr == _last_ptr)
		{
			// error
			return nullptr;
		}

		base_node_c* base_node = nullptr;
		size_t obj_size = sizeof(U);

		if(_mpool_avail_max_byte < _mpool_cur_byte + obj_size)
		{
			// warn
			return nullptr;
		}

		// alloc node
		{
			std::lock_guard<std::mutex> pool_lock(_mpool_lock);

			auto find_iter = _mpool.find(obj_size);
			if(_mpool.end() == find_iter)
			{
				base_node = reinterpret_cast<base_node_c*>(_last_ptr);
				uint32_t adjust_size = 0;	
				uint32_t os_byte = _get_osBit();
				if(obj_size % os_byte != 0)
				{
					uint32_t quotient = obj_size / os_byte;
					adjust_size = (quotient + 1) * os_byte;
					_mpool_adjust_byte += (adjust_size - obj_size);
				}

				// calc position
				_last_ptr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(_last_ptr) + obj_size + adjust_size);
				_mpool_cur_byte += (obj_size + adjust_size);
			}
			else
			{
				base_node = find_iter->second;
				_mpool.erase(find_iter);
			}

			_mpool_alloc_cnt++;
		}
		
		// call placement new
		base_node = new(base_node) U(grp_name, args...);

		std::shared_ptr<U> wrapped_node(static_cast<U*>(base_node), std::bind(&memory_pool_c::_free<U>, this, std::placeholders::_1));
		return wrapped_node;
	}
}

#endif
