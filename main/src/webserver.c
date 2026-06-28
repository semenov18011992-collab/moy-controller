#include "module.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include <string.h>
#include "sensor_module.h"
#include "config_manager.h"
#include "pin_config.h"
#include "cJSON.h"
#include "esp_system.h"
#include "hal/efuse_hal.h"

#define TAG "WEBSERVER"

static httpd_handle_t server = NULL;

// ============================================
// HTML СТРАНИЦА (встроена в код)
// ============================================

static const char* INDEX_HTML = 
"<!DOCTYPE html>"
"<html lang='ru'>"
"<head>"
"<meta charset='UTF-8'>"
"<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
"<title>MOY Controller v4.0</title>"
"<script src='https://cdn.jsdelivr.net/npm/chart.js@4.4.0/dist/chart.umd.min.js'></script>"
"<style>"
"*{margin:0;padding:0;box-sizing:border-box}"
"body{font-family:'Segoe UI',sans-serif;background:#0a0e17;color:#e0e0e0;padding:20px}"
".header{background:linear-gradient(135deg,#0f3460,#16213e);padding:20px;border-radius:12px;margin-bottom:20px;border:1px solid #1a4a7a;display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap}"
".header h1{color:#00d4ff;font-size:28px;font-weight:300}"
".status{display:flex;align-items:center;gap:15px;flex-wrap:wrap}"
".dot{display:inline-block;width:10px;height:10px;background:#00ff88;border-radius:50%;margin-right:8px;animation:pulse 1.5s infinite}"
"@keyframes pulse{0%,100%{opacity:1}50%{opacity:0.3}}"
"@keyframes blink{0%,100%{opacity:1}50%{opacity:.2}}"
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
".pin-card:hover{border-color:#00d4ff;transform:translateY(-2px)}"
".pin-card .pin-name{font-size:12px;color:#8899bb;text-transform:uppercase;letter-spacing:1px}"
".pin-card .pin-value{font-size:28px;font-weight:bold;margin:5px 0}"
".pin-card .pin-value.sensor{color:#00d4ff}"
".pin-card .pin-value.actuator{color:#ffaa44}"
".pin-card .pin-value.input{color:#aa88ff}"
".pin-card .pin-value.on{color:#00ff88}"
".pin-card .pin-value.off{color:#556688}"
".pin-card .pin-value.alarm{color:#ff4444;animation:blink 1s infinite}"
".pin-card .pin-type{font-size:11px;padding:2px 10px;border-radius:10px;display:inline-block;margin-top:5px}"
".pin-card .pin-type.sensor{background:#0f3460;color:#00d4ff}"
".pin-card .pin-type.actuator{background:#3d1f00;color:#ffaa44}"
".pin-card .pin-type.input{background:#2a1f4a;color:#aa88ff}"
".pin-card .pin-actions{margin-top:10px;display:flex;gap:5px;flex-wrap:wrap}"
".pin-card .pin-actions button{padding:5px 15px;border:none;border-radius:5px;cursor:pointer;font-size:12px;transition:all .2s}"
".btn-on{background:#00ff88;color:#0a0e17}"
".btn-on:hover{background:#00cc6a;transform:scale(1.05)}"
".btn-off{background:#ff4444;color:#fff}"
".btn-off:hover{background:#cc3333;transform:scale(1.05)}"
".group-title{font-size:18px;color:#00d4ff;margin:20px 0 10px 0;padding-bottom:5px;border-bottom:1px solid #1a4a7a}"
".group-title:first-of-type{margin-top:0}"
".settings-card{background:#16213e;padding:25px;border-radius:12px;border:1px solid #1f3a5f;max-width:100%;margin-top:20px}"
".settings-card h3{color:#00d4ff;margin-bottom:15px}"
".info-row{display:flex;justify-content:space-between;padding:8px 0;border-bottom:1px solid #1a2a3a}"
".info-row .label{color:#8899bb}"
".info-row .value{color:#fff}"
".settings-card button{padding:8px 20px;border:none;border-radius:6px;cursor:pointer;margin-right:10px;background:#0f3460;color:#fff}"
".settings-card button:hover{background:#1a4a7a}"
"select,input{background:#1a1a2e;color:#fff;border:1px solid #1f3a5f;padding:8px 15px;border-radius:6px;margin-right:10px}"
".config-grid{display:grid;grid-template-columns:70px 1fr 120px 100px auto;gap:10px;padding:8px 10px;border-bottom:1px solid #1a2a3a;align-items:center}"
".config-grid .label{color:#00d4ff;font-weight:bold}"
".config-grid input{background:#1a1a2e;color:#fff;border:1px solid #1f3a5f;padding:4px 8px;border-radius:4px;width:100%}"
".config-grid select{background:#1a1a2e;color:#fff;border:1px solid #1f3a5f;padding:4px 8px;border-radius:4px}"
".config-grid button{padding:4px 12px;border:none;border-radius:4px;cursor:pointer;font-size:12px}"
".btn-save{background:#0f3460;color:#fff}"
".btn-save:hover{background:#1a4a7a}"
".btn-delete{background:#ff4444;color:#fff}"
".btn-delete:hover{background:#cc3333}"
".graph-container{display:grid;grid-template-columns:1fr;gap:20px}"
".graph-card{background:#16213e;padding:20px;border-radius:12px;border:1px solid #1f3a5f}"
".graph-title{font-size:16px;color:#8899bb;margin-bottom:15px}"
"@media(max-width:600px){.pins-grid{grid-template-columns:1fr 1fr}.header h1{font-size:20px}.config-grid{grid-template-columns:1fr;gap:5px}}"
"</style>"
"</head>"
"<body>"
"<div class='header'><h1>MOY Controller v4.0</h1>"
"<div class='status'><span class='dot'></span><span id='statusText'>Подключено</span><span id='lastUpdate' style='margin-left:15px;color:#556688;'>Обновление: --</span></div></div>"
"<div class='tabs'><button class='tab active' onclick='showTab(\"dashboard\")'>Дашборд</button><button class='tab' onclick='showTab(\"graphs\")'>Графики</button><button class='tab' onclick='showTab(\"settings\")'>Настройки</button></div>"
"<div id='dashboard' class='tab-content active'><div id='statsBar'></div><div id='pinsContainer'></div></div>"
"<div id='graphs' class='tab-content'><div class='settings-card'><h3>Графики</h3><div style='height:300px;'><canvas id='tempChart'></canvas></div><div style='margin-top:10px;'><select id='pinSelect'></select><button onclick='loadHistory()' class='btn-save'>Обновить</button></div></div></div>"
"<div id='settings' class='tab-content'>"
"<div class='settings-card'><h3>Системная информация</h3>"
"<div class='info-row'><span class='label'>Платформа:</span><span class='value'>ESP32</span></div>"
"<div class='info-row'><span class='label'>Прошивка:</span><span class='value'>v2.0-scada-full</span></div>"
"<div class='info-row'><span class='label'>Свободная RAM:</span><span class='value' id='freeRam'>--</span></div>"
"<div style='margin-top:15px;'><button onclick='location.reload()' class='btn-save'>Перезагрузить</button><button onclick='resetController()' style='background:#ff4444;color:#fff;border:none;padding:8px 20px;border-radius:6px;cursor:pointer;'>Сбросить</button></div></div>"
"<div class='settings-card'><h3>Конфигурация пинов</h3>"
"<div style='margin-bottom:10px;'><button onclick='loadConfigs()' style='background:#00d4ff;color:#0a0e17;border:none;padding:8px 20px;border-radius:6px;cursor:pointer;'>Загрузить</button><button onclick='saveAllConfigs()' style='background:#00ff88;color:#0a0e17;border:none;padding:8px 20px;border-radius:6px;cursor:pointer;margin-left:10px;'>Сохранить все</button><span id='configStatus' style='margin-left:15px;color:#556688;font-size:14px;'></span></div>"
"<div id='pinConfigContainer'></div></div></div>"
"<script>"
"let pinsData=[], chartInstance=null, configsData=[];"
"function showTab(name){"
"document.querySelectorAll('.tab-content').forEach(function(t){t.classList.remove('active')});"
"document.querySelectorAll('.tab').forEach(function(t){t.classList.remove('active')});"
"document.getElementById(name).classList.add('active');"
"var tabs=document.querySelectorAll('.tab');"
"for(var i=0;i<tabs.length;i++){"
"if(tabs[i].getAttribute('onclick').indexOf(name)!==-1){tabs[i].classList.add('active')}"
"}"
"if(name==='settings'){loadConfigs()}"
"if(name==='graphs'){setTimeout(function(){fillPinSelect()},100)}"
"}"
"function updateTime(){document.getElementById('lastUpdate').textContent='Обновление: '+new Date().toLocaleTimeString()}"
"async function loadPins(){try{var r=await fetch('/api/pins');var d=await r.json();pinsData=d.pins||[];renderPins();updateStats();return true}catch(e){console.error(e);return false}}"
"function renderPins(){var container=document.getElementById('pinsContainer');container.innerHTML='';var groups={sensor:[],actuator:[],input:[]};"
"pinsData.forEach(function(p){var t=p.type||'sensor';if(groups[t])groups[t].push(p)});"
"var outputsOn=0;"
"var titles={sensor:'Датчики',actuator:'Выходы',input:'Входы'};"
"Object.keys(groups).forEach(function(key){var g=groups[key];if(g.length===0)return;"
"var titleDiv=document.createElement('div');titleDiv.className='group-title';titleDiv.textContent='🌡️ '+titles[key];container.appendChild(titleDiv);"
"var grid=document.createElement('div');grid.className='pins-grid';"
"g.forEach(function(p){var isActuator=p.is_output===true;var value=p.value||0;var isOn=value>0.5;if(isActuator&&isOn)outputsOn++;"
"var card=document.createElement('div');card.className='pin-card';"
"var valueClass=p.type||'sensor';var displayValue=value.toFixed(1)+'°C';var controls='';var typeLabel='Датчик';"
"if(isActuator){valueClass=isOn?'actuator on':'actuator off';displayValue=isOn?'ВКЛ':'ВЫКЛ';typeLabel='Выход';controls='<button class=\"'+(isOn?'btn-off':'btn-on')+'\" onclick=\"setPin('+p.id+','+(isOn?0:1)+')\">'+(isOn?'Выкл':'Вкл')+'</button>'}else if(p.type==='input'){displayValue=value>0.5?'АКТИВЕН':'НОРМА';valueClass=value>0.5?'input alarm':'input';typeLabel='Вход'}else{typeLabel='Датчик';if(value>50||value<-10)valueClass+=' alarm'}"
"card.innerHTML='<div class=\"pin-name\">'+(p.name||'Пин '+p.id)+'</div><div class=\"pin-value '+valueClass+'\">'+displayValue+'</div><div class=\"pin-type '+p.type+'\">'+typeLabel+' • GPIO '+p.id+'</div><div class=\"pin-actions\">'+controls+'</div>';grid.appendChild(card)});"
"container.appendChild(grid)});"
"var outEl=document.getElementById('outputsOn');if(outEl)outEl.textContent=outputsOn}"
"function updateStats(){var total=pinsData.length;var sensors=pinsData.filter(function(p){return p.type==='sensor'}).length;var actuators=pinsData.filter(function(p){return p.type==='actuator'||p.is_output===true}).length;var inputs=pinsData.filter(function(p){return p.type==='input'}).length;"
"document.getElementById('statsBar').innerHTML='<div class=\"stats-bar\"><div class=\"stat-item\">Всего пинов: <span>'+total+'</span></div><div class=\"stat-item\">Датчики: <span>'+sensors+'</span></div><div class=\"stat-item\">Выходы: <span>'+actuators+'</span></div><div class=\"stat-item\">Входы: <span>'+inputs+'</span></div><div class=\"stat-item\">Выходы включены: <span id=\"outputsOn\">0</span></div></div>'}"
"async function setPin(pinId,value){try{var r=await fetch('/api/pin/set?pin='+pinId+'&value='+value);var d=await r.json();if(d.success){var p=pinsData.find(function(x){return x.id===pinId});if(p){p.value=value;renderPins();updateStats()}}else{alert('Ошибка')}}catch(e){alert('Ошибка связи')}}"
"function fillPinSelect(){var select=document.getElementById('pinSelect');if(!select)return;select.innerHTML='';pinsData.forEach(function(p){var o=document.createElement('option');o.value=p.id;o.textContent=p.name||'Пин '+p.id;select.appendChild(o)});if(pinsData.length>0)loadHistory()}"
"async function loadHistory(){var pinId=document.getElementById('pinSelect').value;if(!pinId)return;try{var r=await fetch('/api/history?pin='+pinId+'&points=50');var d=await r.json();updateChart(d,pinId)}catch(e){console.error(e)}}"
"function updateChart(data,pinId){var ctx=document.getElementById('tempChart');if(!ctx)return;var canvas=ctx.getContext('2d');var labels=data.map(function(d){return new Date(d.t*1000).toLocaleTimeString()});var values=data.map(function(d){return d.v});if(chartInstance)chartInstance.destroy();var pin=pinsData.find(function(p){return p.id==pinId});var isActuator=pin&&pin.is_output===true;var color=isActuator?'#ffaa44':'#00d4ff';chartInstance=new Chart(canvas,{type:'line',data:{labels:labels,datasets:[{label:pin?pin.name:'Пин '+pinId,data:values,borderColor:color,backgroundColor:color+'20',fill:true,tension:0.4,pointRadius:3,pointBackgroundColor:color}]},options:{responsive:true,maintainAspectRatio:false,plugins:{legend:{labels:{color:'#8899bb'}}},scales:{x:{grid:{color:'#1a2a3a'},ticks:{color:'#556688'}},y:{grid:{color:'#1a2a3a'},ticks:{color:'#556688'}}}}})}"
"async function loadConfigs(){var s=document.getElementById('configStatus');s.textContent='Загрузка...';try{var r=await fetch('/api/pins/config/all');var d=await r.json();configsData=d.configs||[];renderConfigs();s.textContent='Загружено '+configsData.length+' конфигураций'}catch(e){s.textContent='Ошибка загрузки';console.error(e)}}"
"function renderConfigs(){var c=document.getElementById('pinConfigContainer');if(!c)return;if(configsData.length===0){c.innerHTML='<div style=\"color:#556688;padding:20px;text-align:center;\">Нет сохранённых конфигураций</div>';return}var html='';configsData.forEach(function(cfg){html+='<div class=\"config-grid\"><span class=\"label\">GPIO '+cfg.pin+'</span><input type=\"text\" value=\"'+cfg.name+'\" data-pin=\"'+cfg.pin+'\" class=\"cfg-name\" placeholder=\"Имя пина\"><select data-pin=\"'+cfg.pin+'\" class=\"cfg-type\"><option value=\"sensor\" '+(cfg.type==='sensor'?'selected':'')+'>Датчик</option><option value=\"actuator\" '+(cfg.type==='actuator'?'selected':'')+'>Выход</option><option value=\"input\" '+(cfg.type==='input'?'selected':'')+'>Вход</option><option value=\"disabled\" '+(cfg.type==='disabled'?'selected':'')+'>Отключен</option></select><select data-pin=\"'+cfg.pin+'\" class=\"cfg-mode\"><option value=\"digital\" '+(cfg.mode==='digital'?'selected':'')+'>Цифровой</option><option value=\"analog\" '+(cfg.mode==='analog'?'selected':'')+'>Аналоговый</option><option value=\"onewire\" '+(cfg.mode==='onewire'?'selected':'')+'>OneWire</option></select><div><button onclick=\"saveSingleConfig('+cfg.pin+')\" class=\"btn-save\">Сохранить</button><button onclick=\"deleteConfig('+cfg.pin+')\" class=\"btn-delete\">Удалить</button></div></div>'});c.innerHTML=html}"
"async function saveSingleConfig(pin){var s=document.getElementById('configStatus');s.textContent='Сохранение...';var ni=document.querySelector('.cfg-name[data-pin=\"'+pin+'\"]');var ts=document.querySelector('.cfg-type[data-pin=\"'+pin+'\"]');var ms=document.querySelector('.cfg-mode[data-pin=\"'+pin+'\"]');if(!ni){s.textContent='Ошибка';return}var cfg={pin:pin,name:ni.value||'GPIO_'+pin,type:ts?ts.value:'sensor',mode:ms?ms.value:'digital',enabled:ts?ts.value!=='disabled':true};try{var r=await fetch('/api/pin/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(cfg)});var d=await r.json();if(d.success){s.textContent='Пин '+pin+' сохранён';loadConfigs()}else{s.textContent='Ошибка'}}catch(e){s.textContent='Ошибка сохранения'}}"
"async function saveAllConfigs(){var s=document.getElementById('configStatus');s.textContent='Сохранение всех...';var inputs=document.querySelectorAll('.cfg-name');var types=document.querySelectorAll('.cfg-type');var modes=document.querySelectorAll('.cfg-mode');var saved=0;for(var i=0;i<inputs.length;i++){var pin=parseInt(inputs[i].dataset.pin);var cfg={pin:pin,name:inputs[i].value||'GPIO_'+pin,type:types[i]?types[i].value:'sensor',mode:modes[i]?modes[i].value:'digital',enabled:types[i]?types[i].value!=='disabled':true};try{var r=await fetch('/api/pin/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(cfg)});var d=await r.json();if(d.success)saved++}catch(e){}}s.textContent='Сохранено: '+saved;loadConfigs()}"
"async function deleteConfig(pin){if(!confirm('Удалить конфигурацию пина '+pin+'?'))return;var s=document.getElementById('configStatus');s.textContent='Удаление...';try{var r=await fetch('/api/pin/delete?pin='+pin,{method:'DELETE'});var d=await r.json();if(d.success){s.textContent='Пин '+pin+' удалён';loadConfigs()}else{s.textContent='Ошибка удаления'}}catch(e){s.textContent='Ошибка связи'}}"
"async function resetController(){if(confirm('Сбросить контроллер?')){try{await fetch('/api/reset',{method:'POST'});alert('Контроллер перезагружается...');setTimeout(function(){location.reload()},3000)}catch(e){alert('Ошибка сброса')}}}"
"async function loadSystemInfo(){try{var r=await fetch('/api/system');var d=await r.json();document.getElementById('freeRam').textContent=(d.free_heap/1024).toFixed(0)+' KB'}catch(e){}}"
"updateTime();loadPins();loadSystemInfo();setInterval(function(){updateTime();loadPins()},3000);"
"</script>"
"</body>"
"</html>";

