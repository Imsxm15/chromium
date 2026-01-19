// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_AI_INTERNALS_AI_INTERNALS_UI_H_
#define CHROME_BROWSER_UI_WEBUI_AI_INTERNALS_AI_INTERNALS_UI_H_

#include "chrome/browser/ui/webui/top_chrome/top_chrome_web_ui_controller.h"
#include "chrome/browser/ui/webui/top_chrome/top_chrome_webui_config.h"
#include "chrome/common/webui_url_constants.h"

class AiInternalsUI;

class AiInternalsUIConfig : public DefaultTopChromeWebUIConfig<AiInternalsUI> {
 public:
  AiInternalsUIConfig()
      : DefaultTopChromeWebUIConfig(content::kChromeUIScheme,
                                    chrome::kChromeUIAiInternalsHost) {}

  bool IsWebUIEnabled(content::BrowserContext* browser_context) override;
};

class AiInternalsUI : public TopChromeWebUIController {
 public:
  explicit AiInternalsUI(content::WebUI* web_ui);
  AiInternalsUI(const AiInternalsUI&) = delete;
  AiInternalsUI& operator=(const AiInternalsUI&) = delete;
  ~AiInternalsUI() override;

  static constexpr std::string_view GetWebUIName() { return "AiInternals"; }

  WEB_UI_CONTROLLER_TYPE_DECL();
};

#endif  // CHROME_BROWSER_UI_WEBUI_AI_INTERNALS_AI_INTERNALS_UI_H_
