/**
 * Tests for basic library functions (such as string comparison) which do not
 * require an actual Radiant environment.
 */

#include "gtest/gtest.h"

#include "string/string.h"
#include "string/convert.h"
#include "os/path.h"

namespace test
{

TEST(BasicTest, StringCompareNoCase)
{
    EXPECT_EQ(string::icmp("blah", "blah"), 0);
    EXPECT_EQ(string::icmp("blah", "BLAH"), 0);
    EXPECT_EQ(string::icmp("MiXeD", "mIxED"), 0);

    EXPECT_EQ(string::icmp("a", "b"), -1);
    EXPECT_EQ(string::icmp("b", "a"), 1);
    EXPECT_EQ(string::icmp("baaaaa", "aaaaa"), 1);
}

TEST(BasicTest, StringILessFunctor)
{
    string::ILess less;

    EXPECT_TRUE(!less("blah", "BLAH"));
    EXPECT_TRUE(!less("BLAH", "blah"));

    EXPECT_TRUE(less("blah", "BLEH"));
    EXPECT_TRUE(!less("BLEH", "blah"));
}

TEST(BasicTest, StringIsAlphaNumeric)
{
    EXPECT_FALSE(string::isAlphaNumeric(""));

    EXPECT_TRUE(string::isAlphaNumeric("abc"));
    EXPECT_TRUE(string::isAlphaNumeric("ABC"));
    EXPECT_TRUE(string::isAlphaNumeric("12"));
    EXPECT_TRUE(string::isAlphaNumeric("0"));
    EXPECT_TRUE(string::isAlphaNumeric("abc12"));
    EXPECT_TRUE(string::isAlphaNumeric("abcdefghijklmnopqrstuvwxyz123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"));

    EXPECT_FALSE(string::isAlphaNumeric("abc test"));
    EXPECT_FALSE(string::isAlphaNumeric("abc\ntest"));
    EXPECT_FALSE(string::isAlphaNumeric("abc\ttest"));
    EXPECT_FALSE(string::isAlphaNumeric("abc\rtest"));
    EXPECT_FALSE(string::isAlphaNumeric("$abc"));
    EXPECT_FALSE(string::isAlphaNumeric("/abc"));
    EXPECT_FALSE(string::isAlphaNumeric("test&afff"));
}

TEST(BasicTest, StringConvertToNumeric)
{
    // Float
    EXPECT_EQ(string::convert<float>(std::string("1.2")), 1.2f);
    EXPECT_EQ(string::convert<float>(std::string("-86")), -86.0f);
    EXPECT_EQ(string::convert<float>(std::string(""), -99.0f), -99.0f);
    EXPECT_EQ(string::convert<float>(std::string("abc"), -99.0f), -99.0f);

    // Double
    EXPECT_EQ(string::convert<double>(std::string("3.1425")), 3.1425);
    EXPECT_EQ(string::convert<double>(std::string("569")), 569);
    EXPECT_EQ(string::convert<double>(std::string(""), 123.0), 123.0);
    EXPECT_EQ(string::convert<double>(std::string("JFDJD"), 123.0), 123.0);

    // Int
    EXPECT_EQ(string::convert<int>(std::string("3.1425"), 500), 3 /* parsed truncated */);
    EXPECT_EQ(string::convert<int>(std::string("569")), 569);
    EXPECT_EQ(string::convert<int>(std::string(""), -5), -5);
    EXPECT_EQ(string::convert<int>(std::string("-!-"), 1), 1);

    // Unsigned Int
    EXPECT_EQ(string::convert<unsigned>(std::string("6789"), 500), 6789);
    EXPECT_EQ(
        string::convert<unsigned>(std::string("-1")),
        std::numeric_limits<unsigned>::max() /* wraparound */
    );
    EXPECT_EQ(string::convert<unsigned>(std::string(""), 87), 87);
    EXPECT_EQ(string::convert<unsigned>(std::string("P89P"), 1), 1);

    // Short
    EXPECT_EQ(string::convert<short>(std::string("-56.25"), 123), -56 /* parsed truncated */);
    EXPECT_EQ(string::convert<short>(std::string("1023")), 1023);
    EXPECT_EQ(string::convert<short>(std::string(""), 1234), 1234);
    EXPECT_EQ(string::convert<short>(std::string(":)"), 0), 0);

    // Unsigned Short
    EXPECT_EQ(
        string::convert<unsigned short>(std::string("-1"), 5),
        std::numeric_limits<unsigned short>::max() /* wraparound */
    );
    EXPECT_EQ(string::convert<unsigned short>(std::string("46")), 46);
    EXPECT_EQ(string::convert<unsigned short>(std::string(""), 2), 2);
    EXPECT_EQ(string::convert<unsigned short>(std::string("short"), 10), 10);
}

TEST(PathTests, GetFileExtension)
{
    EXPECT_EQ(os::getExtension(""), "");
    EXPECT_EQ(os::getExtension("file55"), "");
    EXPECT_EQ(os::getExtension("file55."), "");
    EXPECT_EQ(os::getExtension("file.extension"), "extension");
    EXPECT_EQ(os::getExtension("File.TGA"), "TGA");
    EXPECT_EQ(os::getExtension("File.tga"), "tga");
    EXPECT_EQ(os::getExtension("file.tga.bak"), "bak");
    EXPECT_EQ(os::getExtension("relativefolder/file.tga"), "tga");
    EXPECT_EQ(os::getExtension("c:\\absolutepath\\tork.bak"), "bak");
    EXPECT_EQ(os::getExtension("\\absolutepath\\tork.doc"), "doc");
    EXPECT_EQ(os::getExtension("dds/textures/darkmod/test.dds"), "dds");
}

TEST(PathTests, RemoveFileExtension)
{
    EXPECT_EQ(os::removeExtension(""), "");
    EXPECT_EQ(os::removeExtension("file55"), "file55");
    EXPECT_EQ(os::removeExtension("file55."), "file55");
    EXPECT_EQ(os::removeExtension("file.extension"), "file");
    EXPECT_EQ(os::removeExtension("File.tga"), "File");
    EXPECT_EQ(os::removeExtension("file.tga.bak"), "file.tga");
    EXPECT_EQ(os::removeExtension("relativefolder/file.tga"), "relativefolder/file");
    EXPECT_EQ(os::removeExtension("c:\\absolutepath\\tork.bak"), "c:\\absolutepath\\tork");
    EXPECT_EQ(os::removeExtension("\\absolutepath\\tork.doc"), "\\absolutepath\\tork");
    EXPECT_EQ(os::removeExtension("dds/textures/darkmod/test.dds"), "dds/textures/darkmod/test");
}

TEST(PathTests, GetToplevelDirectory)
{
    EXPECT_EQ(os::getToplevelDirectory(""), "");
    EXPECT_EQ(os::getToplevelDirectory("file55"), "");
    EXPECT_EQ(os::getToplevelDirectory("file.tga"), "");
    EXPECT_EQ(os::getToplevelDirectory("dir22/"), "dir22/");
    EXPECT_EQ(os::getToplevelDirectory("relativefolder/file.tga"), "relativefolder/");
    EXPECT_EQ(os::getToplevelDirectory("c:/absolutepath/tork.bak"), "c:/");
    EXPECT_EQ(os::getToplevelDirectory("/absolutepath/tork.doc"), "/");
    EXPECT_EQ(os::getToplevelDirectory("dds/textures/darkmod/test.dds"), "dds/");
}

}