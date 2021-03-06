﻿
//
// Air Conditioner - Server MVC View (GUI View Manager)
// Youjie Zhang, 2017
//

#include <memory>
#include <functional>

#include "server-view-controller.h"
#include "server-view-gui.h"

namespace Air_Conditioner
{
    void ServerViewManager::ToWelcomeView ()
    {
        using namespace std::placeholders;
        auto controller = std::make_shared<WelcomeController> (*this);
        _Navigate<WelcomeViewGUI> (
            std::bind (&WelcomeController::Nav, controller, _1));
    }

    void ServerViewManager::ToConfigView ()
    {
        using namespace std::placeholders;
        auto controller = std::make_shared<ConfigController> ();
        _Navigate<ConfigViewGUI> (
            controller->GetConfig (),
            std::bind (&ConfigController::SetConfig, controller, _1),
            std::bind (&ServerViewManager::ToWelcomeView, this));
    }

    void ServerViewManager::ToGuestView ()
    {
        using namespace std::placeholders;
        auto controller = std::make_shared<GuestInfoController> ();
        _Navigate<GuestViewGUI> (
            controller->GetGuestList (),
            std::bind (&GuestInfoController::AddGuest, controller, _1),
            std::bind (&GuestInfoController::RemoveGuest, controller, _1),
            std::bind (&ServerViewManager::ToWelcomeView, this));
    }

    void ServerViewManager::ToLogView ()
    {
        using namespace std::placeholders;
        auto controller = std::make_shared<LogController> ();
        auto timeRange = controller->GetTimeRange ();
        _Navigate<LogViewGUI> (
            timeRange.first, timeRange.second,
            std::bind (&LogController::GetLogOnOff, controller, _1, _2),
            std::bind (&LogController::GetLogRequest, controller, _1, _2),
            std::bind (&ServerViewManager::ToWelcomeView, this));
    }

    void ServerViewManager::ToClientView ()
    {
        using namespace std::placeholders;
        auto controller = std::make_shared<ClientController> ();
        _Navigate<ClientViewGUI> (
            std::bind (&ClientController::GetClientList, controller),
            std::bind (&ServerViewManager::ToWelcomeView, this));
    }
}
