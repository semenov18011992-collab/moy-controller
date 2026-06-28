// ============================================
// МОДУЛЬ НАСТРОЕК
// ============================================

import { API } from './api.js';

export class ConfigModule {
    constructor() {
        this.configs = [];
    }
    
    render() {
        return `
            <div class="settings-card">
                <h3>⚙️ Системная информация</h3>
                <div class="info-row"><span class="label">Платформа:</span><span class="value">ESP32</span></div>
                <div class="info-row"><span class="label">Прошивка:</span><span class="value">v2.0-scada-full</span></div>
                <div class="info-row"><span class="label">Wi-Fi IP:</span><span class="value" id="wifiIP">${window.location.hostname}</span></div>
                <div class="info-row"><span class="label">Свободная RAM:</span><span class="value" id="freeRam">--</span></div>
                <div style="margin-top:15px;">
                    <button onclick="window.location.reload()">🔄 Перезагрузить</button>
                    <button onclick="window.app.modules.config.resetController()" style="background:#ff4444;">⚠️ Сбросить</button>
                </div>
            </div>
            
            <div class="settings-card" style="margin-top:20px;">
                <h3>🔧 Конфигурация пинов</h3>
                <div style="margin-bottom:10px;">
                    <button onclick="window.app.modules.config.loadConfigs()" class="btn-save">🔄 Загрузить</button>
                    <button onclick="window.app.modules.config.saveAll()" style="background:#00d4ff;color:#0a0e17;border:none;padding:8px 20px;border-radius:6px;cursor:pointer;">💾 Сохранить все</button>
                    <span id="configStatus" style="margin-left:15px;color:#556688;font-size:14px;"></span>
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
        } catch (e) {
            status.textContent = '❌ Ошибка загрузки';
            console.error(e);
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
                    <input type="text" value="${cfg.name}" data-pin="${cfg.pin}" placeholder="Имя пина" class="cfg-name">
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
                        <button onclick="window.app.modules.config.saveSingle(${cfg.pin})" class="btn-save">💾 Сохранить</button>
                        <button onclick="window.app.modules.config.deletePin(${cfg.pin})" class="btn-delete">🗑️</button>
                    </div>
                </div>
            `;
        });
        
        container.innerHTML = html;
    }
    
    async load() {
        try {
            const data = await API.getConfigs();
            this.configs = data.configs || [];
        } catch (e) {
            console.warn('No configs loaded');
        }
    }
    
    async saveSingle(pin) {
        const status = document.getElementById('configStatus');
        status.textContent = '⏳ Сохранение...';
        
        const nameInput = document.querySelector(`.cfg-name[data-pin="${pin}"]`);
        const typeSelect = document.querySelector(`.cfg-type[data-pin="${pin}"]`);
        const modeSelect = document.querySelector(`.cfg-mode[data-pin="${pin}"]`);
        
        if (!nameInput) return;
        
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
            await this.loadConfigs();
        } catch (e) {
            status.textContent = '❌ Ошибка сохранения';
            console.error(e);
        }
    }
    
    async saveAll() {
        const status = document.getElementById('configStatus');
        status.textContent = '⏳ Сохранение всех...';
        
        const nameInputs = document.querySelectorAll('.cfg-name');
        const typeSelects = document.querySelectorAll('.cfg-type');
        const modeSelects = document.querySelectorAll('.cfg-mode');
        
        let saved = 0;
        
        for (let i = 0; i < nameInputs.length; i++) {
            const pin = parseInt(nameInputs[i].dataset.pin);
            const config = {
                pin: pin,
                name: nameInputs[i].value || `GPIO_${pin}`,
                type: typeSelects[i] ? typeSelects[i].value : 'sensor',
                mode: modeSelects[i] ? modeSelects[i].value : 'digital',
                enabled: typeSelects[i] ? typeSelects[i].value !== 'disabled' : true
            };
            
            try {
                await API.saveConfig(config);
                saved++;
            } catch (e) {
                console.error(`Error saving pin ${pin}:`, e);
            }
        }
        
        status.textContent = `✅ Сохранено: ${saved}`;
        await this.loadConfigs();
    }
    
    async deletePin(pin) {
        if (!confirm(`Удалить конфигурацию пина ${pin}?`)) return;
        
        const status = document.getElementById('configStatus');
        status.textContent = '⏳ Удаление...';
        
        try {
            await API.deleteConfig(pin);
            status.textContent = `✅ Пин ${pin} удалён`;
            await this.loadConfigs();
        } catch (e) {
            status.textContent = '❌ Ошибка удаления';
            console.error(e);
        }
    }
    
    async resetController() {
        if (confirm('Сбросить контроллер?')) {
            try {
                await API.reset();
                alert('Контроллер перезагружается...');
                setTimeout(() => location.reload(), 3000);
            } catch (e) {
                alert('Ошибка сброса');
            }
        }
    }
}