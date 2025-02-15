#include <iostream>
#include <unistd.h>

#include "util_memory_pool.h"
//#include "util_memory_pool.hpp"

namespace util
{
	uint32_t memory_pool_c::_get_osBit()
	{
		return sizeof(void*);
	}

	uint32_t memory_pool_c::_get_pageSize()
	{
		static long page_size = sysconf(_SC_PAGESIZE);
		return page_size;
	}

	uint32_t memory_pool_c::get_alloc_cnt_for_test()
	{
		std::lock_guard<std::mutex> pool_lock(_mpool_lock);
		return _mpool.size();
	}

	memory_pool_c::memory_pool_c(const std::string& grp_name, int max_cnt)
		: _grp_name(grp_name), _mpool_max_cnt(max_cnt)
	{
		uint32_t page_size = _get_pageSize();
		_mpool_max_byte = page_size;
		std::cout << "page_size: " << page_size << std::endl;

		// mmap return_value is void*
		void* _base_ptr = mmap(nullptr, page_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		_last_ptr = _base_ptr;

		_mpool_alloc_cnt = 0;
		_mpool_cur_byte = 0;
	}

	memory_pool_c::~memory_pool_c()
	{
		_mpool.clear();
		int result = munmap(_base_ptr, _mpool_max_byte);
		if(-1 == result)
		{
			// check errno
		}
	}
}
