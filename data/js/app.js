// Простой дашборд
fetch('/api/pins')
    .then(r => r.json())
    .then(data => {
        console.log('Pins data:', data);
        const container = document.getElementById('sensors');
        if (container) {
            container.innerHTML = '<pre>' + JSON.stringify(data, null, 2) + '</pre>';
        }
    });