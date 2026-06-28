// ============================================
// МОДУЛЬ ГРАФИКОВ
// ============================================

import { API } from './api.js';

export class ChartsModule {
    constructor() {
        this.chartInstance = null;
        this.pins = [];
    }
    
    render() {
        return `
            <div class="graph-container">
                <div class="graph-card">
                    <div class="graph-title">📈 Температура</div>
                    <div style="height:300px;"><canvas id="tempChart"></canvas></div>
                    <div style="margin-top:10px; display:flex; gap:10px; flex-wrap:wrap;">
                        <select id="pinSelect"></select>
                        <button onclick="window.app.modules.charts.loadHistory()" class="btn-save">🔄 Обновить</button>
                    </div>
                </div>
            </div>
        `;
    }
    
    async loadHistory() {
        const select = document.getElementById('pinSelect');
        if (!select) return;
        
        const pinId = select.value;
        if (!pinId) return;
        
        try {
            const data = await API.getHistory(pinId, 50);
            this.updateChart(data, pinId);
        } catch (e) {
            console.error('Ошибка истории:', e);
        }
    }
    
    updateChart(data, pinId) {
        const ctx = document.getElementById('tempChart');
        if (!ctx) return;
        
        const canvas = ctx.getContext('2d');
        const labels = data.map(d => new Date(d.t * 1000).toLocaleTimeString());
        const values = data.map(d => d.v);
        
        if (this.chartInstance) {
            this.chartInstance.destroy();
        }
        
        // Получаем имя пина
        let pinName = `Пин ${pinId}`;
        if (window.app && window.app.modules && window.app.modules.pins) {
            const pin = window.app.modules.pins.pins.find(p => p.id == pinId);
            if (pin) pinName = pin.name || pinName;
        }
        
        this.chartInstance = new Chart(canvas, {
            type: 'line',
            data: {
                labels: labels,
                datasets: [{
                    label: pinName,
                    data: values,
                    borderColor: '#00d4ff',
                    backgroundColor: '#00d4ff20',
                    fill: true,
                    tension: 0.4,
                    pointRadius: 3,
                    pointBackgroundColor: '#00d4ff'
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
    
    initChart() {
        // Заполняем селект пинов
        const select = document.getElementById('pinSelect');
        if (!select) return;
        
        select.innerHTML = '';
        if (window.app && window.app.modules && window.app.modules.pins) {
            const pins = window.app.modules.pins.pins;
            pins.forEach(pin => {
                const option = document.createElement('option');
                option.value = pin.id;
                option.textContent = pin.name || `Пин ${pin.id}`;
                select.appendChild(option);
            });
        }
        
        // Загружаем историю для первого пина
        if (select.options.length > 0) {
            this.loadHistory();
        }
    }
}