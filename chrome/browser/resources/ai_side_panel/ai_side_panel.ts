// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {sendWithPromise} from 'chrome://resources/js/cr.m.js';

const summarizeButton =
    document.querySelector<HTMLButtonElement>('#summarizeTabs');
const summaryOutput = document.querySelector<HTMLDivElement>('#summaryOutput');
const tabsOutput = document.querySelector<HTMLDivElement>('#tabsOutput');
const statusOutput = document.querySelector<HTMLDivElement>('#statusOutput');
const compareStatus = document.querySelector<HTMLDivElement>('#compareStatus');
const compareTabList =
    document.querySelector<HTMLDivElement>('#compareTabList');
const compareButton = document.querySelector<HTMLButtonElement>('#compareTabs');
const compareNotes = document.querySelector<HTMLDivElement>('#compareNotes');
const compareMarkdown =
    document.querySelector<HTMLTextAreaElement>('#compareMarkdown');
const compareCsv = document.querySelector<HTMLTextAreaElement>('#compareCsv');
const agentStatus = document.querySelector<HTMLDivElement>('#agentStatus');
const agentActionList =
    document.querySelector<HTMLDivElement>('#agentActionList');
const agentLog = document.querySelector<HTMLDivElement>('#agentLog');
const securityStatus =
    document.querySelector<HTMLDivElement>('#securityStatus');
const refreshSecurity =
    document.querySelector<HTMLButtonElement>('#refreshSecurity');
const securitySignals =
    document.querySelector<HTMLDivElement>('#securitySignals');
const remoteEndpoint =
    document.querySelector<HTMLInputElement>('#remoteEndpoint');
const remoteApiKey = document.querySelector<HTMLInputElement>('#remoteApiKey');
const remoteKeyState =
    document.querySelector<HTMLDivElement>('#remoteKeyState');
const saveRemoteSettings =
    document.querySelector<HTMLButtonElement>('#saveRemoteSettings');
const clearRemoteKey =
    document.querySelector<HTMLButtonElement>('#clearRemoteKey');
const remoteSettingsStatus =
    document.querySelector<HTMLDivElement>('#remoteSettingsStatus');
const remotePreviewWarning =
    document.querySelector<HTMLDivElement>('#remotePreviewWarning');
const remotePreview =
    document.querySelector<HTMLPreElement>('#remotePreview');
const remoteAnalyze =
    document.querySelector<HTMLButtonElement>('#remoteAnalyze');
const remoteConfirm =
    document.querySelector<HTMLButtonElement>('#remoteConfirm');
const remoteResult =
    document.querySelector<HTMLDivElement>('#remoteResult');

function renderSummary(lines: string[]) {
  if (!summaryOutput) {
    return;
  }
  summaryOutput.textContent = '';
  const ul = document.createElement('ul');
  for (const line of lines) {
    const li = document.createElement('li');
    li.textContent = line;
    ul.appendChild(li);
  }
  summaryOutput.appendChild(ul);
}

function renderTabs(tabs: Array<{title: string, url: string, minutesSinceLastActive: number}>) {
  if (!tabsOutput) {
    return;
  }
  tabsOutput.textContent = '';
  const ul = document.createElement('ul');
  for (const tab of tabs) {
    const li = document.createElement('li');
    li.textContent = `${tab.title || tab.url} (${tab.minutesSinceLastActive}m)`;
    ul.appendChild(li);
  }
  tabsOutput.appendChild(ul);
}

function renderStatus(message: string) {
  if (!statusOutput) {
    return;
  }
  statusOutput.textContent = message;
}

function renderCompareStatus(message: string) {
  if (!compareStatus) {
    return;
  }
  compareStatus.textContent = message;
}

function renderCompareNotes(lines: string[]) {
  if (!compareNotes) {
    return;
  }
  compareNotes.textContent = '';
  const ul = document.createElement('ul');
  for (const line of lines) {
    const li = document.createElement('li');
    li.textContent = line;
    ul.appendChild(li);
  }
  compareNotes.appendChild(ul);
}

