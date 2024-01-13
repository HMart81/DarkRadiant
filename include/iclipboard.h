#pragma once

#include <string>
#include "imodule.h"

namespace radiant
{

/**
 * Interface to DarkRadiant's clipboard which is able to 
 * store and retrieve a string from and to the system clipboard.
 * Access it through the GlobalClipboard() function.
 *
 * This module might not be present in all configurations
 * so its advisable to check for its presence first.
 */
class IClipboard :
    public RegisterableModule
{
public:
    virtual ~IClipboard() {}

    /// Return the contents of the clipboard as a string
    virtual std::string getString() = 0;

    /// Copy the given string to the system clipboard
    virtual void setString(const std::string& str) = 0;

    // A signal that is emitted when the contents of the system clipboard changes
    virtual sigc::signal<void()>& signal_clipboardContentChanged() = 0;
};

}

const char* const MODULE_CLIPBOARD("Clipboard");

inline radiant::IClipboard& GlobalClipboard()
{
    static module::InstanceReference<radiant::IClipboard> _reference(MODULE_CLIPBOARD);
    return _reference;
}
