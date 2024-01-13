#pragma once

#include <wx/frame.h>

#include "ui/imainframe.h"
#include "messages/AutomaticMapSaveRequest.h"

namespace ui
{

class AutoSaveRequestHandler
{
private:
	std::size_t _msgSubscription;

public:
	AutoSaveRequestHandler()
	{
		_msgSubscription = GlobalRadiantCore().getMessageBus().addListener(
			radiant::IMessage::Type::AutomaticMapSaveRequest,
			radiant::TypeListener<map::AutomaticMapSaveRequest>(
				sigc::mem_fun(*this, &AutoSaveRequestHandler::handleRequest)));
	}

	~AutoSaveRequestHandler()
	{
		GlobalRadiantCore().getMessageBus().removeListener(_msgSubscription);
	}

private:
	void handleRequest(map::AutomaticMapSaveRequest& msg)
	{
		if (!GlobalMainFrame().screenUpdatesEnabled())
		{
			msg.denyWithReason("Screen updates blocked");
			return;
		}

		// greebo: Check if we are in focus
		if (!GlobalMainFrame().isActiveApp())
		{
			msg.denyWithReason("Main window not present or not shown on screen, "
				"will wait for another period.");
			return;
		}

		// Check if the user is currently pressing a mouse button
		// Don't start the save if the user is holding a mouse button
		if (wxGetMouseState().ButtonIsDown(wxMOUSE_BTN_ANY))
		{
			msg.denyWithReason("Mouse button held down");
			return;
		}
	}
};

}
