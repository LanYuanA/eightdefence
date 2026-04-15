import re

with open('dashboard.html', 'r', encoding='utf-8') as f:
    text = f.read()

replacements = [
    (r'<div class="data-value" style="color: #38bdf8;">24\.5<span class="data-unit">℃</span> / 56\.2<span class="data-unit">%RH</span></div>', r'<div class="data-value" style="color: #38bdf8;"><span id="val-temp">24.5 ℃</span> / <span id="val-hum">56.2 %</span></div>'),
    (r'<div class="data-value" id="val-placeholder">12<span class="data-unit">μg/m³</span></div>', r'<div class="data-value" id="val-pm25">12<span class="data-unit">μg/m³</span></div>'),
    (r'<div class="data-value" id="val-placeholder">25<span class="data-unit">μg/m³</span></div>', r'<div class="data-value" id="val-pm10">25<span class="data-unit">μg/m³</span></div>'),
    (r'<div class="data-value" style="color: var\(--safe-green\);">0\.03<span class="data-unit">ppm</span></div>', r'<div class="data-value" id="val-ch2o" style="color: var(--safe-green);">0.03<span class="data-unit">ppm</span></div>'),
    (r'<div class="data-value" id="val-placeholder">450<span class="data-unit">ppm</span></div>', r'<div class="data-value" id="val-co2">450<span class="data-unit">ppm</span></div>'),
    (r'<div class="data-value" id="val-placeholder">0\.01<span class="data-unit">ppm</span></div>', r'<div class="data-value" id="val-o3">0.01<span class="data-unit">ppm</span></div>'),
    (r'<div class="data-value" id="val-placeholder">15<span class="data-unit">ppb</span></div>', r'<div class="data-value" id="val-tvoc">15<span class="data-unit">ppb</span></div>'),
    (r'<div class="data-value" style="color: var\(--safe-green\);">正常工作</div>', r'<div class="data-value" id="val-smoke" style="color: var(--safe-green);">正常工作</div>'),
    (r'<div class="data-value" id="val-placeholder">55<span class="data-unit">℃</span></div>', r'<div class="data-value" id="val-smoke-temp">55<span class="data-unit">℃</span></div>'),
    (r'<div class="data-value" style="color: var\(--alert-red\);">检测到积水!</div>', r'<div class="data-value" id="val-water" style="color: var(--alert-red);">检测到积水!</div>'),
    (r'<div class="data-value" style="color: var\(--warn-yellow\);">探测到人体游荡</div>', r'<div class="data-value" id="val-ir-radar" style="color: var(--warn-yellow);">探测到人体游荡</div>'),
    (r'<div class="data-value" id="val-placeholder">触发</div>', r'<div class="data-value" id="val-ir">触发</div>'),
    (r'<div class="data-value" style="color: var\(--text-muted\);">未触发</div>', r'<div class="data-value" id="val-radar" style="color: var(--text-muted);">未触发</div>'),
    (r'<div class="data-value" style="color: #fbbf24;">307<span class="data-unit"> lux</span></div>', r'<div class="data-value" id="val-lux" style="color: #fbbf24;">307<span class="data-unit"> lux</span></div>'),
]

for old, new in replacements:
    text = re.sub(old, new, text)

with open('dashboard.html', 'w', encoding='utf-8') as f:
    f.write(text)
