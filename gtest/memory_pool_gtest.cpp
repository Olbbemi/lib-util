#ifndef MEMORY_POOL_GTEST_CPP
#define MEMORY_POOL_GTEST_CPP

#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <random>

#include "util_memory_pool.h"
#include "util_memory_pool.hpp"

/* ====================================================================== */
/* ========================== DEFINE & ENUM ============================= */
/* ====================================================================== */
#define USER_GRP_NAME "USER"
#define ROOM_GRP_NAME "ROOM"

/* ====================================================================== */
/* ========================== CLASS & STRUCT ============================ */
/* ====================================================================== */
struct uinfo
{
	int age;
	std::string u_name;
	std::string gender;
	std::string country;
	std::string address;

	uinfo(int age, const std::string& name, const std::string& gender, const std::string& country, const std::string& address)
		: age(age), u_name(name), gender(gender), country(country), address(address) {}
};

struct sinfo
{
	int id;
	std::string s_name;
	std::string u_name;
	std::string role;
	
	sinfo(int id, const std::string& s_name, const std::string& u_name, const std::string& role)
		: id(id), s_name(s_name), u_name(u_name), role(role) {}
};

struct rinfo
{
	int id;
	std::string r_name;
	std::string host;
	
	rinfo(int id, const std::string& r_name, const std::string& host)
		: id(id), r_name(r_name), host(host) {}
};

class user_info : public util::base_node_c
{
	public:
		explicit user_info(const std::string& grp_name) // for dummy
			: util::base_node_c(grp_name), _user_name(""), _gender(""), _country(""), _address("") {}

		user_info(const std::string& grp_name, int age, const std::string& name, const std::string& gender)
			: util::base_node_c(grp_name), _age(age), _user_name(name), _gender(gender) {}

		user_info(const std::string& grp_name, int age, const std::string& name, const std::string& gender, const std::string& country, const std::string& address)
			: util::base_node_c(grp_name), _age(age), _user_name(name), _gender(gender), _country(country), _address(address) {}

		user_info() = delete;

	public:
		int _age;               // mandatory
		std::string _user_name; // mandatory
		std::string _gender;    // mandatory
		std::string _country;
		std::string _address;
};

class session_info : public util::base_node_c
{
	public:
		session_info(const std::string& grp_name, int id, const std::string& s_name, const std::string& u_name, const std::string& role)
			: util::base_node_c(grp_name), _id(id), _sess_name(s_name), _u_name(u_name), _role(role) {}

	public:
		int _id;
		std::string _sess_name;
		std::string _u_name;
		std::string _role;
		std::string _reserved;
};

class room_info : public util::base_node_c
{
	public:
		room_info(const std::string& grp_name, int idx, const std::string& name, const std::string& host)
			: util::base_node_c(grp_name), _id(idx), _r_name(name), _host(host) {}

	private:
		int _id;
		std::string _r_name;
		std::string _host;
};

/* ====================================================================== */
/* ========================== GLOBAL & STATIC =========================== */
/* ====================================================================== */
const uinfo g_uinfo_1{10, "kim", "M", "", ""};
const uinfo g_uinfo_2{20, "lee", "W", "US", ""};
const uinfo g_uinfo_3{30, "jin", "W", "KR", "Busan"};

const sinfo g_sinfo_1{1, "sess_1", g_uinfo_1.u_name, "speaker"};
const sinfo g_sinfo_2{2, "sess_2", g_uinfo_2.u_name, "listener"};
const sinfo g_sinfo_3{3, "sess_3", g_uinfo_3.u_name, "-"};

const rinfo g_rinfo_1(101, "room_1", g_uinfo_1.u_name);
const rinfo g_rinfo_2(102, "room_2", g_uinfo_2.u_name);
const rinfo g_rinfo_3(103, "room_3", g_uinfo_3.u_name);

