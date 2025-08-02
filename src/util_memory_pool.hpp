#ifndef MEMORY_POOL_HPP
#define MEMORY_POOL_HPP
#include <functional>

#include "util_logger.h"
#include "util_memory_pool.h"

using namespace util;

/* ====================================================================== */
/* ========================== CLASS & STRUCT ============================ */
/* ====================================================================== */
template <typename U>
void memory_pool_c::_free(U* obj)
{
	static_assert(std::is_base_of<base_node_c, U>::value, "U must inherit from base_node_c");

	base_node_c* base_obj = static_cast<base_node_c*>(obj);
	if(0 != base_obj->_grp_name.compare(_grp_name))
	{
		U_LOG_ROTATE_FILE(util::LOG_LEVEL::WARNING, "grp_name is weird. pivot-grp_name:{}/param-grp_name:{}", _grp_name, base_obj->_grp_name);
		return;
	}

	// call destructor
	obj->~U();

	std::lock_guard<std::mutex> pool_lock(_mpool_lock);

	size_t obj_size = sizeof(U);
	_mpool.insert(std::make_pair(obj_size, base_obj));

	_mpool_alloc_cnt--;
	if (_mpool_alloc_cnt < 0) {
		U_LOG_ROTATE_FILE(util::LOG_LEVEL::WARNING, "_mpool_alloc_cnt is negative number. grp_name:{}/alloc_cnt:{}", _grp_name, _mpool_alloc_cnt);
	}
}

template <typename U, typename... Args>
std::shared_ptr<U> memory_pool_c::alloc(const std::string& grp_name, Args... args)
{
	// check inheritance
	static_assert(std::is_base_of<base_node_c, U>::value, "U must be derived from base_node_c");

	if(0 != _grp_name.compare(grp_name))
	{
		U_LOG_ROTATE_FILE(util::LOG_LEVEL::WARNING, "grp_name is weird. pivot-grp_name:{}/param-grp_name:{}", _grp_name, grp_name);
		return nullptr;
	}

	if(nullptr == _base_ptr || false == _check_mprotect)
	{
		U_LOG_ROTATE_FILE(util::LOG_LEVEL::ERROR, "base_ptr is nullptr. OR check_mprotect is false. grp_name:{}/base_ptr:{}/check_mprotect:{}"
				, _grp_name, _base_ptr == nullptr ? "T" : "F", _check_mprotect == true ? "T" : "F");
		return nullptr;
	}

	base_node_c* base_node = nullptr;
	size_t obj_size = sizeof(U);

	if(_mpool_avail_max_byte < _mpool_cur_byte + obj_size)
	{
		U_LOG_ROTATE_FILE(util::LOG_LEVEL::CRITICAL, "can't alloc memory in {}. max_byte:{}/cur_alloc_byte:{}/req_byte:{}", _grp_name, _mpool_avail_max_byte, _mpool_cur_byte, obj_size);
		return nullptr;
	}

	// alloc node
	{
		std::lock_guard<std::mutex> pool_lock(_mpool_lock);

		auto find_iter = _mpool.find(obj_size);
		if(_mpool.end() == find_iter)
		{
			uint32_t adjust_size            = 0;
			base_node                       = reinterpret_cast<base_node_c*>(_last_ptr);
			constexpr std::uint32_t os_byte = _get_osBit();

			if(obj_size % os_byte != 0)
			{
				std::uint32_t quotient = obj_size / os_byte;
				adjust_size            = (quotient + 1) * os_byte;

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

#endif
