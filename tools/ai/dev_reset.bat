@echo off
setlocal enabledelayedexpansion

set "USER_DATA=%LOCALAPPDATA%\Chromium\User Data"
if not exist "%USER_DATA%" (
  echo Chromium user data not found: "%USER_DATA%"
  exit /b 0
)

set "PREF_PATH=%USER_DATA%\Default\Preferences"
if exist "%PREF_PATH%" (
  powershell -NoProfile -Command ^
    "try { " ^
    "$prefs = Get-Content -Raw '%PREF_PATH%' | ConvertFrom-Json; " ^
    "if ($prefs.PSObject.Properties.Name -contains 'ai') { " ^
    "  $prefs.PSObject.Properties.Remove('ai') | Out-Null; " ^
    "} " ^
    "$prefs | ConvertTo-Json -Depth 100 | Set-Content '%PREF_PATH%'; " ^
    "Write-Host 'Cleared AI prefs in Preferences.'; " ^
    "} catch { " ^
    "Write-Host 'Failed to update Preferences. Ensure Chromium is closed.'; " ^
    "}"
)

for %%D in ("%USER_DATA%\Default\AI" "%USER_DATA%\Default\AIData" "%USER_DATA%\AI") do (
  if exist "%%~D" (
    rmdir /s /q "%%~D"
    echo Deleted "%%~D"
  )
)

echo AI reset complete.
