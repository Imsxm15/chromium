// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/side_panel/ai/ai_side_panel_ui.h"

#include <map>
#include <string>
#include <vector>

#include "base/feature_list.h"
#include "base/strings/escape.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "base/values.h"
#include "chrome/browser/ai/features.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/common/pref_names.h"
#include "chrome/grit/ai_side_panel_resources.h"
#include "chrome/grit/ai_side_panel_resources_map.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/common/url_constants.h"
#include "content/public/browser/web_ui_data_source.h"
#include "ui/webui/webui_util.h"

namespace {

base::Value::Dict BuildTabData(content::WebContents* contents) {
  base::Value::Dict tab;
  tab.Set("title", contents->GetTitle());
  tab.Set("url", contents->GetVisibleURL().spec());
  tab.Set("host", contents->GetVisibleURL().host());
  tab.Set("minutesSinceLastActive",
          (base::TimeTicks::Now() - contents->GetLastActiveTimeTicks())
              .InMinutes());
  return tab;
}

base::Value::List BuildTabList(Browser* browser) {
  base::Value::List tabs;
  TabStripModel* tab_strip_model = browser->tab_strip_model();
  for (int index = 0; index < tab_strip_model->count(); ++index) {
    content::WebContents* contents = tab_strip_model->GetWebContentsAt(index);
    if (!contents) {
      continue;
    }
    base::Value::Dict tab = BuildTabData(contents);
    tab.Set("index", index);
    tabs.Append(std::move(tab));
  }
  return tabs;
}

base::Value::Dict BuildCompareResult(Browser* browser,
                                     const std::vector<int>& tab_indices) {
  base::Value::List rows;
  std::string markdown = "| Title | URL | Host |\n| --- | --- | --- |\n";
  std::string csv = "Title,URL,Host\n";
  TabStripModel* tab_strip_model = browser->tab_strip_model();
  for (int index : tab_indices) {
    if (index < 0 || index >= tab_strip_model->count()) {
      continue;
    }
    content::WebContents* contents = tab_strip_model->GetWebContentsAt(index);
    if (!contents) {
      continue;
    }
    const std::string title = base::UTF16ToUTF8(contents->GetTitle());
    const std::string url = contents->GetVisibleURL().spec();
    const std::string host = contents->GetVisibleURL().host();

    base::Value::Dict row;
    row.Set("title", title);
    row.Set("url", url);
    row.Set("host", host);
    rows.Append(std::move(row));

    markdown.append("| " + title + " | " + url + " | " + host + " |\n");
    csv.append("\"" + title + "\",\"" + url + "\",\"" + host + "\"\n");
  }

  base::Value::List notes;
  if (rows.empty()) {
    notes.Append("No tabs selected for comparison.");
  } else {
    notes.Append("Comparison is based on title and URL only.");
    notes.Append("Missing data: page content features not collected yet.");
  }

  base::Value::Dict result;
  result.Set("rows", std::move(rows));
  result.Set("markdown", markdown);
  result.Set("csv", csv);
  result.Set("notes", std::move(notes));
  return result;
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

base::Value::List BuildAgentActions() {
  base::Value::List actions;
  {
    base::Value::Dict action;
    action.Set("id", "focus");
    action.Set("type", "focus");
    action.Set("label", "Focus the page");
    actions.Append(std::move(action));
  }
  {
    base::Value::Dict action;
    action.Set("id", "click");
    action.Set("type", "click");
    action.Set("label", "Click element by selector");
    action.Set("selector", "button");
    actions.Append(std::move(action));
  }
  {
    base::Value::Dict action;
    action.Set("id", "fill");
    action.Set("type", "fill");
    action.Set("label", "Fill input by selector");
    action.Set("selector", "input");
    action.Set("value", "example");
    actions.Append(std::move(action));
  }
  return actions;
}

std::u16string BuildActionScript(const std::string& type,
                                 const std::string& selector,
                                 const std::string& value) {
  if (type == "focus") {
    return u"(() => { window.focus(); return true; })();";
  }

  const std::u16string escaped_selector =
      base::EscapeJavaScriptString(selector);
  const std::u16string escaped_value = base::EscapeJavaScriptString(value);
  if (type == "click") {
    return u"(() => { const el = document.querySelector(" +
           escaped_selector +
           u"); if (!el) return false; el.click(); return true; })();";
  }
  if (type == "fill") {
    return u"(() => { const el = document.querySelector(" +
           escaped_selector +
           u"); if (!el) return false; el.focus(); el.value = " +
           escaped_value +
           u"; el.dispatchEvent(new Event('input', {bubbles:true}));"
           u" el.dispatchEvent(new Event('change', {bubbles:true}));"
           u" return true; })();";
  }
  return u"(() => false)();";
}

}  // namespace

AiSidePanelMessageHandler::AiSidePanelMessageHandler() = default;

AiSidePanelMessageHandler::~AiSidePanelMessageHandler() = default;

void AiSidePanelMessageHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "getAiLocalSummary",
      base::BindRepeating(&AiSidePanelMessageHandler::HandleGetLocalSummary,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "getAiTabList",
      base::BindRepeating(&AiSidePanelMessageHandler::HandleGetTabList,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "compareAiTabs",
      base::BindRepeating(&AiSidePanelMessageHandler::HandleCompareTabs,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "getAiAgentActions",
      base::BindRepeating(&AiSidePanelMessageHandler::HandleGetAgentActions,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "executeAiAgentAction",
      base::BindRepeating(&AiSidePanelMessageHandler::HandleExecuteAgentAction,
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

  PrefService* prefs = browser->profile()->GetPrefs();
  const bool local_summaries_enabled =
      base::FeatureList::IsEnabled(features::kAiLocalSummaries) &&
      prefs->GetBoolean(prefs::kAiEnabled) &&
      prefs->GetBoolean(prefs::kAiSidePanelEnabled);
  if (!local_summaries_enabled) {
    base::Value::Dict error;
    error.Set("error", "Local summaries are disabled.");
    ResolveJavascriptCallback(callback_id, std::move(error));
    return;
  }

  ResolveJavascriptCallback(callback_id, BuildLocalSummary(browser));
}

void AiSidePanelMessageHandler::HandleGetTabList(
    const base::Value::List& args) {
  AllowJavascript();
  CHECK_GE(args.size(), 1u);
  const base::Value& callback_id = args[0];

  content::WebContents* contents = web_ui()->GetWebContents();
  Browser* browser = chrome::FindBrowserWithTab(contents);
  if (!browser) {
    ResolveJavascriptCallback(callback_id, base::Value::List());
    return;
  }

  ResolveJavascriptCallback(callback_id, BuildTabList(browser));
}

void AiSidePanelMessageHandler::HandleCompareTabs(
    const base::Value::List& args) {
  AllowJavascript();
  CHECK_GE(args.size(), 2u);
  const base::Value& callback_id = args[0];
  const base::Value::List& selected = args[1].GetList();

  content::WebContents* contents = web_ui()->GetWebContents();
  Browser* browser = chrome::FindBrowserWithTab(contents);
  if (!browser) {
    ResolveJavascriptCallback(callback_id, base::Value::Dict());
    return;
  }

  PrefService* prefs = browser->profile()->GetPrefs();
  const bool compare_enabled =
      base::FeatureList::IsEnabled(features::kAiComparePages) &&
      prefs->GetBoolean(prefs::kAiEnabled) &&
      prefs->GetBoolean(prefs::kAiSidePanelEnabled);
  if (!compare_enabled) {
    base::Value::Dict error;
    error.Set("error", "Compare is disabled.");
    ResolveJavascriptCallback(callback_id, std::move(error));
    return;
  }

  std::vector<int> tab_indices;
  tab_indices.reserve(selected.size());
  for (const auto& value : selected) {
    if (!value.is_int()) {
      continue;
    }
    tab_indices.push_back(value.GetInt());
  }

  ResolveJavascriptCallback(callback_id,
                            BuildCompareResult(browser, tab_indices));
}

void AiSidePanelMessageHandler::HandleGetAgentActions(
    const base::Value::List& args) {
  AllowJavascript();
  CHECK_GE(args.size(), 1u);
  const base::Value& callback_id = args[0];
  ResolveJavascriptCallback(callback_id, BuildAgentActions());
}

void AiSidePanelMessageHandler::HandleExecuteAgentAction(
    const base::Value::List& args) {
  AllowJavascript();
  CHECK_GE(args.size(), 2u);
  const base::Value& callback_id = args[0];
  const base::Value::Dict& action = args[1].GetDict();
  const std::string* type = action.FindString("type");
  const std::string* selector = action.FindString("selector");
  const std::string* value = action.FindString("value");
  if (!type) {
    ResolveJavascriptCallback(callback_id, base::Value::Dict());
    return;
  }

  content::WebContents* contents = web_ui()->GetWebContents();
  Browser* browser = chrome::FindBrowserWithTab(contents);
  if (!browser) {
    ResolveJavascriptCallback(callback_id, base::Value::Dict());
    return;
  }

  PrefService* prefs = browser->profile()->GetPrefs();
  const bool agent_enabled =
      base::FeatureList::IsEnabled(features::kAiAgentActions) &&
      prefs->GetBoolean(prefs::kAiEnabled) &&
      prefs->GetBoolean(prefs::kAiSidePanelEnabled);
  if (!agent_enabled) {
    base::Value::Dict error;
    error.Set("error", "Agent actions are disabled.");
    ResolveJavascriptCallback(callback_id, std::move(error));
    return;
  }

  content::WebContents* target = browser->tab_strip_model()->GetActiveWebContents();
  if (!target) {
    ResolveJavascriptCallback(callback_id, base::Value::Dict());
    return;
  }

  if (!target->GetVisibleURL().SchemeIs(content::kChromeUIScheme)) {
    base::Value::Dict error;
    error.Set("error", "Agent actions are limited to chrome:// pages in this MVP.");
    ResolveJavascriptCallback(callback_id, std::move(error));
    return;
  }

  const std::string selector_value = selector ? *selector : std::string();
  const std::string input_value = value ? *value : std::string();
  std::u16string script = BuildActionScript(*type, selector_value, input_value);
  target->GetPrimaryMainFrame()->ExecuteJavaScript(
      script,
      base::BindOnce(&AiSidePanelMessageHandler::OnActionScriptExecuted,
                     weak_factory_.GetWeakPtr(), base::Value(callback_id)));
}

void AiSidePanelMessageHandler::OnActionScriptExecuted(
    base::Value callback_id,
    base::Value result) {
  base::Value::Dict response;
  if (result.is_bool()) {
    response.Set("success", result.GetBool());
  } else {
    response.Set("success", false);
  }
  ResolveJavascriptCallback(callback_id, std::move(response));
}

AiSidePanelUI::AiSidePanelUI(content::WebUI* web_ui)
    : TopChromeWebUIController(web_ui) {
  Profile* const profile = Profile::FromWebUI(web_ui);
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      profile, chrome::kChromeUIAiSidePanelHost);

  webui::SetupWebUIDataSource(source, kAiSidePanelResources,
                              IDR_AI_SIDE_PANEL_AI_SIDE_PANEL_HTML);

  source->AddBoolean("localSummariesEnabled",
                     base::FeatureList::IsEnabled(features::kAiLocalSummaries));

  web_ui->AddMessageHandler(std::make_unique<AiSidePanelMessageHandler>());
}

AiSidePanelUI::~AiSidePanelUI() = default;

WEB_UI_CONTROLLER_TYPE_IMPL(AiSidePanelUI)
