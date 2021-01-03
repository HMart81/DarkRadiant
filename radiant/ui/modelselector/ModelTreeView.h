#pragma once

#include "wxutil/dataview/ResourceTreeView.h"
#include <wx/artprov.h>

namespace ui
{

class ModelTreeView :
    public wxutil::ResourceTreeView
{
public:
    // Treemodel definition
    struct TreeColumns :
        public wxutil::ResourceTreeView::Columns
    {
        TreeColumns() :
            modelPath(add(wxutil::TreeModel::Column::String)),
            skin(add(wxutil::TreeModel::Column::String)),
            isSkin(add(wxutil::TreeModel::Column::Boolean))
        {}

        // iconAndName column contains the filename, e.g. "chair1.lwo"
        // fullPath column contains the VFS path to the model plus skin info, e.g. "models/darkmod/props/chair1.lwo[/skinName]"
        wxutil::TreeModel::Column skin;		// e.g. "chair1_brown_wood", or "" for no skin
        wxutil::TreeModel::Column modelPath;// e.g. "models/darkmod/props/chair1.lwo"
        wxutil::TreeModel::Column isSkin;	// TRUE if this is a skin entry, FALSE if actual model or folder
    };

private:
    bool _showSkins;
    const TreeColumns& _columns;

    wxDataViewItem _progressItem;
    wxIcon _modelIcon;

public:
    ModelTreeView(wxWindow* parent, const TreeColumns& columns) :
        ResourceTreeView(parent, columns, wxBORDER_STATIC | wxDV_NO_HEADER),
        _columns(columns),
        _showSkins(true)
    {
        // Single visible column, containing the directory/shader name and the icon
        AppendIconTextColumn(
            _("Model Path"), _columns.iconAndName.getColumnIndex(),
            wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE,
            wxALIGN_NOT, wxDATAVIEW_COL_SORTABLE
        );

        // Use the TreeModel's full string search function
        AddSearchColumn(_columns.iconAndName);
        EnableFavouriteManagement(decl::Type::Model);
    }

    void SetShowSkins(bool showSkins)
    {
        if (_showSkins == showSkins)
        {
            return;
        }

        // Try to keep the selection intact when switching modes
        auto previousSelection = GetSelectedFullname();

        _showSkins = showSkins;

        SetupTreeModelFilter(); // refresh the view

        if (!previousSelection.empty())
        {
            SetSelectedFullname(previousSelection);
        }
    }

    std::string GetSelectedModelPath()
    {
        return GetColumnValue(_columns.modelPath);
    }

    std::string GetSelectedSkin()
    {
        return GetColumnValue(_columns.skin);
    }

protected:
    bool IsTreeModelRowVisible(wxutil::TreeModel::Row& row) override
    {
        if (!_showSkins && row[_columns.isSkin].getBool())
        {
            return false; // it's a skin, and we shouldn't show it
        }

        // Pass to the base class
        return ResourceTreeView::IsTreeModelRowVisible(row);
    }

private:
    std::string GetColumnValue(const wxutil::TreeModel::Column& column)
    {
        auto item = GetSelection();

        if (!item.IsOk()) return "";

        wxutil::TreeModel::Row row(item, *GetModel());

        return row[column];
    }
};

}
