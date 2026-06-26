// === ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ===
let pinsData = [];
let chartInstance = null;
let updateInterval = null;

// === ЗАГРУЗКА ПИНОВ ===
async function loadPins() {
    try {
        const response = await fetch('/api/pins');
        if (!response.ok) throw new Error('Ошибка сети');
        const data = await response.json();
        pinsData = data.pins || [];
        renderPins();
        updateStats();
        fillPinSelect();
        return true;
    } catch (e) {
        console.error('Ошибка загрузки:', e);
        document.getElementById('pinsGrid').innerHTML =
            '<div style="color:#ff4444;padding:40px;text-align:center;">❌ Ошибка загрузки данных</div>';
        return false;
    }
}

// === ОТРИСОВКА ПИНОВ ===
function renderPins() {
    const grid = document.getElementById('pinsGrid');
    grid.innerHTML = '';
    let outputsOn = 0;

    pinsData.forEach(pin => {
        const isActuator = pin.id >= 26 && pin.id <= 33;
        const value = pin.value || 0;
        const isOn = value > 0.5;
        if (isActuator && isOn) outputsOn++;

        const card = document.createElement('div');
        card.className = 'pin-card';
        card.id = `pin-${pin.id}`;

        let valueClass = 'sensor';
        let displayValue = `${value.toFixed(1)}°C`;
        let controls = '';

        if (isActuator) {
            valueClass = isOn ? 'actuator on' : 'actuator off';
            displayValue = isOn ? 'ВКЛ' : 'ВЫКЛ';
            controls = `
                <button class="${isOn ? 'btn-off' : 'btn-on'}"
                        onclick="setPin(${pin.id}, ${isOn ? 0 : 1})">
                    ${isOn ? '🔴 Выкл' : '🟢 Вкл'}
                </button>
            `;
        } else {
            // Для датчиков — авария при >50
            if (value > 50) valueClass += ' alarm';
        }

        card.innerHTML = `
            <div class="pin-name">${pin.name || `Пин ${pin.id}`}</div>
            <div class="pin-value ${valueClass}">${displayValue}</div>
            <div class="pin-id">GPIO ${pin.id} ${pin.online ? '🟢' : '🔴'}</div>
            <div class="pin-actions">${controls}</div>
        `;
        grid.appendChild(card);
    });

    document.getElementById('outputsOn').textContent = outputsOn;
}

// === УПРАВЛЕНИЕ ПИНОМ ===
async function setPin(pinId, value) {
    try {
        const response = await fetch(`/api/pin/set?pin=${pinId}&value=${value}`);
        const result = await response.json();
        if (result.success) {
            const pin = pinsData.find(p => p.id === pinId);
            if (pin) {
                pin.value = value;
                renderPins();
                updateStats();
            }
        } else {
            alert('Ошибка: ' + (result.error || 'неизвестная ошибка'));
        }
    } catch (e) {
        console.error('Ошибка управления:', e);
        alert('Ошибка связи с контроллером');
    }
}

// === СТАТИСТИКА ===
function updateStats() {
    const total = pinsData.length;
    const active = pinsData.filter(p => p.online).length;
    document.getElementById('totalPins').textContent = total;
    document.getElementById('activePins').textContent = active;
}

// === ЗАПОЛНЕНИЕ ВЫБОРА ДЛЯ ГРАФИКА ===
function fillPinSelect() {
    const select = document.getElementById('pinSelect');
    select.innerHTML = '';
    pinsData.forEach(pin => {
        const option = document.createElement('option');
        option.value = pin.id;
        option.textContent = `${pin.name || `Пин ${pin.id}`}`;
        select.appendChild(option);
    });
    if (pinsData.length > 0) {
        loadHistory();
    }
}

// === ЗАГРУЗКА ИСТОРИИ ===
async function loadHistory() {
    const pinId = document.getElementById('pinSelect').value;
    if (!pinId) return;
    try {
        const response = await fetch(`/api/history?pin=${pinId}&points=50`);
        if (!response.ok) throw new Error('Ошибка загрузки');
        const data = await response.json();
        updateChart(data, pinId);
    } catch (e) {
        console.error('Ошибка истории:', e);
    }
}

// === ГРАФИК ===
function updateChart(data, pinId) {
    const ctx = document.getElementById('tempChart').getContext('2d');
    const labels = data.map(d => new Date(d.t * 1000).toLocaleTimeString());
    const values = data.map(d => d.v);

    if (chartInstance) chartInstance.destroy();

    const pin = pinsData.find(p => p.id == pinId);
    const isActuator = pin && pin.id >= 26 && pin.id <= 33;
    const color = isActuator ? '#ffaa44' : '#00d4ff';

    chartInstance = new Chart(ctx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: [{
                label: pin ? pin.name : `Пин ${pinId}`,
                data: values,
                borderColor: color,
                backgroundColor: color + '20',
                fill: true,
                tension: 0.4,
                pointRadius: 3,
                pointBackgroundColor: color
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: { labels: { color: '#8899bb' } }
            },
            scales: {
                x: { grid: { color: '#1a2a3a' }, ticks: { color: '#556688' } },
                y: { grid: { color: '#1a2a3a' }, ticks: { color: '#556688' } }
            }
        }
    });
}

// === СБРОС КОНТРОЛЛЕРА ===
async function resetController() {
    if (confirm('Сбросить контроллер?')) {
        try {
            await fetch('/api/reset', { method: 'POST' });
            alert('Контроллер перезагружается...');
            setTimeout(() => location.reload(), 3000);
        } catch (e) {
            alert('Ошибка сброса');
        }
    }
}

// === ТАБЫ ===
document.querySelectorAll('.tab').forEach(tab => {
    tab.addEventListener('click', function() {
        document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
        document.querySelectorAll('.tab-content').forEach(t => t.classList.remove('active'));
        this.classList.add('active');
        document.getElementById(`tab-${this.dataset.tab}`).classList.add('active');
        if (this.dataset.tab === 'graphs' && chartInstance) {
            setTimeout(() => chartInstance.resize(), 100);
        }
    });
});

// === ОБНОВЛЕНИЕ ВРЕМЕНИ ===
function updateTime() {
    document.getElementById('lastUpdate').textContent = `Обновление: ${new Date().toLocaleTimeString()}`;
}

// === ИНИЦИАЛИЗАЦИЯ ===
async function init() {
    await loadPins();
    updateTime();
    setInterval(() => {
        loadPins();
        updateTime();
    }, 3000);
}

document.addEventListener('DOMContentLoaded', init);