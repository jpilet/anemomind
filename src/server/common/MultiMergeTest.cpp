#include <server/common/MultiMerge.h>

#include <gtest/gtest.h>

using namespace sail;

class IntStream : public SortedStream<int> {
 public:
  IntStream(std::initializer_list<int> list)
    : _container(list), _it(_container.begin()) { }

  virtual int value() const { return *_it; }
  virtual bool next() { ++_it; return end(); }
  virtual bool end() const { return _it == _container.end(); }
 private:
  std::vector<int> _container;
  std::vector<int>::iterator _it;
};

TEST(MultiMergeTest, BasicSort3) {

  IntStream a{1, 9, 12, 14};
  IntStream b{2, 4, 10, 13, 15};
  IntStream c{3, 5, 6, 7, 8, 11};
  MultiMerge<int> merged;

  merged.addStream(&a);
  merged.addStream(&b);
  merged.addStream(&c);

  for (int i = 1; i<= 15; ++i) {
    EXPECT_FALSE(merged.end());
    EXPECT_EQ(i, merged.value());
    if (i == 15) {
      EXPECT_TRUE(merged.next());
    } else {
      EXPECT_FALSE(merged.next());
    }
  }
  EXPECT_TRUE(merged.end());
  EXPECT_TRUE(a.end());
  EXPECT_TRUE(b.end());
  EXPECT_TRUE(c.end());
}

TEST(MultiMergeTest, EmptyMerge) {
  MultiMerge<int> merged;

  EXPECT_TRUE(merged.end());
  IntStream emptyStream{};
  merged.addStream(&emptyStream);
  EXPECT_TRUE(merged.end());
}
