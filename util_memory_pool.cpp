#include <iostream>
#include <unistd.h>

#include "util_memory_pool.h"
#include "util_memory_pool.hpp"

namespace util
{
	long memory_pool_c::_get_osBit()
	{
		return sizeof(void*);
	}

	long memory_pool_c::_get_pageSize()
	{
		static long page_size = sysconf(_SC_PAGESIZE);
		return page_size;
	}

	memory_pool_c::memory_pool_c(std::string& grp_name, int max_cnt)
	{
		long page_size = _get_pageSize();
		std::cout << "page_size: " << page_size << std::endl;

		// mmap return_value is void*
		void* _base_ptr = mmap(nullptr, page_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		_last_ptr = _base_ptr;

		_mpool_max_cnt = max_cnt;
		_mpool_max_byte = page_size;
		
		_mpool_alloc_cnt = 0;
		_mpool_cur_byte = 0;

		_grp_name = std::move(grp_name);
	}

	memory_pool_c::~memory_pool_c()
	{
		/*for (int idx = 0; idx < _mpool_cur_cnt; idx++)
			delete _vec_mpool[idx];

		_vec_mpool.clear();*/

		
	}
}
