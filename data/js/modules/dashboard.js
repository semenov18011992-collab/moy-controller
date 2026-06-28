// ============================================
// МОДУЛЬ ДАШБОРДА
// ============================================

import { API } from './api.js';

export class Dashboard {
    constructor() {
        this.pins = [];
        this.groups = {
            sensor: { title: '🌡️ Датчики', pins: [] },
            actuator: { title: '⚡ Выходы', pins: [] },
            input: { title: '🔌 Входы', pins: [] }
        };
    }
    
    async load() {
        try {
            const data = await API.getPins();
            this.pins = data.pins || [];
            this.groupPins();
            return true;
        } catch (error) {
            console.error('Ошибка загрузки пинов:', error);
            return false;
        }
    }
    
    async update() {
        await this.load();
        const container = document.getElementById('pinsContainer');
        if (container) {
            container.innerHTML = this.renderPins();
            this.updateStats();
        }
    }
    
    groupPins() {
        Object.keys(this.groups).forEach(key => {
            this.groups[key].pins = [];
        });
        
        this.pins.forEach(pin => {
            const type = pin.type || 'sensor';
            if (this.groups[type]) {
                this.groups[type].pins.push(pin);
            }
        });
    }
    
    render() {
        return `
            <div id="statsBar"></div>
            <div id="pinsContainer">${this.renderPins()}</div>
        `;
    }
    
    renderPins() {
        let html = '';
        let outputsOn = 0;
        
        Object.keys(this.groups).forEach(groupKey => {
            const group = this.groups[groupKey];
            if (group.pins.length === 0) return;
            
            html += `<div class="group-title">${group.title}</div>`;
            html += `<div class="pins-grid">`;
            
            group.pins.forEach(pin => {
                const isActuator = pin.is_output === true;
                const value = pin.value || 0;
                const isOn = value > 0.5;
                if (isActuator && isOn) outputsOn++;
                
                let valueClass = pin.type || 'sensor';
                let displayValue = `${value.toFixed(1)}°C`;
                let controls = '';
                let typeLabel = 'Датчик';
                
                if (isActuator) {
                    valueClass = isOn ? 'actuator on' : 'actuator off';
                    displayValue = isOn ? '🟢 ВКЛ' : '🔴 ВЫКЛ';
                    typeLabel = 'Выход';
                    controls = `
                        <button class="${isOn ? 'btn-off' : 'btn-on'}"
                                onclick="window.app.modules.dashboard.setPin(${pin.id}, ${isOn ? 0 : 1})">
                            ${isOn ? '🔴 Выкл' : '🟢 Вкл'}
                        </button>
                    `;
                } else if (pin.type === 'input') {
                    displayValue = value > 0.5 ? '🔴 АКТИВЕН' : '⚪ НОРМА';
                    valueClass = value > 0.5 ? 'input alarm' : 'input';
                    typeLabel = 'Вход';
                } else {
                    typeLabel = 'Датчик';
                    if (value > 50 || value < -10) valueClass += ' alarm';
                }
                
                html += `
                    <div class="pin-card">
                        <div class="pin-name">${pin.name || `Пин ${pin.id}`}</div>
                        <div class="pin-value ${valueClass}">${displayValue}</div>
                        <div class="pin-type ${pin.type}">${typeLabel} • GPIO ${pin.id}</div>
                        <div class="pin-actions">${controls}</div>
                    </div>
                `;
            });
            
            html += `</div>`;
        });
        
        // Обновляем счётчик
        setTimeout(() => {
            const el = document.getElementById('outputsOn');
            if (el) el.textContent = outputsOn;
        }, 0);
        
        return html;
    }
    
    updateStats() {
        const statsBar = document.getElementById('statsBar');
        if (!statsBar) return;
        
        const total = this.pins.length;
        const sensors = this.pins.filter(p => p.type === 'sensor').length;
        const actuators = this.pins.filter(p => p.type === 'actuator' || p.is_output).length;
        const inputs = this.pins.filter(p => p.type === 'input').length;
        
        statsBar.innerHTML = `
            <div class="stats-bar">
                <div class="stat-item">Всего пинов: <span>${total}</span></div>
                <div class="stat-item">Датчики: <span>${sensors}</span></div>
                <div class="stat-item">Выходы: <span>${actuators}</span></div>
                <div class="stat-item">Входы: <span>${inputs}</span></div>
                <div class="stat-item">Выходы включены: <span id="outputsOn">0</span></div>
            </div>
        `;
    }
    
    async setPin(pinId, value) {
        try {
            const result = await API.setPin(pinId, value);
            if (result.success) {
                const pin = this.pins.find(p => p.id === pinId);
                if (pin) {
                    pin.value = value;
                    this.update();
                }
            } else {
                alert('Ошибка: ' + (result.error || 'неизвестная'));
            }
        } catch (error) {
            console.error('Ошибка управления:', error);
            alert('Ошибка связи с контроллером');
        }
    }
}