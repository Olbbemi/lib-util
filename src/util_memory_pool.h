#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <mutex>
#include <memory>
#include <string>
#include <unordered_map>

#include <cstdint>
#include <sys/mman.h>

namespace util
{
/* ====================================================================== */
/* ========================== CLASS & STRUCT ============================ */
/* ====================================================================== */

	/* if a particular class use this memory-pool, must inherit this class. */
	class base_node_c
	{
	public:
		friend class memory_pool_c;

		base_node_c(const std::string& grp_name) : _grp_name(grp_name) {};

	public:
		base_node_c() = delete;
		virtual ~base_node_c() = default;

		base_node_c(const base_node_c& rsh) = delete;
		base_node_c& operator=(const base_node_c& rhs) = delete;

		base_node_c(base_node_c&& rsh) = delete;
		base_node_c& operator=(base_node_c&& rhs) = delete;

	private:
		std::string _grp_name;
	};

	/*
	 * memory-pool obj can be created anywhere.
	 * because of using mmap, if there is no more space for virtual memory, some obj are unavailable.
	 * each node is managed by shared_ptr and release by custom-deleter(memory_pool_c::_free).
	 */
	class memory_pool_c
	{
	public:
		template<typename U, typename... Args>
		std::shared_ptr<U> alloc(const std::string& grp_name, Args... args);

		uint32_t get_alloc_cnt() const { return _mpool_alloc_cnt; };
	 	uint32_t get_pool_size(bool need_lock = false);
		
		uint64_t get_adjust_byte() const { return _mpool_adjust_byte; };
		uint64_t get_avail_max_byte() const { return _mpool_avail_max_byte; };
		uint64_t get_cur_byte() const { return _mpool_cur_byte; };

	public:
		memory_pool_c(const std::string& grp_name, uint32_t use_page_cnt = 1);
		~memory_pool_c();

		memory_pool_c() = delete;

		memory_pool_c(const memory_pool_c&) = delete;
		memory_pool_c& operator=(const memory_pool_c& rhs) = delete;

		memory_pool_c(memory_pool_c&&) = delete;
		memory_pool_c& operator=(memory_pool_c&& rhs) = delete;

	private:
		static uint32_t _get_pageSize();
		static uint32_t _get_osBit();
	
	private:
		template<typename U>
		void _free(U* obj);

	private:
		std::string _grp_name;
		std::unordered_multimap<size_t, base_node_c*> _mpool;

		void* _last_ptr;
		void* _base_ptr;

		int32_t _mpool_alloc_cnt = 0;

		uint64_t _mpool_max_byte = 0;
		uint64_t _mpool_avail_max_byte = 0;
		uint64_t _mpool_cur_byte = 0;
		uint64_t _mpool_adjust_byte = 0;

		std::mutex _mpool_lock;
	};
}

#endif
