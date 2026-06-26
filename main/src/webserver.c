#include "module.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include <string.h>
#include "sensor_module.h"

#define TAG "WEBSERVER"

static httpd_handle_t server = NULL;

// Главная HTML-страница (встроена в код)
static const char* index_html = 
"<!DOCTYPE html>"
"<html lang='ru'>"
"<head>"
"<meta charset='UTF-8'>"
"<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
"<title>MOY Controller SCADA</title>"
"<script src='https://cdn.jsdelivr.net/npm/chart.js@4.4.0/dist/chart.umd.min.js'></script>"
"<style>"
"*{margin:0;padding:0;box-sizing:border-box}"
"body{font-family:'Segoe UI',sans-serif;background:#0a0e17;color:#e0e0e0;padding:20px}"
".header{background:linear-gradient(135deg,#0f3460,#16213e);padding:20px;border-radius:12px;margin-bottom:20px;border:1px solid #1a4a7a;display:flex;justify-content:space-between;align-items:center}"
".header h1{color:#00d4ff;font-size:28px;font-weight:300}"
".dot{display:inline-block;width:10px;height:10px;background:#00ff88;border-radius:50%;margin-right:8px;animation:pulse 1.5s infinite}"
"@keyframes pulse{0%,100%{opacity:1}50%{opacity:0.3}}"
".tabs{display:flex;gap:10px;margin-bottom:20px;flex-wrap:wrap}"
".tab{padding:10px 25px;background:#16213e;border:1px solid #1f3a5f;border-radius:8px;cursor:pointer;transition:all .3s;color:#8899bb;font-size:14px}"
".tab:hover{border-color:#00d4ff;color:#fff}"
".tab.active{background:#0f3460;border-color:#00d4ff;color:#00d4ff}"
".tab-content{display:none}"
".tab-content.active{display:block}"
".stats-bar{display:flex;gap:20px;flex-wrap:wrap;margin-bottom:20px;padding:10px 0}"
".stat-item{background:#1a1a2e;padding:8px 20px;border-radius:20px;font-size:14px;color:#8899bb}"
".stat-item span{color:#fff;font-weight:bold}"
".pins-grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(200px,1fr));gap:15px}"
".pin-card{background:#16213e;border-radius:12px;padding:15px 20px;border:1px solid #1f3a5f;transition:all .3s}"
".pin-card:hover{border-color:#00d4ff}"
".pin-card .pin-name{font-size:12px;color:#8899bb;text-transform:uppercase;letter-spacing:1px}"
".pin-card .pin-value{font-size:28px;font-weight:bold;margin:5px 0}"
".pin-card .pin-value.sensor{color:#00d4ff}"
".pin-card .pin-value.actuator{color:#ffaa44}"
".pin-card .pin-value.on{color:#00ff88}"
".pin-card .pin-value.off{color:#556688}"
".pin-card .pin-value.alarm{color:#ff4444;animation:blink 1s infinite}"
"@keyframes blink{0%,100%{opacity:1}50%{opacity:.2}}"
".pin-card .pin-id{font-size:11px;color:#445566;margin-top:5px}"
".pin-card .pin-actions{margin-top:10px;display:flex;gap:5px}"
".pin-card .pin-actions button{padding:5px 15px;border:none;border-radius:5px;cursor:pointer;font-size:12px;transition:all .2s}"
".btn-on{background:#00ff88;color:#0a0e17}"
".btn-on:hover{background:#00cc6a;transform:scale(1.05)}"
".btn-off{background:#ff4444;color:#fff}"
".btn-off:hover{background:#cc3333;transform:scale(1.05)}"
".graph-container{display:grid;grid-template-columns:1fr;gap:20px}"
".graph-card{background:#16213e;padding:20px;border-radius:12px;border:1px solid #1f3a5f}"
".graph-title{font-size:16px;color:#8899bb;margin-bottom:15px}"
".settings-card{background:#16213e;padding:25px;border-radius:12px;border:1px solid #1f3a5f;max-width:500px}"
".settings-card h3{color:#00d4ff;margin-bottom:15px}"
".info-row{display:flex;justify-content:space-between;padding:8px 0;border-bottom:1px solid #1a2a3a}"
".info-row .label{color:#8899bb}"
".info-row .value{color:#fff}"
".settings-card button{padding:8px 20px;border:none;border-radius:6px;cursor:pointer;margin-right:10px;background:#0f3460;color:#fff}"
".settings-card button:hover{background:#1a4a7a}"
"select{background:#1a1a2e;color:#fff;border:1px solid #1f3a5f;padding:8px 15px;border-radius:6px;margin-right:10px}"
"</style>"
"</head>"
"<body>"
"<div class='header'><h1>🔥 MOY Controller v4.0 — SCADA</h1>"
"<div class='status'><span class='dot'></span><span id='statusText'>Подключено</span><span id='lastUpdate' style='margin-left:15px;color:#556688;'>Обновление: --</span></div></div>"
"<div class='tabs'><button class='tab active' data-tab='dashboard'>📊 Дашборд</button><button class='tab' data-tab='graphs'>📈 Графики</button><button class='tab' data-tab='settings'>⚙️ Настройки</button></div>"
"<div class='tab-content active' id='tab-dashboard'>"
"<div class='stats-bar'><div class='stat-item'>Всего пинов: <span id='totalPins'>0</span></div><div class='stat-item'>Активных: <span id='activePins'>0</span></div><div class='stat-item'>Выходы включены: <span id='outputsOn'>0</span></div></div>"
"<div class='pins-grid' id='pinsGrid'></div></div>"
"<div class='tab-content' id='tab-graphs'>"
"<div class='graph-container'><div class='graph-card'><div class='graph-title'>📈 Температура</div><div style='height:300px;'><canvas id='tempChart'></canvas></div>"
"<div style='margin-top:10px;'><select id='pinSelect' onchange='loadHistory()'></select><button onclick='loadHistory()'>🔄 Обновить</button></div></div></div></div>"
"<div class='tab-content' id='tab-settings'>"
"<div class='settings-card'><h3>⚙️ Системная информация</h3>"
"<div class='info-row'><span class='label'>Платформа:</span><span class='value'>ESP32-P4</span></div>"
"<div class='info-row'><span class='label'>Прошивка:</span><span class='value'>v1.5-scada-working</span></div>"
"<div class='info-row'><span class='label'>Свободная RAM:</span><span class='value' id='freeRam'>--</span></div>"
"<div style='margin-top:15px;'><button onclick='location.reload()'>🔄 Перезагрузить</button><button onclick='resetController()' style='background:#ff4444;'>⚠️ Сбросить</button></div></div></div>"
"<script>"
"let pinsData=[],chartInstance=null;"
"async function loadPins(){try{const r=await fetch('/api/pins');if(!r.ok)throw new Error('Ошибка сети');const d=await r.json();pinsData=d.pins||[];renderPins();updateStats();fillPinSelect();return true}catch(e){console.error('Ошибка загрузки:',e);document.getElementById('pinsGrid').innerHTML='<div style=\"color:#ff4444;padding:40px;text-align:center;\">❌ Ошибка загрузки данных</div>';return false}}"
"function renderPins(){const grid=document.getElementById('pinsGrid');grid.innerHTML='';let outputsOn=0;pinsData.forEach(p=>{const isActuator=p.id>=26&&p.id<=33;const value=p.value||0;const isOn=value>0.5;if(isActuator&&isOn)outputsOn++;const card=document.createElement('div');card.className='pin-card';card.id='pin-'+p.id;let valueClass='sensor';let displayValue=value.toFixed(1)+'°C';let controls='';if(isActuator){valueClass=isOn?'actuator on':'actuator off';displayValue=isOn?'ВКЛ':'ВЫКЛ';controls='<button class=\"'+(isOn?'btn-off':'btn-on')+'\" onclick=\"setPin('+p.id+','+(isOn?0:1)+')\">'+(isOn?'🔴 Выкл':'🟢 Вкл')+'</button>'}else{if(value>50)valueClass+=' alarm'}card.innerHTML='<div class=\"pin-name\">'+(p.name||'Пин '+p.id)+'</div><div class=\"pin-value '+valueClass+'\">'+displayValue+'</div><div class=\"pin-id\">GPIO '+p.id+' '+(p.online?'🟢':'🔴')+'</div><div class=\"pin-actions\">'+controls+'</div>';grid.appendChild(card)});document.getElementById('outputsOn').textContent=outputsOn}"
"async function setPin(pinId,value){try{const r=await fetch('/api/pin/set?pin='+pinId+'&value='+value);const d=await r.json();if(d.success){const p=pinsData.find(x=>x.id===pinId);if(p){p.value=value;renderPins();updateStats()}}else{alert('Ошибка: '+(d.error||'неизвестная ошибка'))}}catch(e){console.error('Ошибка управления:',e);alert('Ошибка связи с контроллером')}}"
"function updateStats(){const total=pinsData.length;const active=pinsData.filter(p=>p.online).length;document.getElementById('totalPins').textContent=total;document.getElementById('activePins').textContent=active}"
"function fillPinSelect(){const select=document.getElementById('pinSelect');select.innerHTML='';pinsData.forEach(p=>{const o=document.createElement('option');o.value=p.id;o.textContent=p.name||'Пин '+p.id;select.appendChild(o)});if(pinsData.length>0)loadHistory()}"
"async function loadHistory(){const pinId=document.getElementById('pinSelect').value;if(!pinId)return;try{const r=await fetch('/api/history?pin='+pinId+'&points=50');if(!r.ok)throw new Error('Ошибка загрузки');const d=await r.json();updateChart(d,pinId)}catch(e){console.error('Ошибка истории:',e)}}"
"function updateChart(data,pinId){const ctx=document.getElementById('tempChart').getContext('2d');const labels=data.map(d=>new Date(d.t*1000).toLocaleTimeString());const values=data.map(d=>d.v);if(chartInstance)chartInstance.destroy();const pin=pinsData.find(p=>p.id==pinId);const isActuator=pin&&pin.id>=26&&pin.id<=33;const color=isActuator?'#ffaa44':'#00d4ff';chartInstance=new Chart(ctx,{type:'line',data:{labels:labels,datasets:[{label:pin?pin.name:'Пин '+pinId,data:values,borderColor:color,backgroundColor:color+'20',fill:true,tension:0.4,pointRadius:3,pointBackgroundColor:color}]},options:{responsive:true,maintainAspectRatio:false,plugins:{legend:{labels:{color:'#8899bb'}}},scales:{x:{grid:{color:'#1a2a3a'},ticks:{color:'#556688'}},y:{grid:{color:'#1a2a3a'},ticks:{color:'#556688'}}}}})}"
"async function resetController(){if(confirm('Сбросить контроллер?')){try{await fetch('/api/reset',{method:'POST'});alert('Контроллер перезагружается...');setTimeout(()=>location.reload(),3000)}catch(e){alert('Ошибка сброса')}}}"
"document.querySelectorAll('.tab').forEach(t=>{t.addEventListener('click',function(){document.querySelectorAll('.tab').forEach(x=>x.classList.remove('active'));document.querySelectorAll('.tab-content').forEach(x=>x.classList.remove('active'));this.classList.add('active');document.getElementById('tab-'+this.dataset.tab).classList.add('active');if(this.dataset.tab==='graphs'&&chartInstance){setTimeout(()=>chartInstance.resize(),100)}})})"
"function updateTime(){document.getElementById('lastUpdate').textContent='Обновление: '+new Date().toLocaleTimeString()}"
"async function init(){await loadPins();updateTime();setInterval(()=>{loadPins();updateTime()},3000)}"
"document.addEventListener('DOMContentLoaded',init)"
"</script>"
"</body></html>";

