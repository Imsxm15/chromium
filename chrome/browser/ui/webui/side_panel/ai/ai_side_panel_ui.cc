// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/side_panel/ai/ai_side_panel_ui.h"

#include <map>
#include <string>

#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "base/values.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/grit/ai_side_panel_resources.h"
#include "chrome/grit/ai_side_panel_resources_map.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "ui/webui/webui_util.h"

namespace {

base::Value::Dict BuildTabData(content::WebContents* contents) {
  base::Value::Dict tab;
  tab.Set("title", contents->GetTitle());
  tab.Set("url", contents->GetVisibleURL().spec());
  tab.Set("minutesSinceLastActive",
          (base::TimeTicks::Now() - contents->GetLastActiveTimeTicks())
              .InMinutes());
  return tab;
}

base::Value::Dict BuildLocalSummary(Browser* browser) {
  base::Value::List tabs;
  std::map<std::string, int> host_counts;

  TabStripModel* tab_strip_model = browser->tab_strip_model();
  for (int index = 0; index < tab_strip_model->count(); ++index) {
    content::WebContents* contents = tab_strip_model->GetWebContentsAt(index);
    if (!contents) {
      continue;
    }
    tabs.Append(BuildTabData(contents));

    const GURL url = contents->GetVisibleURL();
    if (url.is_valid() && !url.host().empty()) {
      host_counts[url.host()]++;
    }
  }

  base::Value::List summary;
  summary.Append("Local summary (heuristic)");
  if (host_counts.empty()) {
    summary.Append("No active tabs to summarize.");
  } else {
    summary.Append("Top domains by open tabs:");
    for (const auto& [host, count] : host_counts) {
      summary.Append(base::StringPrintf("- %s (%d)",
                                        host.c_str(), count));
    }
    summary.Append("Questions to consider:");
    summary.Append("- Which tabs are duplicates or outdated?");
    summary.Append("- Which tabs need follow-up actions?");
  }

  base::Value::Dict result;
  result.Set("tabs", std::move(tabs));
  result.Set("summary", std::move(summary));
  return result;
}

}  // namespace

AiSidePanelMessageHandler::AiSidePanelMessageHandler() = default;

AiSidePanelMessageHandler::~AiSidePanelMessageHandler() = default;

void AiSidePanelMessageHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "getAiLocalSummary",
      base::BindRepeating(&AiSidePanelMessageHandler::HandleGetLocalSummary,
                          base::Unretained(this)));
}

void AiSidePanelMessageHandler::HandleGetLocalSummary(
    const base::Value::List& args) {
  AllowJavascript();
  CHECK_GE(args.size(), 1u);
  const base::Value& callback_id = args[0];

  content::WebContents* contents = web_ui()->GetWebContents();
  Browser* browser = chrome::FindBrowserWithTab(contents);
  if (!browser) {
    ResolveJavascriptCallback(callback_id, base::Value::Dict());
    return;
  }

  ResolveJavascriptCallback(callback_id, BuildLocalSummary(browser));
}

AiSidePanelUI::AiSidePanelUI(content::WebUI* web_ui)
    : TopChromeWebUIController(web_ui) {
  Profile* const profile = Profile::FromWebUI(web_ui);
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      profile, chrome::kChromeUIAiSidePanelHost);

  webui::SetupWebUIDataSource(source, kAiSidePanelResources,
                              IDR_AI_SIDE_PANEL_AI_SIDE_PANEL_HTML);

  web_ui->AddMessageHandler(std::make_unique<AiSidePanelMessageHandler>());
}

AiSidePanelUI::~AiSidePanelUI() = default;

WEB_UI_CONTROLLER_TYPE_IMPL(AiSidePanelUI)