// ============================================
// ОБРАБОТЧИК КОРНЯ
// ============================================

static esp_err_t root_handler(httpd_req_t* req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, INDEX_HTML, strlen(INDEX_HTML));
    return ESP_OK;
}

// ============================================
// API ОБРАБОТЧИКИ
// ============================================

static esp_err_t api_pins_handler(httpd_req_t* req) {
    httpd_resp_set_type(req, "application/json");
    char* json = sensor_export_json();
    httpd_resp_send(req, json, strlen(json));
    return ESP_OK;
}

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

// ============================================
// API КОНФИГУРАЦИИ
// ============================================

static esp_err_t pin_config_handler(httpd_req_t *req) {
    char content[512];
    size_t recv_size = req->content_len;
    
    if (recv_size > sizeof(content) - 1) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    content[recv_size] = '\0';
    
    cJSON *json = cJSON_Parse(content);
    if (!json) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }
    
    cJSON *pin_json = cJSON_GetObjectItem(json, "pin");
    cJSON *name_json = cJSON_GetObjectItem(json, "name");
    cJSON *type_json = cJSON_GetObjectItem(json, "type");
    cJSON *mode_json = cJSON_GetObjectItem(json, "mode");
    cJSON *enabled_json = cJSON_GetObjectItem(json, "enabled");
    
    if (!pin_json || !cJSON_IsNumber(pin_json)) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing pin number");
        return ESP_FAIL;
    }
    
    uint8_t pin = (uint8_t)pin_json->valueint;
    if (pin >= MAX_PINS) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid pin number");
        return ESP_FAIL;
    }
    
    pin_config_t cfg = {
        .pin = pin,
        .enabled = enabled_json && cJSON_IsBool(enabled_json) ? 
                   (enabled_json->valueint ? 1 : 0) : 1,
        .pullup = 0,
        .inverted = 0,
        .min_val = 0,
        .max_val = 100,
        .crc = 0
    };
    
    if (name_json && cJSON_IsString(name_json)) {
        strncpy(cfg.name, name_json->valuestring, MAX_PIN_NAME_LEN - 1);
        cfg.name[MAX_PIN_NAME_LEN - 1] = '\0';
    } else {
        snprintf(cfg.name, MAX_PIN_NAME_LEN, "GPIO_%d", pin);
    }
    
    if (type_json && cJSON_IsString(type_json)) {
        cfg.type = pin_string_to_type(type_json->valuestring);
    } else {
        cfg.type = PIN_TYPE_SENSOR;
    }
    
    if (mode_json && cJSON_IsString(mode_json)) {
        cfg.mode = pin_string_to_mode(mode_json->valuestring);
    } else {
        cfg.mode = PIN_MODE_DIGITAL;
    }
    
    esp_err_t err = config_save_pin(&cfg);
    cJSON_Delete(json);
    
    if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to save config");
        return ESP_FAIL;
    }
    
    config_apply_pin(&cfg);
    
    cJSON *response = cJSON_CreateObject();
    cJSON_AddBoolToObject(response, "success", true);
    cJSON_AddStringToObject(response, "message", "Config saved");
    cJSON_AddNumberToObject(response, "pin", pin);
    
    char *response_str = cJSON_PrintUnformatted(response);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response_str, strlen(response_str));
    
    free(response_str);
    cJSON_Delete(response);
    
    return ESP_OK;
}

