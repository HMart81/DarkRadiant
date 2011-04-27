#include "ObjectiveConditionsDialog.h"

#include "imainframe.h"
#include "iuimanager.h"
#include "i18n.h"
#include "itextstream.h"

#include "gtkutil/TextColumn.h"
#include "gtkutil/TreeModel.h"

#include <gtkmm/button.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/treeview.h>
#include <gtkmm/box.h>

#include "ObjectiveEntity.h"

namespace objectives
{

namespace
{
	const char* const DIALOG_TITLE = N_("Edit Objective Conditions");

	const std::string RKEY_ROOT = "user/ui/objectivesEditor/conditionsDialog/";
	const std::string RKEY_WINDOW_STATE = RKEY_ROOT + "window";
}

ObjectiveConditionsDialog::ObjectiveConditionsDialog(const Glib::RefPtr<Gtk::Window>& parent, 
	ObjectiveEntity& objectiveEnt) :
	gtkutil::BlockingTransientWindow(_(DIALOG_TITLE), parent),
    gtkutil::GladeWidgetHolder(
        GlobalUIManager().getGtkBuilderFromFile("ObjectiveConditionsDialog.glade")
    ),
	_objectiveEnt(objectiveEnt),
	_objectiveConditionList(Gtk::ListStore::create(_objConditionColumns)),
	_srcObjState(NULL),
	_type(NULL),
	_value(NULL),
	_objectives(Gtk::ListStore::create(_objectiveColumns)),
	_targetObj(NULL)
{
	// Window properties
    set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
    set_position(Gtk::WIN_POS_CENTER_ON_PARENT);

	// Add vbox to dialog
    add(*getGladeWidget<Gtk::Widget>("mainVbox"));
    g_assert(get_child() != NULL);

	// OK and CANCEL actions
	getGladeWidget<Gtk::Button>("cancelButton")->signal_clicked().connect(
        sigc::mem_fun(*this, &ObjectiveConditionsDialog::_onCancel)
    );
	getGladeWidget<Gtk::Button>("okButton")->signal_clicked().connect(
        sigc::mem_fun(*this, &ObjectiveConditionsDialog::_onOK)
    );

	// Connect the window position tracker
    _windowPosition.loadFromPath(RKEY_WINDOW_STATE);
    _windowPosition.connect(this);
    _windowPosition.applyPosition();

	// Copy the objective conditions to our working set
	_objConditions = _objectiveEnt.getObjectiveConditions();

	setupConditionsPanel();
	setupConditionEditPanel();

	updateSentence();
}

void ObjectiveConditionsDialog::setupConditionsPanel()
{
	// Tree view listing the conditions
    Gtk::TreeView* conditionsList = getGladeWidget<Gtk::TreeView>("conditionsTreeView");
    conditionsList->set_model(_objectiveConditionList);
	conditionsList->set_headers_visible(false);

	conditionsList->get_selection()->signal_changed().connect(
		sigc::mem_fun(*this, &ObjectiveConditionsDialog::_onConditionSelectionChanged)
    );
	
	// Number column
	conditionsList->append_column(*Gtk::manage(new gtkutil::TextColumn("", _objConditionColumns.conditionNumber)));

	// Description
	conditionsList->append_column(*Gtk::manage(new gtkutil::TextColumn("", _objConditionColumns.description)));
	
    // Connect button signals
    Gtk::Button* addButton = getGladeWidget<Gtk::Button>("addObjCondButton");
	addButton->signal_clicked().connect(
        sigc::mem_fun(*this, &ObjectiveConditionsDialog::_onAddObjCondition)
    );

    Gtk::Button* delButton = getGladeWidget<Gtk::Button>("delObjCondButton");
	delButton->set_sensitive(false); // disabled at start
	delButton->signal_clicked().connect(
        sigc::mem_fun(*this, &ObjectiveConditionsDialog::_onDelObjCondition)
    );
}

void ObjectiveConditionsDialog::setupConditionEditPanel()
{
	// Initially everything is insensitive
	getGladeWidget<Gtk::Button>("delObjCondButton")->set_sensitive(false);

	// Disable details controls
    getGladeWidget<Gtk::Widget>("ConditionVBox")->set_sensitive(false);

	// Set ranges for spin buttons
	Gtk::SpinButton* srcMission = getGladeWidget<Gtk::SpinButton>("SourceMission");
	srcMission->set_adjustment(*Gtk::manage(new Gtk::Adjustment(1, 1, 99)));
	srcMission->signal_changed().connect(sigc::mem_fun(*this, &ObjectiveConditionsDialog::_onSrcMissionChanged));

	Gtk::SpinButton* srcObj = getGladeWidget<Gtk::SpinButton>("SourceObjective");
	srcObj->set_adjustment(*Gtk::manage(new Gtk::Adjustment(1, 1, 999)));
	srcObj->signal_changed().connect(sigc::mem_fun(*this, &ObjectiveConditionsDialog::_onSrcObjChanged));

	// Create the state dropdown, Glade is from the last century and doesn't support GtkComboBoxText, hmpf
	Gtk::VBox* placeholder = getGladeWidget<Gtk::VBox>("SourceStatePlaceholder");

	_srcObjState = Gtk::manage(new Gtk::ComboBoxText);

	// Populate the list of states. This must be done in order to match the
	// values in the enum, since the index will be used when writing to entity
	_srcObjState->append_text("INCOMPLETE");
	_srcObjState->append_text("COMPLETE");
	_srcObjState->append_text("FAILED");
	_srcObjState->append_text("INVALID");

	_srcObjState->signal_changed().connect(sigc::mem_fun(*this, &ObjectiveConditionsDialog::_onSrcStateChanged)); 

	placeholder->pack_start(*_srcObjState);

	// Create the objectives dropdown, populate from objective entity
	placeholder = getGladeWidget<Gtk::VBox>("TargetObjectivePlaceholder");

	// Populate the liststore
	_objectiveEnt.populateListStore(_objectives, _objectiveColumns);

	// Set up the dropdown
	_targetObj = Gtk::manage(new Gtk::ComboBox(_objectives));

	Gtk::CellRendererText* indexRenderer = Gtk::manage(new Gtk::CellRendererText);
	Gtk::CellRendererText* nameRenderer = Gtk::manage(new Gtk::CellRendererText);
	
	_targetObj->pack_start(*indexRenderer, false);
	_targetObj->pack_start(*nameRenderer, true);
	_targetObj->add_attribute(indexRenderer->property_text(), _objectiveColumns.objNumber);
	_targetObj->add_attribute(nameRenderer->property_text(), _objectiveColumns.description);
	
	_targetObj->signal_changed().connect(sigc::mem_fun(*this, &ObjectiveConditionsDialog::_onTargetObjChanged));

	placeholder->pack_start(*_targetObj);

	placeholder = getGladeWidget<Gtk::VBox>("TypePlaceholder");

	_type = Gtk::manage(new Gtk::ComboBoxText);

	_type->append_text(_("Change Objective State"));	// 0
	_type->append_text(_("Change Visibility"));			// 1
	_type->append_text(_("Change Mandatory Flag"));		// 2

	_type->signal_changed().connect(sigc::mem_fun(*this, &ObjectiveConditionsDialog::_onTypeChanged));

	placeholder->pack_start(*_type);

	placeholder = getGladeWidget<Gtk::VBox>("ValuePlaceholder");

	_value = Gtk::manage(new Gtk::ComboBoxText);

	// Will be populated later on
	_value->signal_changed().connect(sigc::mem_fun(*this, &ObjectiveConditionsDialog::_onValueChanged));
	
	placeholder->pack_start(*_value);
}

ObjectiveCondition& ObjectiveConditionsDialog::getCurrentObjectiveCondition()
{
	int index = (*_curCondition)[_objConditionColumns.conditionNumber];

	return *_objConditions[index];
}

void ObjectiveConditionsDialog::refreshConditionPanel()
{
	ObjectiveCondition& cond = getCurrentObjectiveCondition();

	// Source mission number
	Gtk::SpinButton* srcMission = getGladeWidget<Gtk::SpinButton>("SourceMission");
	srcMission->set_value(cond.sourceMission + 1); // +1 since user-visible values are 1-based

	// Source objective number
	Gtk::SpinButton* srcObj = getGladeWidget<Gtk::SpinButton>("SourceObjective");
	srcObj->set_value(cond.sourceObjective + 1); // +1 since user-visible values are 1-based

	// Source objective state
	_srcObjState->set_active(cond.sourceState);

	// Load objectives from objective entity into dropdown list
	gtkutil::TreeModel::SelectionFinder finder(cond.targetObjective, _objectiveColumns.objNumber.index());
	_objectives->foreach_iter(sigc::mem_fun(finder, &gtkutil::TreeModel::SelectionFinder::forEach));

	if (finder.getIter())
	{
		_targetObj->set_active(finder.getIter());
	}

	// Set condition type and load possible value types
	switch (cond.type)
	{
	case ObjectiveCondition::CHANGE_STATE:
		_type->set_active(0);
		break;

	case ObjectiveCondition::CHANGE_VISIBILITY:
		_type->set_active(1);
		break;

	case ObjectiveCondition::CHANGE_MANDATORY:
		_type->set_active(2);
		break;

	default:
		globalWarningStream() << "Unknown type encountered while refreshing condition edit panel." << std::endl;
		_type->set_active(0);
		break;
	};

	refreshPossibleValues();
}

void ObjectiveConditionsDialog::refreshPossibleValues()
{
	ObjectiveCondition& cond = getCurrentObjectiveCondition();

	// Remove all items from the dropdown
	_value->clear_items();

	// Set condition type and load possible value types
	switch (cond.type)
	{
	case ObjectiveCondition::CHANGE_STATE:
		_value->append_text(_("Set to INCOMPLETE"));
		_value->append_text(_("Set to COMPLETE"));
		_value->append_text(_("Set to FAILED"));
		_value->append_text(_("Set to INVALID"));

		if (cond.value > 3)
		{
			cond.value = 3;
		}

		_value->set_active(cond.value);
		break;

	case ObjectiveCondition::CHANGE_VISIBILITY:
		_value->append_text(_("Set Invisible"));
		_value->append_text(_("Set Visible"));

		if (cond.value > 1)
		{
			cond.value = 1;
		}

		_value->set_active(cond.value);

		break;

	case ObjectiveCondition::CHANGE_MANDATORY:
		_value->append_text(_("Clear mandatory flag"));
		_value->append_text(_("Set mandatory flag"));

		if (cond.value > 1)
		{
			cond.value = 1;
		}

		_value->set_active(cond.value);

		break;

	default:
		globalWarningStream() << "Unknown type encountered while refreshing condition edit panel." << std::endl;
		break;
	};
}

void ObjectiveConditionsDialog::_onConditionSelectionChanged()
{
	Gtk::Button* delObjCondButton = getGladeWidget<Gtk::Button>("delObjCondButton");
    
	// Get the selection
    Gtk::TreeView* condView = getGladeWidget<Gtk::TreeView>("conditionsTreeView");

	_curCondition = condView->get_selection()->get_selected();

	if (_curCondition) 
    {
		delObjCondButton->set_sensitive(true);

		refreshConditionPanel();

		// Enable details controls
        getGladeWidget<Gtk::Widget>("ConditionVBox")->set_sensitive(true);
	}
	else
    {
		// No selection, disable the delete button 
		delObjCondButton->set_sensitive(false);

		// Disable details controls
        getGladeWidget<Gtk::Widget>("ConditionVBox")->set_sensitive(false);
	}
}

void ObjectiveConditionsDialog::_onAddObjCondition()
{
	for (int i = 1; i < INT_MAX; ++i)
	{
		ObjectiveEntity::ConditionMap::iterator found = _objConditions.find(i);

		if (found == _objConditions.end())
		{
			// Create a new condition
			_objConditions[i] = ObjectiveConditionPtr(new ObjectiveCondition);

			_objConditions[i]->sourceMission = 1;
			_objConditions[i]->sourceObjective = 1;

			// TODO: Select the new condition

			// Refresh the dialog
			populateWidgets();

			return;
		}
	}

	throw std::runtime_error("Ran out of free objective condition indices.");
}

void ObjectiveConditionsDialog::_onDelObjCondition()
{
	assert(_curCondition);

	// Get the index of the current objective condition
	int index = (*_curCondition)[_objConditionColumns.conditionNumber];

	_objConditions.erase(index);

	// Repopulate the dialog
	populateWidgets();
}

void ObjectiveConditionsDialog::_onTypeChanged()
{
	if (!isConditionSelected()) return;

	ObjectiveCondition& cond = getCurrentObjectiveCondition();

	cond.type = static_cast<ObjectiveCondition::Type>(_type->get_active_row_number());

	refreshPossibleValues();

	updateSentence();
}

void ObjectiveConditionsDialog::_onSrcMissionChanged()
{
	if (!isConditionSelected()) return;

	ObjectiveCondition& cond = getCurrentObjectiveCondition();

	// Subtract 1 from the source mission, we need 0-based values
	cond.sourceMission = getGladeWidget<Gtk::SpinButton>("SourceMission")->get_value_as_int() - 1;

	updateSentence();
}

void ObjectiveConditionsDialog::_onSrcObjChanged()
{
	if (!isConditionSelected()) return;

	ObjectiveCondition& cond = getCurrentObjectiveCondition();

	// Subtract 1 from the source objective, we need 0-based values
	cond.sourceObjective = getGladeWidget<Gtk::SpinButton>("SourceObjective")->get_value_as_int() - 1;

	updateSentence();
}

void ObjectiveConditionsDialog::_onSrcStateChanged()
{
	if (!isConditionSelected()) return;

	ObjectiveCondition& cond = getCurrentObjectiveCondition();

	int selectedRow = _srcObjState->get_active_row_number();

	assert(selectedRow >= Objective::INCOMPLETE && selectedRow <= Objective::INVALID);
	cond.sourceState = static_cast<Objective::State>(selectedRow);

	updateSentence();
}

void ObjectiveConditionsDialog::_onTargetObjChanged()
{
	if (!isConditionSelected()) return;

	ObjectiveCondition& cond = getCurrentObjectiveCondition();

	cond.targetObjective = _targetObj->get_active_row_number();

	updateSentence();
}

void ObjectiveConditionsDialog::_onValueChanged()
{
	if (!isConditionSelected()) return;

	ObjectiveCondition& cond = getCurrentObjectiveCondition();

	cond.value = _value->get_active_row_number();

	updateSentence();
}

void ObjectiveConditionsDialog::clear()
{
	// Clear the list
	_objectiveConditionList->clear();
}

void ObjectiveConditionsDialog::_preHide()
{
	// Tell the position tracker to save the information
	_windowPosition.saveToPath(RKEY_WINDOW_STATE);

	// Clear all data before hiding
	clear();
}

void ObjectiveConditionsDialog::populateWidgets()
{
	// Clear internal data first
	clear();

	for (ObjectiveEntity::ConditionMap::const_iterator i = _objConditions.begin();
		 i != _objConditions.end(); ++i)
	{
		Gtk::TreeModel::Row row = *_objectiveConditionList->append();

		row[_objConditionColumns.conditionNumber] = i->first;
		row[_objConditionColumns.description] = getDescription(*i->second);
	}
}

std::string ObjectiveConditionsDialog::getDescription(const ObjectiveCondition& cond)
{
	return "empty description";
}

void ObjectiveConditionsDialog::_preShow()
{
	// Restore the position
	_windowPosition.applyPosition();

	populateWidgets();
}

void ObjectiveConditionsDialog::_onCancel()
{
	destroy();
}

void ObjectiveConditionsDialog::save()
{
	_objectiveEnt.setObjectiveConditions(_objConditions);
}

void ObjectiveConditionsDialog::_onOK()
{
	save();
	destroy();
}

bool ObjectiveConditionsDialog::isConditionSelected()
{
	return getGladeWidget<Gtk::TreeView>("conditionsTreeView")->get_selection()->get_selected();
}

std::string ObjectiveConditionsDialog::getSentence(const ObjectiveCondition& cond)
{
	std::string str = "";

	if (cond.isValid())
	{
		str = "This condition is valid.";
		// If Objective 1 in Mission 3 has the state "failed", perform the following: Activate Mandatory Flag on Objective 3.
	}
	else
	{
		str = _("This condition is not valid or complete yet.");
	}

	return str;
}

void ObjectiveConditionsDialog::updateSentence()
{
	Gtk::Label* label = getGladeWidget<Gtk::Label>("Sentence");

	if (isConditionSelected())
	{
		label->set_markup(getSentence(getCurrentObjectiveCondition()));
	}
	else
	{
		label->set_markup("");
	}
}

} // namespace
