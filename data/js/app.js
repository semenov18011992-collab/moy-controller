// ============================================
// ГЛАВНЫЙ МОДУЛЬ APP
// ============================================

import { API } from './modules/api.js';
import { Dashboard } from './modules/dashboard.js';
import { Graphs } from './modules/graphs.js';
import { Settings } from './modules/settings.js';
import { UI } from './modules/ui.js';

class App {
    constructor() {
        // Инициализация модулей
        this.modules = {
            ui: new UI(),
            dashboard: new Dashboard(),
            graphs: new Graphs(),
            settings: new Settings()
        };
        
        this.currentPage = 'dashboard';
        this.init();
    }
    
    async init() {
        console.log('🚀 MOY Controller v4.0');
        console.log('📦 Modules:', Object.keys(this.modules));
        
        // Инициализация UI
        this.modules.ui.init();
        
        // Загрузка данных
        await this.modules.dashboard.load();
        await this.modules.settings.load();
        
        // Настройка навигации
        this.setupNavigation();
        
        // Автообновление
        setInterval(() => {
            this.modules.dashboard.update();
            this.modules.ui.updateTime();
        }, 3000);
        
        console.log('✅ System ready!');
    }
    
    setupNavigation() {
        const tabs = document.querySelectorAll('.tab');
        const container = document.getElementById('pageContainer');
        
        tabs.forEach(tab => {
            tab.addEventListener('click', () => {
                const page = tab.dataset.page;
                this.switchPage(page, tabs, container);
            });
        });
        
        // Показываем начальную страницу
        this.switchPage('dashboard', tabs, container);
    }
    
    switchPage(page, tabs, container) {
        // Обновляем активный таб
        tabs.forEach(t => t.classList.remove('active'));
        document.querySelector(`.tab[data-page="${page}"]`).classList.add('active');
        
        // Показываем страницу
        this.currentPage = page;
        const pages = {
            dashboard: () => this.modules.dashboard.render(),
            graphs: () => this.modules.graphs.render(),
            settings: () => this.modules.settings.render()
        };
        
        if (pages[page]) {
            container.innerHTML = pages[page]();
            // Инициализация после рендера
            if (page === 'graphs') {
                this.modules.graphs.initChart();
            }
            if (page === 'settings') {
                this.modules.settings.loadConfigs();
            }
        }
    }
}

// Запуск приложения
document.addEventListener('DOMContentLoaded', () => {
    window.app = new App();
});