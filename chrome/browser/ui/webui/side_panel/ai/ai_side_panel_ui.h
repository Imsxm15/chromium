// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_SIDE_PANEL_AI_AI_SIDE_PANEL_UI_H_
#define CHROME_BROWSER_UI_WEBUI_SIDE_PANEL_AI_AI_SIDE_PANEL_UI_H_

#include "chrome/browser/ui/webui/top_chrome/top_chrome_web_ui_controller.h"
#include "chrome/browser/ui/webui/top_chrome/top_chrome_webui_config.h"
#include "chrome/common/webui_url_constants.h"
#include "content/public/browser/web_ui_message_handler.h"

class AiSidePanelUI;

class AiSidePanelMessageHandler : public content::WebUIMessageHandler {
 public:
  AiSidePanelMessageHandler();
  AiSidePanelMessageHandler(const AiSidePanelMessageHandler&) = delete;
  AiSidePanelMessageHandler& operator=(const AiSidePanelMessageHandler&) =
      delete;
  ~AiSidePanelMessageHandler() override;

  void RegisterMessages() override;

 private:
  void HandleGetLocalSummary(const base::Value::List& args);
};

class AiSidePanelUIConfig : public DefaultTopChromeWebUIConfig<AiSidePanelUI> {
 public:
  AiSidePanelUIConfig()
      : DefaultTopChromeWebUIConfig(content::kChromeUIScheme,
                                    chrome::kChromeUIAiSidePanelHost) {}
};

class AiSidePanelUI : public TopChromeWebUIController {
 public:
  explicit AiSidePanelUI(content::WebUI* web_ui);
  AiSidePanelUI(const AiSidePanelUI&) = delete;
  AiSidePanelUI& operator=(const AiSidePanelUI&) = delete;
  ~AiSidePanelUI() override;

  static constexpr std::string_view GetWebUIName() { return "AiSidePanel"; }

  WEB_UI_CONTROLLER_TYPE_DECL();
};

#endif  // CHROME_BROWSER_UI_WEBUI_SIDE_PANEL_AI_AI_SIDE_PANEL_UI_H_
