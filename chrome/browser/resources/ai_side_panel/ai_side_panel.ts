// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {sendWithPromise} from 'chrome://resources/js/cr.m.js';

const summarizeButton =
    document.querySelector<HTMLButtonElement>('#summarizeTabs');
const summaryOutput = document.querySelector<HTMLDivElement>('#summaryOutput');
const tabsOutput = document.querySelector<HTMLDivElement>('#tabsOutput');

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

summarizeButton?.addEventListener('click', async () => {
  const response = await sendWithPromise('getAiLocalSummary');
  renderSummary(response.summary || []);
  renderTabs(response.tabs || []);
});
