// ============================================
// МОДУЛЬ ГРАФИКОВ
// ============================================

import { API } from './api.js';

export class Graphs {
    constructor() {
        this.chartInstance = null;
    }
    
    render() {
        return `
            <div class="graph-container">
                <div class="graph-card">
                    <div class="graph-title">📈 График</div>
                    <div style="height:300px;"><canvas id="tempChart"></canvas></div>
                    <div style="margin-top:10px; display:flex; gap:10px; flex-wrap:wrap;">
                        <select id="pinSelect"></select>
                        <button onclick="window.app.modules.graphs.loadHistory()" class="btn-save">🔄 Обновить</button>
                    </div>
                </div>
            </div>
        `;
    }
    
    async loadHistory() {
        const select = document.getElementById('pinSelect');
        if (!select || !select.value) return;
        
        const pinId = parseInt(select.value);
        try {
            const data = await API.getHistory(pinId, 50);
            this.updateChart(data, pinId);
        } catch (error) {
            console.error('Ошибка истории:', error);
        }
    }
    
    updateChart(data, pinId) {
        const canvas = document.getElementById('tempChart');
        if (!canvas) return;
        
        const ctx = canvas.getContext('2d');
        const labels = data.map(d => new Date(d.t * 1000).toLocaleTimeString());
        const values = data.map(d => d.v);
        
        if (this.chartInstance) {
            this.chartInstance.destroy();
        }
        
        let pinName = `Пин ${pinId}`;
        if (window.app && window.app.modules && window.app.modules.dashboard) {
            const pin = window.app.modules.dashboard.pins.find(p => p.id === pinId);
            if (pin) pinName = pin.name || pinName;
        }
        
        this.chartInstance = new Chart(ctx, {
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
        const select = document.getElementById('pinSelect');
        if (!select) return;
        
        select.innerHTML = '';
        if (window.app && window.app.modules && window.app.modules.dashboard) {
            const pins = window.app.modules.dashboard.pins;
            pins.forEach(pin => {
                const option = document.createElement('option');
                option.value = pin.id;
                option.textContent = pin.name || `Пин ${pin.id}`;
                select.appendChild(option);
            });
        }
        
        if (select.options.length > 0) {
            this.loadHistory();
        }
    }
}