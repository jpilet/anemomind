#include <server/common/HtmlLog.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(HtmlLogTest, BasicRenderingTest) {
  {
    auto page = HtmlPage::make("/tmp", "index");
    auto html = HtmlTag::make(page, "html");
    {
      auto head = HtmlTag::make(html, "head");
      auto title = HtmlTag::make(head, "title");
      title->stream() << "This is the title!";
    }{
      auto body = HtmlTag::make(html, "body");
      body->stream() << "This is the body!";
    }
  }

  std::ifstream file("/tmp/index.html");
  EXPECT_TRUE(file.good());
  std::string line;
  std::getline(file, line);
  EXPECT_EQ(line, "<html><head><title>This is the title!</title></head><body>This is the body!</body></html>");
}

