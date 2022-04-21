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

























