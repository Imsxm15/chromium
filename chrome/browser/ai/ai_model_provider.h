// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_AI_AI_MODEL_PROVIDER_H_
#define CHROME_BROWSER_AI_AI_MODEL_PROVIDER_H_

#include <memory>
#include <string>

#include "base/values.h"
#include "url/gurl.h"

class PrefService;
class Profile;

class AiModelProvider {
 public:
  struct RemoteRequest {
    std::string endpoint;
    base::Value::Dict headers;
    base::Value::Dict payload;
  };

  virtual ~AiModelProvider() = default;

  virtual bool IsAvailable() const = 0;
  virtual RemoteRequest BuildRemoteRequest(const GURL& url,
                                           const std::string& title) const = 0;
  virtual base::Value::Dict RequestAnalysis(const GURL& url,
                                            const std::string& title) = 0;
};

std::unique_ptr<AiModelProvider> CreateAiModelProvider(Profile* profile);

bool StoreAiRemoteApiKey(PrefService* prefs, const std::string& api_key);
void ClearAiRemoteApiKey(PrefService* prefs);
std::string GetAiRemoteApiKey(const PrefService* prefs);

#endif  // CHROME_BROWSER_AI_AI_MODEL_PROVIDER_H_
