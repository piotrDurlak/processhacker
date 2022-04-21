/*
 * Process Hacker -
 *   Main window: Services tab
 *
 * Copyright (C) 2009-2016 wj32
 * Copyright (C) 2017-2021 dmex
 *
 * This file is part of Process Hacker.
 *
 * Process Hacker is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Process Hacker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Process Hacker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <phapp.h>
#include <mainwnd.h>

#include <emenu.h>

#include <phplug.h>
#include <phsettings.h>
#include <settings.h>
#include <srvlist.h>
#include <srvprv.h>

#include <mainwndp.h>

PPH_MAIN_TAB_PAGE PhMwpServicesPage;
HWND PhMwpServiceTreeNewHandle;
PH_PROVIDER_EVENT_QUEUE PhMwpServiceEventQueue;

static PH_CALLBACK_REGISTRATION ServiceAddedRegistration;
static PH_CALLBACK_REGISTRATION ServiceModifiedRegistration;
static PH_CALLBACK_REGISTRATION ServiceRemovedRegistration;
static PH_CALLBACK_REGISTRATION ServicesUpdatedRegistration;

static BOOLEAN ServiceTreeListLoaded = FALSE;
static PPH_TN_FILTER_ENTRY DriverFilterEntry = NULL;
static PPH_TN_FILTER_ENTRY MicrosoftFilterEntry = NULL;





VOID PhMwpToggleDriverServiceTreeFilter(
    VOID
    )
{
    if (!DriverFilterEntry)
    {
        DriverFilterEntry = PhAddTreeNewFilter(PhGetFilterSupportServiceTreeList(), PhMwpDriverServiceTreeFilter, NULL);
    }
    else
    {
        PhRemoveTreeNewFilter(PhGetFilterSupportServiceTreeList(), DriverFilterEntry);
        DriverFilterEntry = NULL;
    }

    PhApplyTreeNewFilters(PhGetFilterSupportServiceTreeList());

    PhSetIntegerSetting(L"HideDriverServices", !!DriverFilterEntry);
}

VOID PhMwpToggleMicrosoftServiceTreeFilter(
    VOID
    )
{
    if (MicrosoftFilterEntry)
    {
        PhRemoveTreeNewFilter(PhGetFilterSupportServiceTreeList(), MicrosoftFilterEntry);
        MicrosoftFilterEntry = NULL;
    }
    else
    {
        if (!PhEnableServiceQueryStage2)
        {
            PhShowInformation2(
                PhMainWndHandle,
                NULL,
                L"This filter cannot function because digital signature checking is not enabled.\r\n%s",
                L"Enable it in Options > General and restart Process Hacker."
                );
            return;
        }

        MicrosoftFilterEntry = PhAddTreeNewFilter(PhGetFilterSupportServiceTreeList(), PhMwpMicrosoftServiceTreeFilter, NULL);
    }

    PhApplyTreeNewFilters(PhGetFilterSupportServiceTreeList());

    PhSetIntegerSetting(L"HideDefaultServices", !!MicrosoftFilterEntry);
}

BOOLEAN PhMwpDriverServiceTreeFilter(
    _In_ PPH_TREENEW_NODE Node,
    _In_opt_ PVOID Context
    )
{
    PPH_SERVICE_NODE serviceNode = (PPH_SERVICE_NODE)Node;

    if (serviceNode->ServiceItem->Type & SERVICE_DRIVER)
        return FALSE;

    return TRUE;
}

BOOLEAN PhMwpMicrosoftServiceTreeFilter(
    _In_ PPH_TREENEW_NODE Node,
    _In_opt_ PVOID Context
    )
{
    static PH_STRINGREF microsoftSignerNameSr = PH_STRINGREF_INIT(L"Microsoft Windows");
    PPH_SERVICE_NODE serviceNode = (PPH_SERVICE_NODE)Node;

    if (!PhIsNullOrEmptyString(serviceNode->ServiceItem->VerifySignerName))
    {
        if (PhEqualStringRef(&serviceNode->ServiceItem->VerifySignerName->sr, &microsoftSignerNameSr, TRUE))
            return FALSE;
    }

    return TRUE;
}

VOID PhMwpInitializeServiceMenu(
    _In_ PPH_EMENU Menu,
    _In_ PPH_SERVICE_ITEM *Services,
    _In_ ULONG NumberOfServices
    )
{
    if (NumberOfServices == 0)
    {
        PhSetFlagsAllEMenuItems(Menu, PH_EMENU_DISABLED, PH_EMENU_DISABLED);
    }
    else if (NumberOfServices == 1)
    {
        if (!Services[0]->ProcessId)
            PhEnableEMenuItem(Menu, ID_SERVICE_GOTOPROCESS, FALSE);
    }
    else
    {
        PhSetFlagsAllEMenuItems(Menu, PH_EMENU_DISABLED, PH_EMENU_DISABLED);
        PhEnableEMenuItem(Menu, ID_SERVICE_COPY, TRUE);
    }

    if (NumberOfServices == 1)
    {
        switch (Services[0]->State)
        {
        case SERVICE_RUNNING:
            {
                PhEnableEMenuItem(Menu, ID_SERVICE_START, FALSE);
                PhEnableEMenuItem(Menu, ID_SERVICE_CONTINUE, FALSE);
                PhEnableEMenuItem(Menu, ID_SERVICE_PAUSE,
                    Services[0]->ControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE);
                PhEnableEMenuItem(Menu, ID_SERVICE_STOP,
                    Services[0]->ControlsAccepted & SERVICE_ACCEPT_STOP);
            }
            break;
        case SERVICE_PAUSED:
            {
                PhEnableEMenuItem(Menu, ID_SERVICE_START, FALSE);
                PhEnableEMenuItem(Menu, ID_SERVICE_CONTINUE,
                    Services[0]->ControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE);
                PhEnableEMenuItem(Menu, ID_SERVICE_PAUSE, FALSE);
                PhEnableEMenuItem(Menu, ID_SERVICE_STOP,
                    Services[0]->ControlsAccepted & SERVICE_ACCEPT_STOP);
            }
            break;
        case SERVICE_STOPPED:
            {
                PhEnableEMenuItem(Menu, ID_SERVICE_CONTINUE, FALSE);
                PhEnableEMenuItem(Menu, ID_SERVICE_PAUSE, FALSE);
                PhEnableEMenuItem(Menu, ID_SERVICE_STOP, FALSE);
            }
            break;
        case SERVICE_START_PENDING:
        case SERVICE_CONTINUE_PENDING:
        case SERVICE_PAUSE_PENDING:
        case SERVICE_STOP_PENDING:
            {
                PhEnableEMenuItem(Menu, ID_SERVICE_START, FALSE);
                PhEnableEMenuItem(Menu, ID_SERVICE_CONTINUE, FALSE);
                PhEnableEMenuItem(Menu, ID_SERVICE_PAUSE, FALSE);
                PhEnableEMenuItem(Menu, ID_SERVICE_STOP, FALSE);
            }
            break;
        }

        if (!(Services[0]->ControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE))
        {
            PPH_EMENU_ITEM item;

            if (item = PhFindEMenuItem(Menu, 0, NULL, ID_SERVICE_CONTINUE))
                PhDestroyEMenuItem(item);
            if (item = PhFindEMenuItem(Menu, 0, NULL, ID_SERVICE_PAUSE))
                PhDestroyEMenuItem(item);
        }
    }
}

VOID PhShowServiceContextMenu(
    _In_ PPH_TREENEW_CONTEXT_MENU ContextMenu
    )
{
    PH_PLUGIN_MENU_INFORMATION menuInfo;
    PPH_SERVICE_ITEM *services;
    ULONG numberOfServices;

    PhGetSelectedServiceItems(&services, &numberOfServices);

    if (numberOfServices != 0)
    {
        PPH_EMENU menu;
        PPH_EMENU_ITEM item;

        menu = PhCreateEMenu();
        PhInsertEMenuItem(menu, PhCreateEMenuItem(0, ID_SERVICE_START, L"&Start", NULL, NULL), ULONG_MAX);
        PhInsertEMenuItem(menu, PhCreateEMenuItem(0, ID_SERVICE_CONTINUE, L"C&ontinue", NULL, NULL), ULONG_MAX);
        PhInsertEMenuItem(menu, PhCreateEMenuItem(0, ID_SERVICE_PAUSE, L"&Pause", NULL, NULL), ULONG_MAX);
        PhInsertEMenuItem(menu, PhCreateEMenuItem(0, ID_SERVICE_STOP, L"S&top", NULL, NULL), ULONG_MAX);
        PhInsertEMenuItem(menu, PhCreateEMenuItem(0, ID_SERVICE_DELETE, L"&Delete\bDel", NULL, NULL), ULONG_MAX);
        PhInsertEMenuItem(menu, PhCreateEMenuSeparator(), ULONG_MAX);
        PhInsertEMenuItem(menu, PhCreateEMenuItem(0, ID_SERVICE_GOTOPROCESS, L"&Go to process", NULL, NULL), ULONG_MAX);
        PhInsertEMenuItem(menu, PhCreateEMenuSeparator(), ULONG_MAX);
        PhInsertEMenuItem(menu, PhCreateEMenuItem(0, ID_SERVICE_OPENKEY, L"Open &key", NULL, NULL), ULONG_MAX);
        PhInsertEMenuItem(menu, PhCreateEMenuItem(0, ID_SERVICE_OPENFILELOCATION, L"Open &file location\bCtrl+Enter", NULL, NULL), ULONG_MAX);
        PhInsertEMenuItem(menu, PhCreateEMenuItem(0, ID_SERVICE_PROPERTIES, L"P&roperties\bEnter", NULL, NULL), ULONG_MAX);
        PhInsertEMenuItem(menu, PhCreateEMenuSeparator(), ULONG_MAX);
        PhInsertEMenuItem(menu, PhCreateEMenuItem(0, ID_SERVICE_COPY, L"&Copy\bCtrl+C", NULL, NULL), ULONG_MAX);
        PhSetFlagsEMenuItem(menu, ID_SERVICE_PROPERTIES, PH_EMENU_DEFAULT, PH_EMENU_DEFAULT);
        PhMwpInitializeServiceMenu(menu, services, numberOfServices);
        PhInsertCopyCellEMenuItem(menu, ID_SERVICE_COPY, PhMwpServiceTreeNewHandle, ContextMenu->Column);

        if (PhPluginsEnabled)
        {
            PhPluginInitializeMenuInfo(&menuInfo, menu, PhMainWndHandle, 0);
            menuInfo.u.Service.Services = services;
            menuInfo.u.Service.NumberOfServices = numberOfServices;

            PhInvokeCallback(PhGetGeneralCallback(GeneralCallbackServiceMenuInitializing), &menuInfo);
        }

        item = PhShowEMenu(
            menu,
            PhMainWndHandle,
            PH_EMENU_SHOW_LEFTRIGHT,
            PH_ALIGN_LEFT | PH_ALIGN_TOP,
            ContextMenu->Location.x,
            ContextMenu->Location.y
            );

        if (item)
        {
            BOOLEAN handled = FALSE;

            handled = PhHandleCopyCellEMenuItem(item);

            if (!handled && PhPluginsEnabled)
                handled = PhPluginTriggerEMenuItem(&menuInfo, item);

            if (!handled)
                SendMessage(PhMainWndHandle, WM_COMMAND, item->Id, 0);
        }

        PhDestroyEMenu(menu);
    }

    PhFree(services);
}