static esp_err_t pins_config_all_handler(httpd_req_t *req) {
    pin_config_t configs[MAX_PINS];
    uint8_t count = 0;
    
    esp_err_t err = config_load_all(configs, &count);
    if (err != ESP_OK && err != ESP_ERR_NOT_FOUND) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to load configs");
        return ESP_FAIL;
    }
    
    cJSON *root = cJSON_CreateObject();
    cJSON *pins_array = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "configs", pins_array);
    
    for (int i = 0; i < count; i++) {
        cJSON *pin_obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(pin_obj, "pin", configs[i].pin);
        cJSON_AddStringToObject(pin_obj, "name", configs[i].name);
        cJSON_AddStringToObject(pin_obj, "type", pin_type_to_string(configs[i].type));
        cJSON_AddStringToObject(pin_obj, "mode", pin_mode_to_string(configs[i].mode));
        cJSON_AddBoolToObject(pin_obj, "enabled", configs[i].enabled);
        cJSON_AddNumberToObject(pin_obj, "min_val", configs[i].min_val);
        cJSON_AddNumberToObject(pin_obj, "max_val", configs[i].max_val);
        cJSON_AddBoolToObject(pin_obj, "pullup", configs[i].pullup);
        cJSON_AddBoolToObject(pin_obj, "inverted", configs[i].inverted);
        cJSON_AddItemToArray(pins_array, pin_obj);
    }
    
    char *response = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    
    free(response);
    cJSON_Delete(root);
    
    return ESP_OK;
}

static esp_err_t pin_delete_handler(httpd_req_t *req) {
    char query[64];
    char pin_str[8] = {0};
    
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "pin", pin_str, sizeof(pin_str));
    }
    
    if (strlen(pin_str) == 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing pin parameter");
        return ESP_FAIL;
    }
    
    uint8_t pin = atoi(pin_str);
    if (pin >= MAX_PINS) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid pin number");
        return ESP_FAIL;
    }
    
    esp_err_t err = config_delete_pin(pin);
    if (err != ESP_OK && err != ESP_ERR_NOT_FOUND) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to delete config");
        return ESP_FAIL;
    }
    
    cJSON *response = cJSON_CreateObject();
    cJSON_AddBoolToObject(response, "success", true);
    cJSON_AddStringToObject(response, "message", "Config deleted");
    cJSON_AddNumberToObject(response, "pin", pin);
    
    char *response_str = cJSON_PrintUnformatted(response);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response_str, strlen(response_str));
    
    free(response_str);
    cJSON_Delete(response);
    
    return ESP_OK;
}

static esp_err_t system_info_handler(httpd_req_t *req) {
    cJSON *response = cJSON_CreateObject();
    cJSON_AddNumberToObject(response, "free_heap", esp_get_free_heap_size());
    cJSON