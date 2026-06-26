# 🔥 MOY Controller v4.0

**Универсальный промышленный контроллер на базе ESP32-P4 / ESP32**

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.5.4-blue)](https://github.com/espressif/esp-idf)
[![Platform](https://img.shields.io/badge/platform-ESP32--P4-orange)](https://www.espressif.com/en/products/socs/esp32-p4)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

---

## 📌 Описание

MOY Controller — это **модульная SCADA-система** для управления котельными, промышленными процессами и умным домом. Контроллер построен на архитектуре **ядро + плагины**, что позволяет легко добавлять новые функции без переписывания существующего кода.

### 🎯 Ключевые возможности

- ✅ **Модульная архитектура** — добавляйте новые модули без изменения ядра
- ✅ **SCADA-интерфейс** — веб-API для мониторинга и управления
- ✅ **50+ GPIO** — входы/выходы с поддержкой PCF8574
- ✅ **Веб-сервер** — REST API на порту 80
- ✅ **Мультиплатформенность** — ESP32 / ESP32-S3 / ESP32-P4

---

## 📊 Текущий статус

| Компонент | Статус |
|-----------|--------|
| Ядро (core) | ✅ Работает |
| Модуль GPIO | ✅ Работает (50 пинов + PCF8574) |
| Модуль Web | ✅ Работает (веб-сервер на порту 80) |
| Модуль Test | ✅ Работает |
| Модуль Sensor | ✅ Работает (SCADA режим) |
| API `/api/pins` | ✅ Работает |
| API `/api/history` | ✅ Работает |
| API `/api/pin/set` | ✅ Работает |

---

## 🚀 Быстрый старт

### 1. Клонирование репозитория

```bash
git clone https://github.com/semenov18011992-collab/moy-controller.git
cd moy-controller

🏗️ Архитектура проекта
moy-controller/
├── main/
│   ├── include/              # Заголовочные файлы
│   │   ├── core.h            # Ядро системы
│   │   ├── module.h          # API модулей
│   │   ├── config.h          # Конфигурация
│   │   ├── platform.h        # Аппаратная абстракция
│   │   ├── webserver.h       # Веб-сервер
│   │   └── sensor_module.h   # Модуль датчиков
│   ├── src/
│   │   ├── core.c            # Ядро системы
│   │   ├── module.c          # Реестр модулей
│   │   ├── platform.c        # Определение платформы
│   │   ├── config.c          # Загрузка конфигурации
│   │   ├── webserver.c       # Веб-сервер + API
│   │   ├── sensor_module.c   # SCADA модуль
│   │   └── modules/
│   │       ├── module_gpio.c # GPIO драйвер
│   │       ├── module_web.c  # Веб-модуль
│   │       └── module_test.c # Тестовый модуль
│   └── main.c                # Точка входа
├── data/                     # Веб-файлы
├── CMakeLists.txt
├── sdkconfig.defaults
└── partitions.csv
