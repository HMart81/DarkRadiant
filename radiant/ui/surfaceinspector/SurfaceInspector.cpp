#include "SurfaceInspector.h"

#include <gtk/gtk.h>
#include "ieventmanager.h"
#include "gtkutil/TransientWindow.h"
#include "gtkutil/IconTextButton.h"
#include "gtkutil/LeftAlignedLabel.h"
#include "gtkutil/LeftAlignment.h"
#include "mainframe.h"

namespace ui {

	namespace {
		const std::string WINDOW_TITLE = "Surface Inspector";
		const std::string LABEL_PROPERTIES = "Texture Properties";
		const std::string LABEL_OPERATIONS = "Texture Operations";
		
		const std::string HSHIFT = "horizshift";
		const std::string VSHIFT = "vertshift";
		const std::string HSCALE = "horziscale";
		const std::string VSCALE = "vertscale";
		const std::string ROTATION = "rotation";
	
		const std::string LABEL_HSHIFT = "Horiz. Shift:";
		const std::string LABEL_VSHIFT = "Vert. Shift:";
		const std::string LABEL_HSCALE = "Horiz. Scale:";
		const std::string LABEL_VSCALE = "Vert. Scale:";
		const std::string LABEL_ROTATION = "Rotation:";
		const char* LABEL_SHADER = "Shader:";
		const char* LABEL_STEP = "Step:";
		
		const char* LABEL_FIT_TEXTURE = "Fit Texture:";
		const char* LABEL_FIT = "Fit";
		
		const char* LABEL_FLIP_TEXTURE = "Flip Texture:";
		const char* LABEL_FLIPX = "Flip Horizontal";
		const char* LABEL_FLIPY = "Flip Vertical";
				
		const char* LABEL_APPLY_TEXTURE = "Apply Texture:";
		const char* LABEL_NATURAL = "Natural";
		const char* LABEL_AXIAL = "Axial";
		
		const char* LABEL_DEFAULT_SCALE = "Default Scale:";
		const char* LABEL_TEXTURE_LOCK = "Texture Lock";
		
		const std::string RKEY_ENABLE_TEXTURE_LOCK = "user/ui/brush/textureLock";
		const std::string RKEY_DEFAULT_TEXTURE_SCALE = "user/ui/textures/defaultTextureScale";
	}

SurfaceInspector::SurfaceInspector() :
	_callbackActive(false)
{
	// Be sure to pass FALSE to the TransientWindow to prevent it from self-destruction
	_dialog = gtkutil::TransientWindow(WINDOW_TITLE, MainFrame_getWindow(), false);
	
	// Set the default border width in accordance to the HIG
	gtk_container_set_border_width(GTK_CONTAINER(_dialog), 12);
	
	g_signal_connect(G_OBJECT(_dialog), "delete-event", G_CALLBACK(onDelete), this);
	
	// Create all the widgets and pack them into the window
	populateWindow();
	
	// Connect the defaultTexScale and texLockButton widgets to "their" registry keys
	_connector.connectGtkObject(GTK_OBJECT(_defaultTexScale), RKEY_DEFAULT_TEXTURE_SCALE);
	_connector.connectGtkObject(GTK_OBJECT(_texLockButton), RKEY_ENABLE_TEXTURE_LOCK);
	// Load the values from the Registry
	_connector.importValues();
	
	GlobalRegistry().addKeyObserver(this, RKEY_ENABLE_TEXTURE_LOCK);
	GlobalRegistry().addKeyObserver(this, RKEY_DEFAULT_TEXTURE_SCALE);
	
	GlobalEventManager().connectDialogWindow(GTK_WINDOW(_dialog));
}

SurfaceInspector::~SurfaceInspector() {
	GlobalEventManager().disconnectDialogWindow(GTK_WINDOW(_dialog));
}

void SurfaceInspector::toggle() {
	// Pass the call to the utility methods that save/restore the window position
	if (GTK_WIDGET_VISIBLE(_dialog)) {
		gtkutil::TransientWindow::minimise(_dialog);
		gtk_widget_hide_all(_dialog);
	}
	else {
		gtkutil::TransientWindow::restore(_dialog);
		_connector.importValues();
		gtk_widget_show_all(_dialog);
	}
}

void SurfaceInspector::keyChanged() {
	// Avoid callback loops
	if (_callbackActive) { 
		return;
	}
	
	_callbackActive = true;
	
	// Tell the registryconnector to import the values from the Registry
	_connector.importValues();
	
	_callbackActive = false;
}

void SurfaceInspector::populateWindow() {
	// Create the overall vbox
	GtkWidget* dialogVBox = gtk_vbox_new(false, 6);
	gtk_container_add(GTK_CONTAINER(_dialog), dialogVBox);
	
	// Create the title label (bold font)
	GtkWidget* topLabel = gtkutil::LeftAlignedLabel(
    	std::string("<span weight=\"bold\">") + LABEL_PROPERTIES + "</span>"
    );
    gtk_box_pack_start(GTK_BOX(dialogVBox), topLabel, true, true, 0);
    
    // Setup the table with default spacings
	GtkTable* table = GTK_TABLE(gtk_table_new(6, 2, false));
    gtk_table_set_col_spacings(table, 12);
    gtk_table_set_row_spacings(table, 6);
    
    // Pack it into an alignment so that it is indented
	GtkWidget* alignment = gtkutil::LeftAlignment(GTK_WIDGET(table), 18, 1.0); 
	gtk_box_pack_start(GTK_BOX(dialogVBox), GTK_WIDGET(alignment), true, true, 0);
	
	// Create the entry field and pack it into the first table row
	GtkWidget* shaderLabel = gtkutil::LeftAlignedLabel(LABEL_SHADER);
	gtk_table_attach_defaults(table, shaderLabel, 0, 1, 0, 1);
	
	_shaderEntry = gtk_entry_new();
	//gtk_entry_set_width_chars(GTK_ENTRY(_shaderEntry), 40);
	gtk_table_attach_defaults(table, _shaderEntry, 1, 2, 0, 1);
	
	// Populate the table with the according widgets
	_manipulators[HSHIFT] = createManipulatorRow(LABEL_HSHIFT, table, 1, false);
	_manipulators[VSHIFT] = createManipulatorRow(LABEL_VSHIFT, table, 2, true);
	_manipulators[HSCALE] = createManipulatorRow(LABEL_HSCALE, table, 3, false);
	_manipulators[VSCALE] = createManipulatorRow(LABEL_VSCALE, table, 4, true);
	_manipulators[ROTATION] = createManipulatorRow(LABEL_ROTATION, table, 5, false);
	
	// ======================== Texture Operations ====================================
	
	// Create the texture operations label (bold font)
    GtkWidget* operLabel = gtkutil::LeftAlignedLabel(
    	std::string("<span weight=\"bold\">") + LABEL_OPERATIONS + "</span>"
    );
    gtk_misc_set_padding(GTK_MISC(operLabel), 0, 2); // Small spacing to the top/bottom
    gtk_box_pack_start(GTK_BOX(dialogVBox), operLabel, true, true, 0);
    
    // Setup the table with default spacings
	GtkTable* operTable = GTK_TABLE(gtk_table_new(4, 2, false));
    gtk_table_set_col_spacings(operTable, 12);
    gtk_table_set_row_spacings(operTable, 6);
    
    // Pack this into another alignment
	GtkWidget* operAlignment = gtkutil::LeftAlignment(GTK_WIDGET(operTable), 18, 1.0);
    
    // Pack the table into the dialog
	gtk_box_pack_start(GTK_BOX(dialogVBox), GTK_WIDGET(operAlignment), true, true, 0);
	
	// ------------------------ Fit Texture -----------------------------------
	
	GtkWidget* fitHBox = gtk_hbox_new(false, 6); 
	
	// Create the "Fit Texture" label
	_fitTexture.label = gtkutil::LeftAlignedLabel(LABEL_FIT_TEXTURE);
	gtk_table_attach_defaults(operTable, _fitTexture.label, 0, 1, 0, 1);
	
	_fitTexture.widthAdj = gtk_adjustment_new(1.0f, 0.0f, 1000.0f, 1.0f, 1.0f, 1.0f);
	_fitTexture.heightAdj = gtk_adjustment_new(1.0f, 0.0f, 1000.0f, 1.0f, 1.0f, 1.0f);
	
	// Create the width entry field
	_fitTexture.width = gtk_spin_button_new(GTK_ADJUSTMENT(_fitTexture.widthAdj), 1.0f, 4);
	gtk_widget_set_size_request(_fitTexture.width, 55, -1);
	gtk_box_pack_start(GTK_BOX(fitHBox), _fitTexture.width, false, false, 0);
	
	// Create the "x" label
	GtkWidget* xLabel = gtk_label_new("x");
	gtk_misc_set_alignment(GTK_MISC(xLabel), 0.5f, 0.5f);
	gtk_box_pack_start(GTK_BOX(fitHBox), xLabel, false, false, 0);
	
	// Create the height entry field
	_fitTexture.height = gtk_spin_button_new(GTK_ADJUSTMENT(_fitTexture.heightAdj), 1.0f, 4);
	gtk_widget_set_size_request(_fitTexture.height, 55, -1);
	gtk_box_pack_start(GTK_BOX(fitHBox), _fitTexture.height, false, false, 0);
	
	_fitTexture.button = gtk_button_new_with_label(LABEL_FIT);
	gtk_widget_set_size_request(_fitTexture.button, 30, -1);
	gtk_box_pack_start(GTK_BOX(fitHBox), _fitTexture.button, true, true, 0);
		
	gtk_table_attach_defaults(operTable, fitHBox, 1, 2, 0, 1);
	
	// ------------------------ Operation Buttons ------------------------------
	
	// Create the "Flip Texture" label
	_flipTexture.label = gtkutil::LeftAlignedLabel(LABEL_FLIP_TEXTURE);
	gtk_table_attach_defaults(operTable, _flipTexture.label, 0, 1, 1, 2);
	
	_flipTexture.hbox = gtk_hbox_new(true, 6); 
	_flipTexture.flipX = gtk_button_new_with_label(LABEL_FLIPX);
	_flipTexture.flipY = gtk_button_new_with_label(LABEL_FLIPY);
	gtk_box_pack_start(GTK_BOX(_flipTexture.hbox), _flipTexture.flipX, true, true, 0);
	gtk_box_pack_start(GTK_BOX(_flipTexture.hbox), _flipTexture.flipY, true, true, 0);
	
	gtk_table_attach_defaults(operTable, _flipTexture.hbox, 1, 2, 1, 2);
	
	// Create the "Apply Texture" label
	_applyTex.label = gtkutil::LeftAlignedLabel(LABEL_APPLY_TEXTURE);
	gtk_table_attach_defaults(operTable, _applyTex.label, 0, 1, 2, 3);
	
	_applyTex.hbox = gtk_hbox_new(true, 6); 
	_applyTex.natural = gtk_button_new_with_label(LABEL_NATURAL);
	_applyTex.axial = gtk_button_new_with_label(LABEL_AXIAL);
	gtk_box_pack_start(GTK_BOX(_applyTex.hbox), _applyTex.natural, true, true, 0);
	gtk_box_pack_start(GTK_BOX(_applyTex.hbox), _applyTex.axial, true, true, 0);
	
	gtk_table_attach_defaults(operTable, _applyTex.hbox, 1, 2, 2, 3);
	
	// Default Scale
	GtkWidget* defaultScaleLabel = gtkutil::LeftAlignedLabel(LABEL_DEFAULT_SCALE);
	gtk_table_attach_defaults(operTable, defaultScaleLabel, 0, 1, 3, 4);
	
	GtkWidget* hbox2 = gtk_hbox_new(true, 6);
	 
	// Create the default texture scale spinner
	GtkObject* defaultAdj = gtk_adjustment_new(
		GlobalRegistry().getFloat(RKEY_DEFAULT_TEXTURE_SCALE), 
		0.0f, 1000.0f, 0.1f, 0.1f, 0.1f
	);
	_defaultTexScale = gtk_spin_button_new(GTK_ADJUSTMENT(defaultAdj), 1.0f, 4);
	gtk_widget_set_size_request(_defaultTexScale, 55, -1);
	gtk_box_pack_start(GTK_BOX(hbox2), _defaultTexScale, true, true, 0);
	
	// Texture Lock Toggle
	_texLockButton = gtk_toggle_button_new_with_label(LABEL_TEXTURE_LOCK); 
	gtk_box_pack_start(GTK_BOX(hbox2), _texLockButton, true, true, 0);
	
	gtk_table_attach_defaults(operTable, hbox2, 1, 2, 3, 4);
}

SurfaceInspector::ManipulatorRow SurfaceInspector::createManipulatorRow(
	const std::string& label, GtkTable* table, int row, bool vertical) 
{
	ManipulatorRow manipRow;
	
	manipRow.hbox = gtk_hbox_new(false, 6);
		
	// Create the label
	manipRow.label = gtkutil::LeftAlignedLabel(label);
	gtk_table_attach_defaults(table, manipRow.label, 0, 1, row, row+1);
		
	// Create the entry field
	manipRow.value = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(manipRow.value), 7);
	gtk_box_pack_start(GTK_BOX(manipRow.hbox), manipRow.value, true, true, 0);
	