/* ====================================================================== */
/* =============================== GTEST ================================ */
/* ====================================================================== */
TEST(MemoryPoolTest, AllocWithVariableArg) 
{
	/*
	 * Alloc-test with variable argument.
	 * The return value of func is shared_ptr<struct> and when shared_ptr is released, "_free<struct>" member function is called automatically.
	 * The object created by memory-pool is never nullptr and each member value in object must be the same as value used as variable argument.
	 */
	util::memory_pool_c user_mpool(USER_GRP_NAME);
	{
		// make user_mpool
		std::shared_ptr<user_info> dummy = user_mpool.alloc<user_info>(USER_GRP_NAME);
		std::shared_ptr<user_info> user_1 = user_mpool.alloc<user_info>(USER_GRP_NAME, g_uinfo_1.age, g_uinfo_1.u_name, g_uinfo_1.gender);
		std::shared_ptr<user_info> user_2 = user_mpool.alloc<user_info>(USER_GRP_NAME, g_uinfo_2.age, g_uinfo_2.u_name, g_uinfo_2.gender, g_uinfo_2.country, g_uinfo_2.address);
		std::shared_ptr<user_info> user_3 = user_mpool.alloc<user_info>(USER_GRP_NAME, g_uinfo_3.age, g_uinfo_3.u_name, g_uinfo_3.gender, g_uinfo_3.country, g_uinfo_3.address);

		// check return value
		ASSERT_NE(dummy, nullptr);
		ASSERT_NE(user_1, nullptr);
		ASSERT_NE(user_2, nullptr);
		ASSERT_NE(user_3, nullptr);

		// check field
		EXPECT_EQ(dummy->_age, 0);

		EXPECT_EQ(user_1->_age, g_uinfo_1.age);
		EXPECT_EQ(user_1->_user_name, g_uinfo_1.u_name);
		EXPECT_EQ(user_1->_gender, g_uinfo_1.gender);

		EXPECT_EQ(user_2->_age, g_uinfo_2.age);
		EXPECT_EQ(user_2->_user_name, g_uinfo_2.u_name);
		EXPECT_EQ(user_2->_gender, g_uinfo_2.gender);
		EXPECT_EQ(user_2->_country, g_uinfo_2.country);

		EXPECT_EQ(user_3->_age, g_uinfo_3.age);
		EXPECT_EQ(user_3->_user_name, g_uinfo_3.u_name);
		EXPECT_EQ(user_3->_gender, g_uinfo_3.gender);
		EXPECT_EQ(user_3->_country, g_uinfo_3.country);
		EXPECT_EQ(user_3->_address, g_uinfo_3.address);

		// check alloc_count
		uint32_t user_mpool_alloc_cnt = user_mpool.get_alloc_cnt();	
		EXPECT_EQ(user_mpool_alloc_cnt, 4);
	}

	// after free, check alloc_count
	uint32_t user_mpool_alloc_cnt = user_mpool.get_alloc_cnt();	
	EXPECT_EQ(user_mpool_alloc_cnt, 0);

	uint32_t mpool_size = user_mpool.get_pool_size();
	EXPECT_EQ(mpool_size, 4);
}

TEST(MemoryPoolTest, AllocSameSize) 
{
	/*
	 * After Processing "alloc()" and "_free()" test for particular object[A], Test again with other class of the same size[B].
	 * In a situation about [B], the object created by "alloc()" must be reused memory. 
	 * And it is never nullptr and each member value in object must be the same as value used as variable argument.
	 */
	util::memory_pool_c mpool(USER_GRP_NAME);

	// uinfo alloc & free
	{
		std::shared_ptr<user_info> user_1 = mpool.alloc<user_info>(USER_GRP_NAME, g_uinfo_1.age, g_uinfo_1.u_name, g_uinfo_1.gender, g_uinfo_1.country, g_uinfo_1.address);
		std::shared_ptr<user_info> user_2 = mpool.alloc<user_info>(USER_GRP_NAME, g_uinfo_2.age, g_uinfo_2.u_name, g_uinfo_2.gender, g_uinfo_2.country, g_uinfo_2.address);
		std::shared_ptr<user_info> user_3 = mpool.alloc<user_info>(USER_GRP_NAME, g_uinfo_3.age, g_uinfo_3.u_name, g_uinfo_3.gender, g_uinfo_3.country, g_uinfo_3.address);
	}

	// sinfo alloc & free
	{
		std::shared_ptr<session_info> sess_1 = mpool.alloc<session_info>(USER_GRP_NAME, g_sinfo_1.id, g_sinfo_1.s_name, g_sinfo_1.u_name, g_sinfo_1.role);
		std::shared_ptr<session_info> sess_2 = mpool.alloc<session_info>(USER_GRP_NAME, g_sinfo_2.id, g_sinfo_2.s_name, g_sinfo_2.u_name, g_sinfo_2.role);
		std::shared_ptr<session_info> sess_3 = mpool.alloc<session_info>(USER_GRP_NAME, g_sinfo_3.id, g_sinfo_3.s_name, g_sinfo_3.u_name, g_sinfo_3.role);

		// check return value
		ASSERT_NE(sess_1, nullptr);
		ASSERT_NE(sess_2, nullptr);
		ASSERT_NE(sess_3, nullptr);

		// check field
		EXPECT_EQ(sess_1->_id, g_sinfo_1.id);
		EXPECT_EQ(sess_1->_sess_name, g_sinfo_1.s_name);
		EXPECT_EQ(sess_1->_u_name, g_sinfo_1.u_name);
		EXPECT_EQ(sess_1->_role, g_sinfo_1.role);
	
		EXPECT_EQ(sess_2->_id, g_sinfo_2.id);
		EXPECT_EQ(sess_2->_sess_name, g_sinfo_2.s_name);
		EXPECT_EQ(sess_2->_u_name, g_sinfo_2.u_name);
		EXPECT_EQ(sess_2->_role, g_sinfo_2.role);

		EXPECT_EQ(sess_3->_id, g_sinfo_3.id);
		EXPECT_EQ(sess_3->_sess_name, g_sinfo_3.s_name);
		EXPECT_EQ(sess_3->_u_name, g_sinfo_3.u_name);
		EXPECT_EQ(sess_3->_role, g_sinfo_3.role);

		// check alloc_count
		uint32_t user_mpool_alloc_cnt = mpool.get_alloc_cnt();	
		EXPECT_EQ(user_mpool_alloc_cnt, 3);
	}

	// after free, check alloc_count
	uint32_t user_mpool_alloc_cnt = mpool.get_alloc_cnt();	
	EXPECT_EQ(user_mpool_alloc_cnt, 0);

	uint32_t mpool_size = mpool.get_pool_size();
	EXPECT_EQ(mpool_size, 3);
}

