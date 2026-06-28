// ============================================
// UI МОДУЛЬ
// ============================================

export class UI {
    init() {
        this.updateTime();
        this.showIP();
        this.loadSystemInfo();
    }
    
    updateTime() {
        const el = document.getElementById('timeDisplay');
        if (el) {
            el.textContent = new Date().toLocaleTimeString();
        }
    }
    
    showIP() {
        const el = document.getElementById('wifiIP');
        if (el) {
            el.textContent = window.location.hostname;
        }
    }
    
    async loadSystemInfo() {
        try {
            const response = await fetch('/api/system');
            const data = await response.json();
            const ramEl = document.getElementById('freeRam');
            if (ramEl) {
                ramEl.textContent = (data.free_heap / 1024).toFixed(0) + ' KB';
            }
        } catch (error) {
            console.warn('System info not available');
        }
    }
    
    setStatus(text, isOnline = true) {
        const el = document.getElementById('statusText');
        if (el) el.textContent = text;
        
        const dot = document.querySelector('.dot');
        if (dot) {
            dot.style.background = isOnline ? '#00ff88' : '#ff4444';
        }
    }
}