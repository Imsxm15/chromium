// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/side_panel/ai/ai_side_panel_ui.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/grit/ai_side_panel_resources.h"
#include "chrome/grit/ai_side_panel_resources_map.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "ui/webui/webui_util.h"

AiSidePanelUI::AiSidePanelUI(content::WebUI* web_ui)
    : TopChromeWebUIController(web_ui) {
  Profile* const profile = Profile::FromWebUI(web_ui);
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      profile, chrome::kChromeUIAiSidePanelHost);

  webui::SetupWebUIDataSource(source, kAiSidePanelResources,
                              IDR_AI_SIDE_PANEL_AI_SIDE_PANEL_HTML);
}

AiSidePanelUI::~AiSidePanelUI() = default;

WEB_UI_CONTROLLER_TYPE_IMPL(AiSidePanelUI)