// Обработчик корня
static esp_err_t root_handler(httpd_req_t* req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, index_html, strlen(index_html));
    return ESP_OK;
}

// API: получить все пины
static esp_err_t api_pins_handler(httpd_req_t* req) {
    httpd_resp_set_type(req, "application/json");
    char* json = sensor_export_json();
    httpd_resp_send(req, json, strlen(json));
    return ESP_OK;
}

// API: получить историю
static esp_err_t api_history_handler(httpd_req_t* req) {
    char query[64] = {0};
    uint8_t pin_id = 34;
    int points = 50;
    
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        char param[16];
        if (httpd_query_key_value(query, "pin", param, sizeof(param)) == ESP_OK) {
            pin_id = atoi(param);
        }
        if (httpd_query_key_value(query, "points", param, sizeof(param)) == ESP_OK) {
            points = atoi(param);
            if (points > 100) points = 100;
            if (points < 10) points = 10;
        }
    }
    
    httpd_resp_set_type(req, "application/json");
    char* json = sensor_export_history_json(pin_id, points);
    httpd_resp_send(req, json, strlen(json));
    return ESP_OK;
}

// API: установить пин
static esp_err_t api_set_pin_handler(httpd_req_t* req) {
    char query[64] = {0};
    uint8_t pin_id = 0;
    float value = 0;
    
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        char param[16];
        if (httpd_query_key_value(query, "pin", param, sizeof(param)) == ESP_OK) {
            pin_id = atoi(param);
        }
        if (httpd_query_key_value(query, "value", param, sizeof(param)) == ESP_OK) {
            value = atof(param);
        }
    }
    
    char* response = sensor_set_pin(pin_id, value);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

bool webserver_start(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start server");
        return false;
    }
    
    httpd_uri_t uri_root = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = root_handler
    };
    httpd_register_uri_handler(server, &uri_root);
    
    httpd_uri_t uri_api_pins = {
        .uri       = "/api/pins",
        .method    = HTTP_GET,
        .handler   = api_pins_handler
    };
    httpd_register_uri_handler(server, &uri_api_pins);
    
    httpd_uri_t uri_api_history = {
        .uri       = "/api/history",
        .method    = HTTP_GET,
        .handler   = api_history_handler
    };
    httpd_register_uri_handler(server, &uri_api_history);
    
    httpd_uri_t uri_api_set_pin = {
        .uri       = "/api/pin/set",
        .method    = HTTP_GET,
        .handler   = api_set_pin_handler
    };
    httpd_register_uri_handler(server, &uri_api_set_pin);
    
    ESP_LOGI(TAG, "Webserver started on port 80");
    return true;
}

void webserver_stop(void) {
    if (server) {
        httpd_stop(server);
        server = NULL;
        ESP_LOGI(TAG, "Webserver stopped");
    }
}