function renderCompareTabs(tabs: Array<{index: number, title: string, url: string}>) {
  if (!compareTabList) {
    return;
  }
  compareTabList.textContent = '';
  for (const tab of tabs) {
    const label = document.createElement('label');
    label.style.display = 'block';
    const checkbox = document.createElement('input');
    checkbox.type = 'checkbox';
    checkbox.value = `${tab.index}`;
    label.appendChild(checkbox);
    const text = document.createElement('span');
    text.textContent = ` ${tab.title || tab.url}`;
    label.appendChild(text);
    compareTabList.appendChild(label);
  }
}

function renderAgentStatus(message: string) {
  if (!agentStatus) {
    return;
  }
  agentStatus.textContent = message;
}

function appendAgentLog(message: string) {
  if (!agentLog) {
    return;
  }
  const entry = document.createElement('div');
  entry.textContent = message;
  agentLog.appendChild(entry);
}

function renderAgentActions(actions: Array<{id: string, type: string, label: string, selector?: string, value?: string}>) {
  if (!agentActionList) {
    return;
  }
  agentActionList.textContent = '';
  for (const action of actions) {
    const row = document.createElement('div');
    row.style.marginBottom = '8px';

    const label = document.createElement('div');
    label.textContent = action.label;
    row.appendChild(label);

    const selectorInput = document.createElement('input');
    selectorInput.type = 'text';
    selectorInput.placeholder = 'selector';
    selectorInput.value = action.selector || '';
    if (action.type === 'focus') {
      selectorInput.disabled = true;
    }
    row.appendChild(selectorInput);

    const valueInput = document.createElement('input');
    valueInput.type = 'text';
    valueInput.placeholder = 'value';
    valueInput.value = action.value || '';
    if (action.type !== 'fill') {
      valueInput.disabled = true;
    }
    row.appendChild(valueInput);

    const button = document.createElement('button');
    button.textContent = 'Confirm';
    button.addEventListener('click', async () => {
      const payload = {
        id: action.id,
        type: action.type,
        selector: selectorInput.value,
        value: valueInput.value,
      };
      const response = await sendWithPromise('executeAiAgentAction', payload);
      if (response.error) {
        renderAgentStatus(response.error);
        return;
      }
      renderAgentStatus('');
      appendAgentLog(`${action.label}: ${response.success ? 'done' : 'failed'}`);
    });
    row.appendChild(button);

    agentActionList.appendChild(row);
  }
}

function renderSecurityStatus(message: string) {
  if (!securityStatus) {
    return;
  }
  securityStatus.textContent = message;
}

function renderSecuritySignals(signals: string[]) {
  if (!securitySignals) {
    return;
  }
  securitySignals.textContent = '';
  const ul = document.createElement('ul');
  for (const signal of signals) {
    const li = document.createElement('li');
    li.textContent = signal;
    ul.appendChild(li);
  }
  securitySignals.appendChild(ul);
}

function renderRemoteSettingsStatus(message: string) {
  if (!remoteSettingsStatus) {
    return;
  }
  remoteSettingsStatus.textContent = message;
}

function renderRemoteKeyState(message: string) {
  if (!remoteKeyState) {
    return;
  }
  remoteKeyState.textContent = message;
}

function renderRemotePreview(data: unknown) {
  if (!remotePreview) {
    return;
  }
  remotePreview.textContent = JSON.stringify(data, null, 2);
}

function renderRemotePreviewWarning(message: string) {
  if (!remotePreviewWarning) {
    return;
  }
  remotePreviewWarning.textContent = message;
}

function renderRemoteResult(message: string) {
  if (!remoteResult) {
    return;
  }
  remoteResult.textContent = message;
}

summarizeButton?.addEventListener('click', async () => {
  const response = await sendWithPromise('getAiLocalSummary');
  if (response.error) {
    renderStatus(response.error);
    renderSummary([]);
    renderTabs([]);
    return;
  }
  renderStatus('');
  renderSummary(response.summary || []);
  renderTabs(response.tabs || []);
});

async function loadCompareTabs() {
  const response = await sendWithPromise('getAiTabList');
  renderCompareTabs(response || []);
}

