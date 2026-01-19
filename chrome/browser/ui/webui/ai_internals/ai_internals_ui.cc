// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/ai_internals/ai_internals_ui.h"

#include "base/feature_list.h"
#include "chrome/browser/ai/features.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/pref_names.h"
#include "chrome/grit/ai_internals_resources.h"
#include "chrome/grit/ai_internals_resources_map.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "ui/webui/webui_util.h"

bool AiInternalsUIConfig::IsWebUIEnabled(
    content::BrowserContext* browser_context) {
  if (!base::FeatureList::IsEnabled(features::kAiSidePanel)) {
    return false;
  }
  auto* profile = Profile::FromBrowserContext(browser_context);
  if (!profile) {
    return false;
  }
  return profile->GetPrefs()->GetBoolean(prefs::kAiEnabled);
}

AiInternalsUI::AiInternalsUI(content::WebUI* web_ui)
    : TopChromeWebUIController(web_ui) {
  Profile* const profile = Profile::FromWebUI(web_ui);
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      profile, chrome::kChromeUIAiInternalsHost);

  webui::SetupWebUIDataSource(source, kAiInternalsResources,
                              IDR_AI_INTERNALS_AI_INTERNALS_HTML);

  source->AddBoolean("aiEnabled",
                     profile->GetPrefs()->GetBoolean(prefs::kAiEnabled));
  source->AddBoolean(
      "aiSidePanelEnabled",
      profile->GetPrefs()->GetBoolean(prefs::kAiSidePanelEnabled));
  source->AddBoolean(
      "allowRemoteRequests",
      profile->GetPrefs()->GetBoolean(prefs::kAiAllowRemoteRequests));
  source->AddInteger("dataRetentionDays",
                     profile->GetPrefs()->GetInteger(prefs::kAiDataRetentionDays));
}

AiInternalsUI::~AiInternalsUI() = default;

WEB_UI_CONTROLLER_TYPE_IMPL(AiInternalsUI)
