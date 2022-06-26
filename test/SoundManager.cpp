#include "RadiantTest.h"

#include "isound.h"

namespace test
{

using SoundManagerTest = RadiantTest;

TEST_F(SoundManagerTest, ShaderParsing)
{
    // All of these shaders need to be parsed and present
    EXPECT_TRUE(GlobalSoundManager().getSoundShader("parsing_test_case1"));
    EXPECT_TRUE(GlobalSoundManager().getSoundShader("parsing_test_case2"));
    EXPECT_TRUE(GlobalSoundManager().getSoundShader("parsing_test_case3"));
    EXPECT_TRUE(GlobalSoundManager().getSoundShader("parsing_test_case4"));
    EXPECT_TRUE(GlobalSoundManager().getSoundShader("parsing_test_case5"));
    EXPECT_TRUE(GlobalSoundManager().getSoundShader("parsing_test_case6"));
}

TEST_F(SoundManagerTest, GetExistingSoundShader)
{
    // This shader is defined
    auto existing = GlobalSoundManager().getSoundShader("parsing_test_case1");
    EXPECT_TRUE(existing) << "Could not find parsing_test_case1 shader";

    EXPECT_EQ(existing->getDeclName(), "parsing_test_case1");
    EXPECT_EQ(existing->getModName(), "The Dark Mod 2.0 (Standalone)");
    EXPECT_EQ(existing->getDeclType(), decl::Type::SoundShader);
    EXPECT_EQ(existing->getDisplayFolder(), "ambient/environmental/city");
    EXPECT_NEAR(existing->getRadii().getMin(true), 9, 0.01); // in meters
    EXPECT_NEAR(existing->getRadii().getMax(true), 30, 0.01); // in meters
    EXPECT_EQ(existing->getShaderFilePath(), "sound/parsing_test.sndshd");
    auto fileList = existing->getSoundFileList();
    EXPECT_EQ(fileList.size(), 2);
    EXPECT_EQ(fileList.at(0), "sound/nonexistent.ogg");
    EXPECT_EQ(fileList.at(1), "sound/nonexistent2.ogg");

    EXPECT_NE(existing->getDefinition().find("maxDistance 30"), std::string::npos)
        << "Didn't find the expected contents";

    EXPECT_NE(existing->getBlockSyntax().contents.find("maxDistance 30"), std::string::npos)
        << "Didn't find the expected contents";
}

TEST_F(SoundManagerTest, GetNonExistingSoundShader)
{
    // This shader is defined nowhere
    auto nonexisting = GlobalSoundManager().getSoundShader("nonexisting_shader_1242");
    EXPECT_TRUE(nonexisting) << "SoundManager should always return a non-empty reference";
    EXPECT_EQ(nonexisting->getBlockSyntax().fileInfo.visibility, vfs::Visibility::HIDDEN)
        << "Non-existing shader's VFS visibility should be hidden";
    EXPECT_TRUE(nonexisting->getBlockSyntax().contents.empty())
        << "Non-existing shader's content should be empty";
}

}
