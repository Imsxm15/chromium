# AI Browser MVP

## Architecture
- **Side panel UI**: `chrome://ai-side-panel` renders the MVP surface and
  communicates with `AiSidePanelMessageHandler` for local summaries, compare,
  agent actions, security insights, and the optional remote analysis preview.
- **Local signals**: heuristics are computed locally from the active tab URL to
  keep privacy guarantees intact.
- **Remote abstraction**: `AiModelProvider` defines a small interface for
  building a remote request preview and (optionally) performing a call. The
  OpenAI-like implementation is disabled by default and only returns preview
  data or a stub response.

## Feature flags
- `kAiSidePanel`
- `kAiLocalSummaries`
- `kAiComparePages`
- `kAiAnchoredNotes`
- `kAiProjects`
- `kAiAgentActions`
- `kAiSecurityInsights`

## Prefs
- `ai.enabled`
- `ai.side_panel.enabled`
- `ai.allow_remote_requests`
- `ai.remote_endpoint`
- `ai.remote_api_key` (encrypted via OS keychain when supported)
- `ai.data_retention_days`
- `ai.notes.enabled`
- `ai.projects.enabled`
- `ai.semantic_history.enabled`
- `ai.permissions.expire_days`

## Build
```bash
gn gen out/Default
ninja -C out/Default chrome
```

## Tests
```bash
ninja -C out/Default chrome_unittests
```

## Dev reset
Use `tools/ai/dev_reset.bat` on Windows to clear AI prefs and best-effort
profile artifacts while Chromium is closed.

## Limitations
- Remote LLM calls are **not** executed in the MVP; only a preview payload is
  shown and a stub response is returned.
- Side panel features are intentionally minimal and gated behind prefs/flags.
