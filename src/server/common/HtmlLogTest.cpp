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
      {
        auto p = HtmlTag::make(body, "p", {{"width", 5.6}});
        p->stream() << "A paragraph";
      }
    }
  }

  std::ifstream file("/tmp/index.html");
  EXPECT_TRUE(file.good());
  std::string line;
  std::getline(file, line);
  EXPECT_EQ(line, "<html><head><title>This is the title!</title></head><body>This is the body!<p width='5.6'>A paragraph</p></body></html>");
}

TEST(HtmlLogTest, TableTest) {
  {
    auto page = HtmlPage::make("/tmp", "tabletest");
    const char letters[] = "abcdef";
    renderTable(page, 2, 3,
        [](int i, int j) {return i == 0;},
        [&](HtmlNode::Ptr dst, int i, int j) {
          dst->stream() << letters[i*3 + j];
        });
  }
  std::ifstream file("/tmp/tabletest.html");
  EXPECT_TRUE(file.good());
  std::string line;
  std::getline(file, line);
  EXPECT_EQ(line,
      "<table><tr><th>a</th><th>b</th><th>c</th></tr><tr><td>d</td><td>e</td><td>f</td></tr></table>");
}

TEST(HtmlLogTest, TableTest2) {
  {
    auto page = HtmlPage::make("/tmp", "tabletest2");
    auto headerRow = SubTable::header(1, 3,
        [](HtmlNode::Ptr dst, int i, int j) {
      const char c[] = "abc";
      dst->stream() << c[j];
    });
    auto bottomRow = SubTable::cell(1, 3,
        [](HtmlNode::Ptr dst, int i, int j) {
      const char c[] = "def";
      dst->stream() << c[j];
    });

    renderTable(page, vcat(headerRow, bottomRow));
  }
  std::ifstream file("/tmp/tabletest2.html");
  EXPECT_TRUE(file.good());
  std::string line;
  std::getline(file, line);
  EXPECT_EQ(line,
      "<table><tr><th>a</th><th>b</th><th>c</th></tr><tr><td>d</td><td>e</td><td>f</td></tr></table>");

}

