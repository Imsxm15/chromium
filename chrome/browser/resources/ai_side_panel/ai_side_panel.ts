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
