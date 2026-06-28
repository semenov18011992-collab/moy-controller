// ============================================
// МОДУЛЬ НАСТРОЕК
// ============================================

import { API } from './api.js';

export class Settings {
    constructor() {
        this.configs = [];
    }
    
    async load() {
        try {
            const data = await API.getConfigs();
            this.configs = data.configs || [];
        } catch (error) {
            console.warn('No configs loaded');
        }
    }
    
    render() {
        return `
            <div class="settings-card">
                <h3>⚙️ Системная информация</h3>
                <div class="info-row">
                    <span class="label">Платформа:</span>
                    <span class="value">ESP32</span>
                </div>
                <div class="info-row">
                    <span class="label">Прошивка:</span>
                    <span class="value">v2.0-scada-full</span>
                </div>
                <div class="info-row">
                    <span class="label">Wi-Fi IP:</span>
                    <span class="value" id="wifiIP">${window.location.hostname}</span>
                </div>
                <div class="info-row">
                    <span class="label">Свободная RAM:</span>
                    <span class="value" id="freeRam">--</span>
                </div>
                <div style="margin-top:15px;">
                    <button onclick="location.reload()" class="btn-save">🔄 Перезагрузить</button>
                    <button onclick="window.app.modules.settings.resetController()" style="background:#ff4444;color:#fff;border:none;padding:8px 20px;border-radius:6px;cursor:pointer;margin-left:10px;">
                        ⚠️ Сбросить
                    </button>
                </div>
            </div>
            
            <div class="settings-card">
                <h3>🔧 Конфигурация пинов</h3>
                <div style="margin-bottom:10px;">
                    <button onclick="window.app.modules.settings.loadConfigs()" class="btn-refresh">🔄 Загрузить</button>
                    <button onclick="window.app.modules.settings.saveAll()" style="background:#00ff88;color:#0a0e17;border:none;padding:8px 20px;border-radius:6px;cursor:pointer;margin-left:10px;">
                        💾 Сохранить все
                    </button>
                    <span id="configStatus" class="status-msg"></span>
                </div>
                <div id="pinConfigContainer"></div>
            </div>
        `;
    }
    
    async loadConfigs() {
        const status = document.getElementById('configStatus');
        status.textContent = '⏳ Загрузка...';
        
        try {
            const data = await API.getConfigs();
            this.configs = data.configs || [];
            this.renderConfigs();
            status.textContent = `✅ Загружено ${this.configs.length} конфигураций`;
        } catch (error) {
            status.textContent = '❌ Ошибка загрузки';
            console.error(error);
        }
    }
    
    renderConfigs() {
        const container = document.getElementById('pinConfigContainer');
        if (!container) return;
        
        if (this.configs.length === 0) {
            container.innerHTML = '<div style="color:#556688;padding:20px;text-align:center;">Нет сохранённых конфигураций</div>';
            return;
        }
        
        let html = '';
        this.configs.forEach(cfg => {
            html += `
                <div class="config-grid">
                    <span class="label">GPIO ${cfg.pin}</span>
                    <input type="text" value="${cfg.name}" data-pin="${cfg.pin}" class="cfg-name" placeholder="Имя пина">
                    <select data-pin="${cfg.pin}" class="cfg-type">
                        <option value="sensor" ${cfg.type === 'sensor' ? 'selected' : ''}>🌡️ Датчик</option>
                        <option value="actuator" ${cfg.type === 'actuator' ? 'selected' : ''}>⚡ Выход</option>
                        <option value="input" ${cfg.type === 'input' ? 'selected' : ''}>🔌 Вход</option>
                        <option value="disabled" ${cfg.type === 'disabled' ? 'selected' : ''}>⛔ Отключен</option>
                    </select>
                    <select data-pin="${cfg.pin}" class="cfg-mode">
                        <option value="digital" ${cfg.mode === 'digital' ? 'selected' : ''}>Цифровой</option>
                        <option value="analog" ${cfg.mode === 'analog' ? 'selected' : ''}>Аналоговый</option>
                        <option value="onewire" ${cfg.mode === 'onewire' ? 'selected' : ''}>OneWire</option>
                    </select>
                    <div>
                        <button onclick="window.app.modules.settings.saveSingle(${cfg.pin})" class="btn-save">💾 Сохранить</button>
                        <button onclick="window.app.modules.settings.deletePin(${cfg.pin})" class="btn-delete">🗑️</button>
                    </div>
                </div>
            `;
        });
        
        container.innerHTML = html;
    }
    
    async saveSingle(pin) {
        const status = document.getElementById('configStatus');
        status.textContent = '⏳ Сохранение...';
        
        const nameInput = document.querySelector(`.cfg-name[data-pin="${pin}"]`);
        const typeSelect = document.querySelector(`.cfg-type[data-pin="${pin}"]`);
        const modeSelect = document.querySelector(`.cfg-mode[data-pin="${pin}"]`);
        
        if (!nameInput) {
            status.textContent = '❌ Ошибка: элемент не найден';
            return;
        }
        
        const config = {
            pin: pin,
            name: nameInput.value || `GPIO_${pin}`,
            type: typeSelect ? typeSelect.value : 'sensor',
            mode: modeSelect ? modeSelect.value : 'digital',
            enabled: typeSelect ? typeSelect.value !== 'disabled' : true
        };
        
        try {
            await API.saveConfig(config);
            status.textContent = `✅ Пин ${pin} сохранён`;
            this.loadConfigs();
        } catch (error) {
            status.textContent = '❌ Ошибка сохранения';
            console.error(error);
        }
    }
    
    async saveAll() {
        const status = document.getElementById('configStatus');
        status.textContent = '⏳ Сохранение всех...';
        
        const inputs = document.querySelectorAll('.cfg-name');
        const types = document.querySelectorAll('.cfg-type');
        const modes = document.querySelectorAll('.cfg-mode');
        
        let saved = 0;
        for (let i = 0; i < inputs.length; i++) {
            const pin = parseInt(inputs[i].dataset.pin);
            const config = {
                pin: pin,
                name: inputs[i].value || `GPIO_${pin}`,
                type: types[i] ? types[i].value : 'sensor',
                mode: modes[i] ? modes[i].value : 'digital',
                enabled: types[i] ? types[i].value !== 'disabled' : true
            };
            
            try {
                await API.saveConfig(config);
                saved++;
            } catch (error) {
                console.error(`Error saving pin ${pin}:`, error);
            }
        }
        
        status.textContent = `✅ Сохранено: ${saved}`;
        this.loadConfigs();
    }
    
    async deletePin(pin) {
        if (!confirm(`Удалить конфигурацию пина ${pin}?`)) return;
        
        const status = document.getElementById('configStatus');
        status.textContent = '⏳ Удаление...';
        
        try {
            await API.deleteConfig(pin);
            status.textContent = `✅ Пин ${pin} удалён`;
            this.loadConfigs();
        } catch (error) {
            status.textContent = '❌ Ошибка удаления';
            console.error(error);
        }
    }
    
    async resetController() {
        if (confirm('Сбросить контроллер?')) {
            try {
                await API.reset();
                alert('Контроллер перезагружается...');
                setTimeout(() => location.reload(), 3000);
            } catch (error) {
                alert('Ошибка сброса');
            }
        }
    }
}