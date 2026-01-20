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
  const inputs = compareTabList.querySelectorAll<HTMLInputElement>('input[type=checkbox]');
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
