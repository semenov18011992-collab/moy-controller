// ============================================
// API МОДУЛЬ
// ============================================

export const API = {
    async request(endpoint, options = {}) {
        try {
            const response = await fetch(endpoint, {
                ...options,
                headers: {
                    'Content-Type': 'application/json',
                    ...options.headers
                }
            });
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}`);
            }
            return await response.json();
        } catch (error) {
            console.error(`API Error ${endpoint}:`, error);
            throw error;
        }
    },
    
    // === ПИНЫ ===
    getPins() {
        return this.request('/api/pins');
    },
    
    setPin(pin, value) {
        return this.request(`/api/pin/set?pin=${pin}&value=${value}`);
    },
    
    // === КОНФИГУРАЦИЯ ===
    getConfigs() {
        return this.request('/api/pins/config/all');
    },
    
    saveConfig(config) {
        return this.request('/api/pin/config', {
            method: 'POST',
            body: JSON.stringify(config)
        });
    },
    
    deleteConfig(pin) {
        return this.request(`/api/pin/delete?pin=${pin}`, {
            method: 'DELETE'
        });
    },
    
    // === ИСТОРИЯ ===
    getHistory(pin, points = 50) {
        return this.request(`/api/history?pin=${pin}&points=${points}`);
    },
    
    // === СИСТЕМА ===
    getSystem() {
        return this.request('/api/system');
    },
    
    reset() {
        return this.request('/api/reset', { method: 'POST' });
    }
};