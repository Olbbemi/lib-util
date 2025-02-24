#include <sys/mman.h>
#include <unistd.h>

#include "util_memory_pool.h"

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

	uint32_t memory_pool_c::get_pool_size(bool need_lock)
	{
		if(true == need_lock)
		{
			std::lock_guard<std::mutex> pool_lock(_mpool_lock);
			return _mpool.size();
		}

		return _mpool.size();
	}

	memory_pool_c::memory_pool_c(const std::string& grp_name, uint32_t use_page_cnt)
		: _grp_name(grp_name)
	{
		uint32_t page_size = _get_pageSize();
		_mpool_avail_max_byte = page_size * use_page_cnt;
		_mpool_max_byte = page_size * (use_page_cnt + 1);
		
		// mmap return_value is void*
		_base_ptr = mmap(nullptr, _mpool_max_byte, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if(MAP_FAILED == _base_ptr)
		{
			// error
			_base_ptr = nullptr;
		}

		if(-1 == mprotect(reinterpret_cast<char*>(_base_ptr) + (page_size * use_page_cnt), page_size, PROT_NONE)) 
		{
			// error
		}

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
