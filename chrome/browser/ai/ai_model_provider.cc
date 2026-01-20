// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ai/ai_model_provider.h"

#include <utility>

#include "base/base64.h"
#include "base/strings/stringprintf.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/pref_names.h"
#include "components/os_crypt/sync/os_crypt.h"
#include "components/prefs/pref_service.h"

namespace {

constexpr char kDefaultRemoteModelName[] = "placeholder-model";

std::string MaskApiKey(const std::string& api_key) {
  if (api_key.empty()) {
    return "unset";
  }
  constexpr size_t kVisibleChars = 4;
  if (api_key.size() <= kVisibleChars) {
    return std::string(api_key.size(), '*');
  }
  return base::StringPrintf("****%s",
                            api_key.substr(api_key.size() - kVisibleChars)
                                .c_str());
}

class DummyModelProvider : public AiModelProvider {
 public:
  DummyModelProvider() = default;
  ~DummyModelProvider() override = default;

  bool IsAvailable() const override { return false; }

  RemoteRequest BuildRemoteRequest(const GURL& url,
                                   const std::string& title) const override {
    RemoteRequest request;
    request.endpoint = std::string();
    request.headers.Set("Authorization", "unset");
    request.headers.Set("Content-Type", "application/json");
    request.payload.Set("model", kDefaultRemoteModelName);
    request.payload.Set("url", url.spec());
    request.payload.Set("title", title);
    return request;
  }

  base::Value::Dict RequestAnalysis(const GURL& url,
                                    const std::string& title) override {
    base::Value::Dict response;
    response.Set("error", "Remote provider is not configured.");
    response.Set("url", url.spec());
    response.Set("title", title);
    return response;
  }
};

class OpenAiLikeModelProvider : public AiModelProvider {
 public:
  OpenAiLikeModelProvider(std::string endpoint, std::string api_key)
      : endpoint_(std::move(endpoint)), api_key_(std::move(api_key)) {}
  ~OpenAiLikeModelProvider() override = default;

  bool IsAvailable() const override {
    return !endpoint_.empty() && !api_key_.empty();
  }

  RemoteRequest BuildRemoteRequest(const GURL& url,
                                   const std::string& title) const override {
    RemoteRequest request;
    request.endpoint = endpoint_;
    request.headers.Set("Authorization",
                        base::StringPrintf("Bearer %s",
                                           MaskApiKey(api_key_).c_str()));
    request.headers.Set("Content-Type", "application/json");

    base::Value::Dict payload;
    payload.Set("model", kDefaultRemoteModelName);
    base::Value::List messages;
    {
      base::Value::Dict message;
      message.Set("role", "system");
      message.Set("content",
                  "You are a browser security assistant. Summarize risks.");
      messages.Append(std::move(message));
    }
    {
      base::Value::Dict message;
      message.Set("role", "user");
      message.Set("content", base::StringPrintf("Page title: %s\nURL: %s",
                                                title.c_str(),
                                                url.spec().c_str()));
      messages.Append(std::move(message));
    }
    payload.Set("messages", std::move(messages));
    request.payload = std::move(payload);
    return request;
  }

  base::Value::Dict RequestAnalysis(const GURL& url,
                                    const std::string& title) override {
    base::Value::Dict response;
    response.Set("error",
                 "Remote provider calls are disabled in this MVP.");
    response.Set("url", url.spec());
    response.Set("title", title);
    response.Set("endpoint", endpoint_);
    return response;
  }

 private:
  std::string endpoint_;
  std::string api_key_;
};

}  // namespace

std::unique_ptr<AiModelProvider> CreateAiModelProvider(Profile* profile) {
  PrefService* prefs = profile->GetPrefs();
  const std::string endpoint = prefs->GetString(prefs::kAiRemoteEndpoint);
  const std::string api_key = GetAiRemoteApiKey(prefs);
  if (!endpoint.empty() && !api_key.empty()) {
    return std::make_unique<OpenAiLikeModelProvider>(endpoint, api_key);
  }
  return std::make_unique<DummyModelProvider>();
}

bool StoreAiRemoteApiKey(PrefService* prefs, const std::string& api_key) {
  std::string encrypted;
  if (!OSCrypt::EncryptString(api_key, &encrypted)) {
    return false;
  }
  std::string encoded;
  base::Base64Encode(encrypted, &encoded);
  prefs->SetString(prefs::kAiRemoteApiKey, encoded);
  return true;
}

void ClearAiRemoteApiKey(PrefService* prefs) {
  prefs->SetString(prefs::kAiRemoteApiKey, std::string());
}

std::string GetAiRemoteApiKey(const PrefService* prefs) {
  const std::string encoded = prefs->GetString(prefs::kAiRemoteApiKey);
  if (encoded.empty()) {
    return std::string();
  }
  std::string encrypted;
  if (!base::Base64Decode(encoded, &encrypted)) {
    return std::string();
  }
  std::string decrypted;
  if (!OSCrypt::DecryptString(encrypted, &decrypted)) {
    return std::string();
  }
  return decrypted;
}