TEST(MemoryPoolTest, MaxAlloc)
{
	/*
	 * In a situation that memory allocation byte is reached the limit, if "alloc()" member function is called, thre return value is nullptr.
	 * In any case, if the maximum byets defined in the memory pool, will crash by page-guard.
	 */
	util::memory_pool_c mpool(USER_GRP_NAME);

	std::vector<std::shared_ptr<user_info>> vec_user_info;
	std::vector<std::shared_ptr<room_info>> vec_room_info;

	uint64_t mpool_max_byte = mpool.get_avail_max_byte();

	uint64_t uinfo_size = sizeof(user_info);
	uint64_t rinfo_size = sizeof(room_info);

	uint64_t bigger = uinfo_size < rinfo_size ? rinfo_size : uinfo_size;
	uint64_t smaller = uinfo_size < rinfo_size ? uinfo_size : rinfo_size;
	uint32_t quotient = mpool_max_byte / bigger;

	// hope that quotient is never zero.
	ASSERT_NE(quotient, 0);

	std::random_device rd;
	std::mt19937 gen{rd()};
	std::uniform_int_distribution<int> dist(1, quotient);
	
	int rand_value = dist(gen);
	for(int i = 0; i < rand_value; i++)
	{
		std::shared_ptr<user_info> user = mpool.alloc<user_info>(USER_GRP_NAME, g_uinfo_1.age, g_uinfo_1.u_name, g_uinfo_1.gender, g_uinfo_1.country, g_uinfo_1.address);
		EXPECT_NE(user, nullptr);
		EXPECT_EQ(mpool.get_adjust_byte(), 0);

		vec_user_info.push_back(user);
	}

	while(mpool.get_cur_byte() + smaller < mpool_max_byte)
	{
		std::shared_ptr<room_info> room = mpool.alloc<room_info>(USER_GRP_NAME, g_rinfo_1.id, g_rinfo_1.r_name, g_rinfo_1.host);
		EXPECT_NE(room, nullptr);
		EXPECT_EQ(mpool.get_adjust_byte(), 0);

		vec_room_info.push_back(room);
	}

	std::shared_ptr<user_info> user = mpool.alloc<user_info>(USER_GRP_NAME, g_uinfo_1.age, g_uinfo_1.u_name, g_uinfo_1.gender, g_uinfo_1.country, g_uinfo_1.address);
	EXPECT_EQ(user, nullptr);

	std::shared_ptr<room_info> room = mpool.alloc<room_info>(USER_GRP_NAME, g_rinfo_1.id, g_rinfo_1.r_name, g_rinfo_1.host);
	EXPECT_EQ(room, nullptr);
	
	vec_user_info.clear();
	vec_room_info.clear();
}

TEST(MemoryPoolTest, DifferentGroupName)
{
	/*
	 * if the group name between memory-pool object and argument used in "alloc()" mismatch, the return value created by "alloc()" will be nullptr. 
	 */
	util::memory_pool_c mpool(USER_GRP_NAME);

	std::shared_ptr<user_info> user = mpool.alloc<user_info>(ROOM_GRP_NAME, g_uinfo_1.age, g_uinfo_1.u_name, g_uinfo_1.gender, g_uinfo_1.country, g_uinfo_1.address);
	std::shared_ptr<session_info> sess = mpool.alloc<session_info>(USER_GRP_NAME, g_sinfo_1.id, g_sinfo_1.s_name, g_sinfo_1.u_name, g_sinfo_1.role);

	EXPECT_EQ(user, nullptr);
	EXPECT_NE(sess, nullptr);	
}

TEST(MemoryPoolTest, MemoryAdjustInAlloc) 
{
	/*
	 * This test is meanless because base_node's alignof is always 8byte.
	 * So every object size are multiple of 8. (Need not to adjust.)
	 */
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif
