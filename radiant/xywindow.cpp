/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
For a list of contributors, see the accompanying CONTRIBUTORS file.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

//
// XY Window
//
// Leonardo Zide (leo@lokigames.com)
//

#include "xywindow.h"

#include "debugging/debugging.h"

#include "iclipper.h"
#include "ientity.h"
#include "ieclass.h"
#include "igl.h"
#include "brush/TexDef.h"
#include "ibrush.h"
#include "iundo.h"
#include "iregistry.h"

#include <gtk/gtklabel.h>
#include <gtk/gtkmenuitem.h>

#include "generic/callback.h"
#include "string/string.h"
#include "stream/stringstream.h"

#include "scenelib.h"
#include "renderer.h"
#include "moduleobserver.h"

#include "gtkutil/menu.h"
#include "gtkutil/container.h"
#include "gtkutil/widget.h"
#include "gtkutil/glwidget.h"
#include "gtkmisc.h"
#include "select.h"
#include "csg.h"
#include "brushmanip.h"
#include "entity.h"
#include "camera/GlobalCamera.h"
#include "texwindow.h"
#include "mainframe.h"
#include "preferences.h"
#include "commands.h"
#include "grid.h"
#include "windowobservers.h"
#include "plugin.h"
#include "ui/colourscheme/ColourScheme.h"
#include "ui/eventmapper/EventMapper.h"

#include "selection/SelectionBox.h"

#include "xyview/XYWnd.h"
#include "xyview/GlobalXYWnd.h"

// =============================================================================
// variables

bool g_bCrossHairs = false;

/* This function determines the point currently being "looked" at, it is used for toggling the ortho views
 * If something is selected the center of the selection is taken as new origin, otherwise the camera
 * position is considered to be the new origin of the toggled orthoview.
*/
Vector3 getFocusPosition() {
	Vector3 position(0,0,0);
	
	if (GlobalSelectionSystem().countSelected() != 0) {
		Select_GetMid(position);
	}
	else {
		position = g_pParentWnd->GetCamWnd()->getCameraOrigin();
	}
	
	return position;
}

void XY_Split_Focus() {
	// Re-position all available views
	GlobalXYWnd().positionAllViews(getFocusPosition());
}

void XY_Focus() {
	GlobalXYWnd().positionView(getFocusPosition());
}

void XY_Top() {
	GlobalXYWnd().setActiveViewType(XY);
	GlobalXYWnd().positionView(getFocusPosition());
}

void XY_Side() {
	GlobalXYWnd().setActiveViewType(XZ);
	GlobalXYWnd().positionView(getFocusPosition());
}

void XY_Front() {
	GlobalXYWnd().setActiveViewType(YZ);
	GlobalXYWnd().positionView(getFocusPosition());
}

void toggleActiveOrthoView() {
	GlobalXYWnd().toggleActiveView();
	GlobalXYWnd().positionView(getFocusPosition());
}

void XY_Zoom100() {
	GlobalXYWnd().setScale(1);
}

void XY_ZoomIn() {
	XYWnd* xywnd = GlobalXYWnd().getActiveXY();

	if (xywnd != NULL) {
		xywnd->zoomIn();
	}
}

void XY_ZoomOut() {
	XYWnd* xywnd = GlobalXYWnd().getActiveXY();

	if (xywnd != NULL) {
		xywnd->zoomOut();
	}
}

void ToggleShowCrosshair()
{
  g_bCrossHairs ^= 1; 
  GlobalXYWnd().updateAllViews();
}

void ToggleShowGrid()
{
  g_xywindow_globals_private.d_showgrid = !g_xywindow_globals_private.d_showgrid;
  GlobalXYWnd().updateAllViews();
}

ToggleShown g_xy_top_shown(true);

void XY_Top_Shown_Construct(GtkWindow* parent)
{
  g_xy_top_shown.connect(GTK_WIDGET(parent));
}

ToggleShown g_yz_side_shown(false);

void YZ_Side_Shown_Construct(GtkWindow* parent)
{
  g_yz_side_shown.connect(GTK_WIDGET(parent));
}

ToggleShown g_xz_front_shown(false);

void XZ_Front_Shown_Construct(GtkWindow* parent)
{
  g_xz_front_shown.connect(GTK_WIDGET(parent));
}

void ShowNamesToggle()
{
  GlobalEntityCreator().setShowNames(!GlobalEntityCreator().getShowNames());
  GlobalXYWnd().updateAllViews();
}
typedef FreeCaller<ShowNamesToggle> ShowNamesToggleCaller;
void ShowNamesExport(const BoolImportCallback& importer)
{
  importer(GlobalEntityCreator().getShowNames());
}
typedef FreeCaller1<const BoolImportCallback&, ShowNamesExport> ShowNamesExportCaller;

void ShowAnglesToggle()
{
  GlobalEntityCreator().setShowAngles(!GlobalEntityCreator().getShowAngles());
  GlobalXYWnd().updateAllViews();
}
typedef FreeCaller<ShowAnglesToggle> ShowAnglesToggleCaller;
void ShowAnglesExport(const BoolImportCallback& importer)
{
  importer(GlobalEntityCreator().getShowAngles());
}
typedef FreeCaller1<const BoolImportCallback&, ShowAnglesExport> ShowAnglesExportCaller;

void ShowBlocksToggle()
{
  g_xywindow_globals_private.show_blocks ^= 1;
  GlobalXYWnd().updateAllViews();
}
typedef FreeCaller<ShowBlocksToggle> ShowBlocksToggleCaller;
void ShowBlocksExport(const BoolImportCallback& importer)
{
  importer(g_xywindow_globals_private.show_blocks);
}
typedef FreeCaller1<const BoolImportCallback&, ShowBlocksExport> ShowBlocksExportCaller;

void ShowCoordinatesToggle()
{
  g_xywindow_globals_private.show_coordinates ^= 1;
  GlobalXYWnd().updateAllViews();
}
typedef FreeCaller<ShowCoordinatesToggle> ShowCoordinatesToggleCaller;
void ShowCoordinatesExport(const BoolImportCallback& importer)
{
  importer(g_xywindow_globals_private.show_coordinates);
}
typedef FreeCaller1<const BoolImportCallback&, ShowCoordinatesExport> ShowCoordinatesExportCaller;

void ShowOutlineToggle()
{
  g_xywindow_globals_private.show_outline ^= 1;
  GlobalXYWnd().updateAllViews();
}
typedef FreeCaller<ShowOutlineToggle> ShowOutlineToggleCaller;
void ShowOutlineExport(const BoolImportCallback& importer)
{
  importer(g_xywindow_globals_private.show_outline);
}
typedef FreeCaller1<const BoolImportCallback&, ShowOutlineExport> ShowOutlineExportCaller;

void ShowAxesToggle()
{
  g_xywindow_globals_private.show_axis ^= 1;
  GlobalXYWnd().updateAllViews();
}
typedef FreeCaller<ShowAxesToggle> ShowAxesToggleCaller;
void ShowAxesExport(const BoolImportCallback& importer)
{
  importer(g_xywindow_globals_private.show_axis);
}
typedef FreeCaller1<const BoolImportCallback&, ShowAxesExport> ShowAxesExportCaller;

void ShowWorkzoneToggle()
{
  g_xywindow_globals_private.d_show_work ^= 1;
  GlobalXYWnd().updateAllViews();
}
typedef FreeCaller<ShowWorkzoneToggle> ShowWorkzoneToggleCaller;
void ShowWorkzoneExport(const BoolImportCallback& importer)
{
  importer(g_xywindow_globals_private.d_show_work);
}
typedef FreeCaller1<const BoolImportCallback&, ShowWorkzoneExport> ShowWorkzoneExportCaller;

ShowNamesExportCaller g_show_names_caller;
BoolExportCallback g_show_names_callback(g_show_names_caller);
ToggleItem g_show_names(g_show_names_callback);

ShowAnglesExportCaller g_show_angles_caller;
BoolExportCallback g_show_angles_callback(g_show_angles_caller);
ToggleItem g_show_angles(g_show_angles_callback);

ShowBlocksExportCaller g_show_blocks_caller;
BoolExportCallback g_show_blocks_callback(g_show_blocks_caller);
ToggleItem g_show_blocks(g_show_blocks_callback);

ShowCoordinatesExportCaller g_show_coordinates_caller;
BoolExportCallback g_show_coordinates_callback(g_show_coordinates_caller);
ToggleItem g_show_coordinates(g_show_coordinates_callback);

ShowOutlineExportCaller g_show_outline_caller;
BoolExportCallback g_show_outline_callback(g_show_outline_caller);
ToggleItem g_show_outline(g_show_outline_callback);

ShowAxesExportCaller g_show_axes_caller;
BoolExportCallback g_show_axes_callback(g_show_axes_caller);
ToggleItem g_show_axes(g_show_axes_callback);

ShowWorkzoneExportCaller g_show_workzone_caller;
BoolExportCallback g_show_workzone_callback(g_show_workzone_caller);
ToggleItem g_show_workzone(g_show_workzone_callback);

void XYShow_registerCommands()
{
  GlobalToggles_insert("ShowAngles", ShowAnglesToggleCaller(), ToggleItem::AddCallbackCaller(g_show_angles));
  GlobalToggles_insert("ShowNames", ShowNamesToggleCaller(), ToggleItem::AddCallbackCaller(g_show_names));
  GlobalToggles_insert("ShowBlocks", ShowBlocksToggleCaller(), ToggleItem::AddCallbackCaller(g_show_blocks));
  GlobalToggles_insert("ShowCoordinates", ShowCoordinatesToggleCaller(), ToggleItem::AddCallbackCaller(g_show_coordinates));
  GlobalToggles_insert("ShowWindowOutline", ShowOutlineToggleCaller(), ToggleItem::AddCallbackCaller(g_show_outline));
  GlobalToggles_insert("ShowAxes", ShowAxesToggleCaller(), ToggleItem::AddCallbackCaller(g_show_axes));
  GlobalToggles_insert("ShowWorkzone", ShowWorkzoneToggleCaller(), ToggleItem::AddCallbackCaller(g_show_workzone));
}

void XYWnd_registerShortcuts()
{
  command_connect_accelerator("ToggleCrosshairs");
}

#include "preferencesystem.h"
#include "stringio.h"

void ToggleShown_importBool(ToggleShown& self, bool value)
{
  self.set(value);
}
typedef ReferenceCaller1<ToggleShown, bool, ToggleShown_importBool> ToggleShownImportBoolCaller;
void ToggleShown_exportBool(const ToggleShown& self, const BoolImportCallback& importer)
{
  importer(self.active());
}
typedef ConstReferenceCaller1<ToggleShown, const BoolImportCallback&, ToggleShown_exportBool> ToggleShownExportBoolCaller;


void XYWindow_Construct()
{
  GlobalCommands_insert("ToggleCrosshairs", FreeCaller<ToggleShowCrosshair>(), Accelerator('X', (GdkModifierType)GDK_SHIFT_MASK));
  GlobalCommands_insert("ToggleGrid", FreeCaller<ToggleShowGrid>(), Accelerator('0'));

  GlobalToggles_insert("ToggleView", ToggleShown::ToggleCaller(g_xy_top_shown), ToggleItem::AddCallbackCaller(g_xy_top_shown.m_item), Accelerator('V', (GdkModifierType)(GDK_SHIFT_MASK|GDK_CONTROL_MASK)));
  GlobalToggles_insert("ToggleSideView", ToggleShown::ToggleCaller(g_yz_side_shown), ToggleItem::AddCallbackCaller(g_yz_side_shown.m_item));
  GlobalToggles_insert("ToggleFrontView", ToggleShown::ToggleCaller(g_xz_front_shown), ToggleItem::AddCallbackCaller(g_xz_front_shown.m_item));
  GlobalCommands_insert("NextView", FreeCaller<toggleActiveOrthoView>(), Accelerator(GDK_Tab, (GdkModifierType)GDK_CONTROL_MASK));
  GlobalCommands_insert("ZoomIn", FreeCaller<XY_ZoomIn>(), Accelerator(GDK_Delete));
  GlobalCommands_insert("ZoomOut", FreeCaller<XY_ZoomOut>(), Accelerator(GDK_Insert));
  GlobalCommands_insert("ViewTop", FreeCaller<XY_Top>());
  GlobalCommands_insert("ViewSide", FreeCaller<XY_Side>());
  GlobalCommands_insert("ViewFront", FreeCaller<XY_Front>());
  GlobalCommands_insert("Zoom100", FreeCaller<XY_Zoom100>());
  GlobalCommands_insert("CenterXYViews", FreeCaller<XY_Split_Focus>(), Accelerator(GDK_Tab, (GdkModifierType)(GDK_SHIFT_MASK|GDK_CONTROL_MASK)));
  GlobalCommands_insert("CenterXYView", FreeCaller<XY_Focus>(), Accelerator(GDK_Tab, (GdkModifierType)(GDK_SHIFT_MASK|GDK_CONTROL_MASK)));

  GlobalPreferenceSystem().registerPreference("SI_ShowCoords", BoolImportStringCaller(g_xywindow_globals_private.show_coordinates), BoolExportStringCaller(g_xywindow_globals_private.show_coordinates));
  GlobalPreferenceSystem().registerPreference("SI_ShowOutlines", BoolImportStringCaller(g_xywindow_globals_private.show_outline), BoolExportStringCaller(g_xywindow_globals_private.show_outline));
  GlobalPreferenceSystem().registerPreference("SI_ShowAxis", BoolImportStringCaller(g_xywindow_globals_private.show_axis), BoolExportStringCaller(g_xywindow_globals_private.show_axis));

  GlobalPreferenceSystem().registerPreference("XZVIS", makeBoolStringImportCallback(ToggleShownImportBoolCaller(g_xz_front_shown)), makeBoolStringExportCallback(ToggleShownExportBoolCaller(g_xz_front_shown)));
  GlobalPreferenceSystem().registerPreference("YZVIS", makeBoolStringImportCallback(ToggleShownImportBoolCaller(g_yz_side_shown)), makeBoolStringExportCallback(ToggleShownExportBoolCaller(g_yz_side_shown)));

  XYWnd::captureStates();
}

void XYWindow_Destroy() {
	XYWnd::releaseStates();
}
