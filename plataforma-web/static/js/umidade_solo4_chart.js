document.addEventListener('DOMContentLoaded', function() {

    var ctx = document.getElementById('umidadeSolo4Chart').getContext('2d');

    var data = {
        labels: [],
        datasets: [{
            label: 'Umidade Solo 4',
            borderColor: 'rgb(75, 192, 192)',
            data: [0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100],
            fill: false
        }]
    };

    var umidadeSolo4Chart = new Chart(ctx, {
        type: 'line',
        data: data,
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: {
                    display: false
                }
            }
        }
    });

    function updateChart() {

        fetch('/get_latest_message')
            .then(response => response.json())
            .then(data => {

                umidadeSolo4Chart.data.labels.push(new Date().toLocaleTimeString());
                umidadeSolo4Chart.data.datasets[0].data.push(parseFloat(data.message.umidade_solo4));

                if(umidadeSolo4Chart.data.labels.length > 10) {
                    umidadeSolo4Chart.data.labels.shift();
                    umidadeSolo4Chart.data.datasets[0].data.shift();
                }
                            
                umidadeSolo4Chart.update();
            });
    }

    setInterval(updateChart, 5000);
});
