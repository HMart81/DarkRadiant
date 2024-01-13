#pragma once

#include "imodule.h"
#include "iselectiontest.h"

namespace selection
{

enum class PasteMode
{
	// Shader is projected using the texdef matrix in the source
	Projected,

	// If a patch is hit, the algorithm will take the surface 
	// topology of the patch into account, to avoid distortions.
	Natural,
};

/**
 * Public interface of the shader clipboard which is able
 * to pick/copy/paste shaders from and to Texturable objects.
 */
class IShaderClipboard :
    public RegisterableModule
{
public:
    virtual ~IShaderClipboard() {}

	virtual void clear() = 0;

	// Returns the material name of the object in the clipboard (or an empty string)
	virtual std::string getShaderName() = 0;

	enum class SourceType
	{
		Empty,
		Face,
		Patch,
		Shader,
	};

	// Returns the type of element in the shaderclipboard's source slot
	virtual SourceType getSourceType() const = 0;

	// Sets the content of the shaderclipboard to the given material
	virtual void setSourceShader(const std::string& shader) = 0;

	virtual void pickFromSelectionTest(SelectionTest& test) = 0;

	/**
	 * Pastes the shader from the source in the clipboard to the object
	 * located by the given selection test.
	 * 
	 * @pasteToAllFaces: if a brush is hit, the source shader will be pasted
	 * to all its faces, not just the one hit by the selection test.
	 * 
	 * Might throw a cmd::ExecutionFailure.
	 */
	virtual void pasteShader(SelectionTest& test, PasteMode mode, bool pasteToAllFaces) = 0;

	/**
	 * Will attempt to apply the texture coordinates of the source patch to the
	 * target patch located by the given SelectionTest. 
	 * Will throw a cmd::ExecutionFailure if the source or target objects are not matching up.
	 */
	virtual void pasteTextureCoords(SelectionTest& test) = 0;

	/**
	 * Applies the material only to the object hit by the given selection test.
	 * Will leave the rest of the surface properties unchanged, if possible.
	 */
	virtual void pasteMaterialName(SelectionTest& test) = 0;

	/**
	 * Is emitted when the shader source changes.
	 */
	virtual sigc::signal<void()>& signal_sourceChanged() = 0;
};

} // namespace

const char* const MODULE_SHADERCLIPBOARD("ShaderClipboard");

inline selection::IShaderClipboard& GlobalShaderClipboard()
{
	static module::InstanceReference<selection::IShaderClipboard> _reference(MODULE_SHADERCLIPBOARD);
	return _reference;
}