	if (vertical) {
		GtkWidget* vbox = gtk_vbox_new(true, 0);
		
		manipRow.larger = gtkutil::IconTextButton("", "arrow_up.png", false);
		gtk_widget_set_size_request(manipRow.larger, 30, 12);
		gtk_box_pack_start(GTK_BOX(vbox), manipRow.larger, false, false, 0);
		
		manipRow.smaller = gtkutil::IconTextButton("", "arrow_down.png", false);
		gtk_widget_set_size_request(manipRow.smaller, 30, 12);
		gtk_box_pack_start(GTK_BOX(vbox), manipRow.smaller, false, false, 0);
		
		gtk_box_pack_start(GTK_BOX(manipRow.hbox), vbox, false, false, 0);
	}
	else {
		GtkWidget* hbox = gtk_hbox_new(true, 0);
		
		manipRow.smaller = gtkutil::IconTextButton("", "arrow_left.png", false);
		gtk_widget_set_size_request(manipRow.smaller, 15, 24);
		gtk_box_pack_start(GTK_BOX(hbox), manipRow.smaller, false, false, 0);
		
		manipRow.larger = gtkutil::IconTextButton("", "arrow_right.png", false);
		gtk_widget_set_size_request(manipRow.larger, 15, 24);
		gtk_box_pack_start(GTK_BOX(hbox), manipRow.larger, false, false, 0);
		
		gtk_box_pack_start(GTK_BOX(manipRow.hbox), hbox, false, false, 0);
	}
	
	// Create the label
	manipRow.steplabel = gtkutil::LeftAlignedLabel(LABEL_STEP); 
	gtk_box_pack_start(GTK_BOX(manipRow.hbox), manipRow.steplabel, false, false, 0);
	
	// Create the entry field
	manipRow.step = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(manipRow.step), 5);
	gtk_box_pack_start(GTK_BOX(manipRow.hbox), manipRow.step, false, false, 0);
	
	// Pack the hbox into the table
	gtk_table_attach_defaults(table, manipRow.hbox, 1, 2, row, row+1);
	
	// Return the filled structure
	return manipRow;
}

void SurfaceInspector::toggleInspector() {
	// The static instance
	static SurfaceInspector _inspector;

	// Now toggle the dialog
	_inspector.toggle();
}

gboolean SurfaceInspector::onDelete(GtkWidget* widget, GdkEvent* event, SurfaceInspector* self) {
	// Toggle the visibility of the inspector window
	self->toggle();
	
	// Don't propagate the delete event
	return true;
}

} // namespace ui