compareButton?.addEventListener('click', async () => {
  if (!compareTabList) {
    return;
  }
  const selected: number[] = [];
  const inputs =
      compareTabList.querySelectorAll<HTMLInputElement>('input[type=checkbox]');
  inputs.forEach(input => {
    if (input.checked) {
      selected.push(Number.parseInt(input.value, 10));
    }
  });
  if (selected.length < 2 || selected.length > 10) {
    renderCompareStatus('Select between 2 and 10 tabs to compare.');
    return;
  }
  const response = await sendWithPromise('compareAiTabs', selected);
  if (response.error) {
    renderCompareStatus(response.error);
    return;
  }
  renderCompareStatus('');
  renderCompareNotes(response.notes || []);
  if (compareMarkdown) {
    compareMarkdown.value = response.markdown || '';
  }
  if (compareCsv) {
    compareCsv.value = response.csv || '';
  }
});

loadCompareTabs();

async function loadAgentActions() {
  const response = await sendWithPromise('getAiAgentActions');
  renderAgentActions(response || []);
}

loadAgentActions();

async function refreshSecurityInsights() {
  const response = await sendWithPromise('getAiSecurityInsights');
  if (response.error) {
    renderSecurityStatus(response.error);
    renderSecuritySignals([]);
    return;
  }
  renderSecurityStatus('');
  renderSecuritySignals(response || []);
}

refreshSecurity?.addEventListener('click', refreshSecurityInsights);
refreshSecurityInsights();

async function loadRemoteSettings() {
  const response = await sendWithPromise('getAiRemoteModelSettings');
  if (response.error) {
    renderRemoteSettingsStatus(response.error);
    return;
  }
  if (remoteEndpoint) {
    remoteEndpoint.value = response.endpoint || '';
  }
  const hasKey = Boolean(response.hasKey);
  renderRemoteKeyState(
      hasKey ? 'Clé enregistrée.' : 'Aucune clé enregistrée.');
  renderRemoteSettingsStatus(
      response.allowRemote ?
          'Les requêtes distantes sont autorisées.' :
          'Les requêtes distantes sont désactivées dans les paramètres.');
}

saveRemoteSettings?.addEventListener('click', async () => {
  const payload: {endpoint?: string, apiKey?: string, clearKey?: boolean} = {};
  if (remoteEndpoint) {
    payload.endpoint = remoteEndpoint.value.trim();
  }
  if (remoteApiKey && remoteApiKey.value.trim()) {
    payload.apiKey = remoteApiKey.value.trim();
  }
  const response = await sendWithPromise('setAiRemoteModelSettings', payload);
  if (response.error) {
    renderRemoteSettingsStatus(response.error);
    return;
  }
  const hasKey = Boolean(response.hasKey);
  renderRemoteKeyState(
      hasKey ? 'Clé enregistrée.' : 'Aucune clé enregistrée.');
  renderRemoteSettingsStatus('Configuration enregistrée.');
  if (remoteApiKey) {
    remoteApiKey.value = '';
  }
});

clearRemoteKey?.addEventListener('click', async () => {
  const response = await sendWithPromise(
      'setAiRemoteModelSettings', {clearKey: true});
  if (response.error) {
    renderRemoteSettingsStatus(response.error);
    return;
  }
  renderRemoteKeyState('Aucune clé enregistrée.');
  renderRemoteSettingsStatus('Clé supprimée.');
});

remoteAnalyze?.addEventListener('click', async () => {
  renderRemoteResult('');
  const response = await sendWithPromise('getAiRemoteAnalysisPreview');
  if (response.error) {
    renderRemotePreviewWarning(response.error);
    renderRemotePreview({});
    if (remoteConfirm) {
      remoteConfirm.disabled = true;
    }
    return;
  }
  renderRemotePreviewWarning(
      'Avertissement: cette action envoie les données ci-dessous à un '
      + 'fournisseur externe. Vérifiez avant de confirmer.');
  renderRemotePreview(response);
  if (remoteConfirm) {
    remoteConfirm.disabled = false;
  }
});

remoteConfirm?.addEventListener('click', async () => {
  const response = await sendWithPromise('requestAiRemoteAnalysis');
  if (response.error) {
    renderRemoteResult(response.error);
    return;
  }
  renderRemoteResult(JSON.stringify(response, null, 2));
});

loadRemoteSettings();
