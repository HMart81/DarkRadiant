#pragma once

#include "wxutil/dialog/DialogBase.h"
#include <string>
#include "icommandsystem.h"
#include "icolourscheme.h"

#include "wxutil/dataview/TreeModel.h"

class wxButton;
class wxPanel;
class wxDataViewEvent;
class wxColourPickerEvent;
class wxSizer;

namespace wxutil { class TreeView; }

namespace ui
{

/// Dialog for choosing and editing colour schemes
class ColourSchemeEditor :
	public wxutil::DialogBase
{
    // The list of available colour schemes
    wxDataViewListCtrl* _schemeList = nullptr;

	// The vbox containing the colour buttons and its frame
	wxPanel* _colourFrame;

	// The "delete scheme" button
	wxButton* _deleteButton;

public:
	// Constructor
	ColourSchemeEditor();

	// Command target
	static void DisplayDialog(const cmd::ArgumentList& args);

	int ShowModal();

	// Signal for client code to get notified on colour changes
	static sigc::signal<void()>& signal_ColoursChanged();

private:
	// private helper functions
	void populateTree();
	void constructWindow();
    wxBoxSizer* constructListButtons();
    void addOptionsPanel(wxBoxSizer& vbox);
	wxSizer* constructColourSelector(colours::IColourItem& colour, const std::string& name);
	void updateColourSelectors();

	// Queries the user for a string and returns it
	// Returns "" if the user aborts or nothing is entered
	std::string inputDialog(const std::string& title, const std::string& label);

	// Puts the cursor on the currently active scheme
	void selectActiveScheme();

	// Updates the colour selectors after a selection change
	void selectionChanged();

	// Returns the name of the currently selected scheme
	std::string	getSelectedScheme();

	// Deletes or copies a scheme
	void deleteScheme();
	void copyScheme();

	// Deletes a scheme from the list store (called from deleteScheme())
	void deleteSchemeFromList();

	// Callbacks
	void callbackSelChanged(wxDataViewEvent& ev);
	void callbackColorChanged(wxColourPickerEvent& ev, colours::IColourItem& item);

	// Updates the windows after a colour change
	static void updateWindows();
};

} // namespace ui
