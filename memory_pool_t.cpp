#include "memory_pool_t.h"

namespace util
{
	void base_node_t::retain()
	{
		std::atomic_fetch_add(&_ref_cnt, 1);
	}

	void base_node_t::release()
	{
		uint8_t result = std::atomic_fetch_sub(&_ref_cnt, 1);
		if (0 == _ref_cnt)
			memory_pool_t::_free(this, _cls_type);	
	}

	/////

	memory_pool_t::~memory_pool_t()
	{
		/*for (int idx = 0; idx < _mpool_cur_cnt; idx++)
			delete _vec_mpool[idx];

		_vec_mpool.clear();*/

		
	}

	void memory_pool_t::init(int max_cnt)
	{
		memory_pool_t& obj = _get_inst();
		obj._mpool_max_cnt = max_cnt;	
	}

	void memory_pool_t::_free(base_node_t* node, mpool_cls_type_e cls_type)
	{
		memory_pool_t& obj = _get_inst();

		auto find_iter = obj._mpool_cls.find(cls_type);
		find_iter->second.push(node);
	}
}