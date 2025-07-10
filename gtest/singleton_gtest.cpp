#ifndef SINGLETON_GTEST_CPP
#define SINGLETON_GTEST_CPP

#include <gtest/gtest.h>
#include <string>

#include "util_singleton.h"

/* ====================================================================== */
/* ========================== DEFINE & ENUM ============================= */
/* ====================================================================== */
const std::string TEST_DATA = "test_data";

/* ====================================================================== */
/* ========================== CLASS & STRUCT ============================ */
/* ====================================================================== */
class test_alpha : public util::dynamic_singletonHolder_c<test_alpha>
{
	public:
		test_alpha() = default;
		~test_alpha() = default;

		void init(const std::string& test_data) { _test_data = test_data; }
		const char* get_data() { return _test_data.c_str(); }

	private:
		std::string _test_data;
};

class test_beta
{
	public:
		test_beta() = default;
		~test_beta() = default;

		void init(const std::string& test_data) { _test_data = test_data; }
		const char* get_data() { return _test_data.c_str(); }

	private:
		std::string _test_data;
};

/* ====================================================================== */
/* =============================== GTEST ================================ */
/* ====================================================================== */
TEST(SingletonTest, Check_Valid_StaticSingleton)
{
	test_beta& test_obj_1 = util::static_singletonHolder_c<test_beta>::get_instance();
	test_beta& test_obj_2 = util::static_singletonHolder_c<test_beta>::get_instance();

	test_obj_1.init(TEST_DATA);

	EXPECT_STREQ(test_obj_1.get_data(), TEST_DATA.c_str());
	EXPECT_STREQ(test_obj_1.get_data(), test_obj_2.get_data());
}

TEST(SingletonTest, Check_Valid_DynamicSingleton)
{
	test_beta* test_obj_1 = util::dynamic_singletonHolder_c<test_beta>::get_instance();
	test_beta* test_obj_2 = util::dynamic_singletonHolder_c<test_beta>::get_instance();

	test_obj_1->init(TEST_DATA);

	EXPECT_STREQ(test_obj_1->get_data(), TEST_DATA.c_str());
	EXPECT_STREQ(test_obj_1->get_data(), test_obj_2->get_data());

	/**/
	EXPECT_TRUE(util::dynamic_singletonHolder_c<test_beta>::release(test_obj_1));
	EXPECT_FALSE(util::dynamic_singletonHolder_c<test_beta>::is_released());

	/**/
	EXPECT_TRUE(util::dynamic_singletonHolder_c<test_beta>::release(test_obj_2));
	EXPECT_TRUE(util::dynamic_singletonHolder_c<test_beta>::is_released());
}

TEST(SingletonTest, IsA_DynamicSingletonClass)
{
	/* IS-A relation test */
	test_alpha* test_obj_1 = test_alpha::get_instance();
	ASSERT_NE(nullptr,  test_obj_1);	

	test_alpha* test_obj_2 = test_alpha::get_instance();
	ASSERT_EQ(test_obj_1, test_obj_2);

	test_obj_1->init(TEST_DATA);
	
	EXPECT_STREQ(test_obj_1->get_data(), TEST_DATA.c_str());
	EXPECT_STREQ(test_obj_1->get_data(), test_obj_2->get_data());

	EXPECT_TRUE(test_alpha::release(test_obj_1));
	EXPECT_TRUE(test_alpha::release(test_obj_2));
	EXPECT_TRUE(test_alpha::is_released());
}

TEST(SingletonTest, HasA_DynamicSingletonClass)
{
	/* Has-A relation test */
	test_beta* test_obj_1 = util::dynamic_singletonHolder_c<test_beta>::get_instance();
	test_beta* test_obj_2 = util::dynamic_singletonHolder_c<test_beta>::get_instance();

	test_obj_1->init(TEST_DATA);

	EXPECT_STREQ(test_obj_1->get_data(), TEST_DATA.c_str());
	EXPECT_STREQ(test_obj_1->get_data(), test_obj_2->get_data());

	EXPECT_TRUE(util::dynamic_singletonHolder_c<test_beta>::release(test_obj_1));
	EXPECT_TRUE(util::dynamic_singletonHolder_c<test_beta>::release(test_obj_2));
	EXPECT_TRUE(util::dynamic_singletonHolder_c<test_beta>::is_released());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif
