#ifndef MEMORY_POOL_T
#define MEMORY_POOL_T

#include <queue>
#include <atomic>
#include <shared_mutex>
#include <unordered_map>

namespace util
{
	/*
		=== usage ===

		- 
		-
		-
		

		=== TODO ===
	*/
	// �޸� Ǯ : ������ ����� ������ �ش� -> ÷���� ��â ������ �ʿ䵵 ��� ����(�̸� �����صδ°͵� �ϳ��� ����������? �װͺ��� heapalloc �� ���������� ��ũ��.)
	// �׷� ���� �̸� ���� �޸𸮸� ����� �� �ʿ䰡 ����.
	// ���������� �����δ� ������� �����غ��δ�
	// ����� ������������ HeapCreate() �� ���� mmap, mprotect �� ����ؼ� ����ؾ� �Ѵٰ� ��... :(

	/*
		- �� 3�ܰ迡 ���� ����
		- 1. std �ڷᱸ��(vector) �� new, delete �� �̿��Ͽ� �⺻���� �޸� Ǯ ��� ����
		- 2. new,delete ��� HeapCreate(for windows), mmap || mprotect(for linux) �� �̿�
		- 3. lock-free ���� �߰� (1,2 ���� �ټ��� �����忡�� ����� �� ���������� lock ��ü�� ����ؾ� ��)
			- �ֽ� pc �� lock-free �� ������ lock �� ����ϴ� �Ͱ� �������� �񱳸� ����? (��� ����� ���� �� �ʿ�� ������ �ϱ���..)

	*/

	enum class mpool_cls_type_e
	{
		STANDARD_TYPE = 0, // 
		// class_name

	};

	union standard_type_u
	{
		int8_t		_int8_t;
		uint8_t		_uint8_t;
		int16_t		_int16_t;
		uint16_t	_uint16_t;
		int32_t		_int32_t;
		uint32_t	_uint32_t;
		int64_t		_int64_t;
		uint64_t	_uint64_t;
	};

	class base_node_t
	{
	public:
		base_node_t(mpool_cls_type_e cls_type) : _cls_type(cls_type), _ref_cnt(1) {};
		virtual ~base_node_t() = default;

		void retain();
		void release();

	private:
		std::atomic_uint8_t _ref_cnt;
		mpool_cls_type_e _cls_type;
	};

	class memory_pool_t
	{
	public:
		memory_pool_t() = default;
		~memory_pool_t();

	public:
		friend class base_node_t;

		static void init(int max_cnt);
		//static size_t get_mpool_size(mpool_cls_type_e cls_type = static_cast< mpool_cls_type_e>(0));

		template<typename U>
		[[nodiscard]]
		static U* alloc_basic();

		template<typename U>
		static void free_basic(U* node);

		template<typename U>
		[[nodiscard]]
		static U* alloc(mpool_cls_type_e cls_type);

	public:
		template<typename U>
		static void _free(U* node, mpool_cls_type_e cls_type);

		[[nodiscard]]
		static memory_pool_t& _get_inst()
		{
			static memory_pool_t obj;
			return obj;
		}

	private:

		std::queue<standard_type_u*> _mpool_basic;
		std::unordered_map<mpool_cls_type_e, std::queue<base_node_t*>> _mpool_cls;
		std::unordered_map<mpool_cls_type_e, uint16_t> _mpool_cnt;

		int _mpool_max_cnt = 0;
		int _mpool_total_cnt = 0;
	};

	template<typename U>
	void memory_pool_t::free_basic(U* node)
	{
		static_assert(std::is_integral_v<U>
			, "The first parameter of _free() function, must inherit from base_node_t or integral type.");
		
		memory_pool_t& obj = _get_inst();
		obj._mpool_basic.push(reinterpret_cast<standard_type_u*>(node));

		// If U is invalid type, it must be compile time error occurred
		// Then, not need error handling
	}

	template<typename U>
	U* memory_pool_t::alloc_basic()
	{
		/*
		
		*/
		memory_pool_t& obj = _get_inst();

		standard_type_u* basic_node = nullptr;
		if (0 == obj._mpool_basic.size())
			basic_node = new standard_type_u();
		else
		{
			basic_node = obj._mpool_basic.front();
			obj._mpool_basic.pop();
		}

		if constexpr (true == std::is_same<U, int8_t>::value)
			return &basic_node->_int8_t;
		else if constexpr (true == std::is_same<U, uint8_t>::value)
			return &basic_node->_uint8_t;
		else if constexpr (true == std::is_same<U, int16_t>::value)
			return &basic_node->_int16_t;
		else if constexpr (true == std::is_same<U, uint16_t>::value)
			return &basic_node->_uint16_t;
		else if constexpr (true == std::is_same<U, int32_t>::value)
			return &basic_node->_int32_t;
		else if constexpr (true == std::is_same<U, uint32_t>::value)
			return &basic_node->_uint32_t;
		 else if constexpr (true == std::is_same<U, int64_t>::value)
			 return &basic_node->_int64_t;
		 else if constexpr (true == std::is_same<U, uint64_t>::value)
			 return &basic_node->_uint64_t;
		else
			return nullptr;
	}

	template<typename U>
	U* memory_pool_t::alloc(mpool_cls_type_e cls_type)
	{
		/*
		
		*/
		if (cls_type <= mpool_cls_type_e::STANDARD_TYPE)
			return nullptr;

		base_node_t* node = nullptr;
		memory_pool_t& obj = _get_inst();

		// _mpool_max_cnt Ȯ���ؼ� ����ó�� �ʿ���
		//std::atomic_fetch_add(&obj.a, 1);

		auto find_iter = obj._mpool_cls.find(cls_type);
		if (find_iter == obj._mpool_cls.end())
		{
			obj._mpool_cnt.insert(std::make_pair(cls_type, 1)); // �ش� Ÿ���� �� �Ҵ� ������ �����ϱ� ����
			obj._mpool_cls.insert(std::make_pair(cls_type, std::queue<base_node_t*>{}));

			node = new U();
			goto done;
		}

		if (0 == find_iter->second.size())
		{
			auto iter = obj._mpool_cnt.find(cls_type);
			iter->second++;

			node = new U();
			goto done;
		}
		else
		{
			node = find_iter->second.front();
			find_iter->second.pop();

			goto done;
		}

	done:
		return dynamic_cast<U*>(node); // if node is nullptr, dynamic_cast<U*>(node) results also nullptr 
	}

	template<typename U>
	void memory_pool_t::_free(U* node, mpool_cls_type_e cls_type)
	{
		static_assert(std::is_base_of_v<base_node_t, U>, "The first parameter of _free() function, must derive from base_node_t");

		memory_pool_t& obj = _get_inst();

		auto find_iter = obj._mpool_cls.find(cls_type);
		find_iter->second.push(node);
	}
}

#endif