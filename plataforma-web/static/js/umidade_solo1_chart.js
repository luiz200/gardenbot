document.addEventListener('DOMContentLoaded', function() {

    var ctx = document.getElementById('umidadeSolo1Chart').getContext('2d');

    var data = {
        labels: [],
        datasets: [{
            label: 'Umidade Solo 1',
            borderColor: 'rgb(75, 192, 192)',
            data: [0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100],
            fill: false
        }]
    };

    var umidadeSolo1Chart = new Chart(ctx, {
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

                umidadeSolo1Chart.data.labels.push(new Date().toLocaleTimeString());
                umidadeSolo1Chart.data.datasets[0].data.push(parseFloat(data.message.umidade_solo1));

                if (umidadeSolo1Chart.data.labels.length > 10) {
                    umidadeSolo1Chart.data.labels.shift();
                    umidadeSolo1Chart.data.datasets[0].data.shift();
                }

                umidadeSolo1Chart.update();
            });
    }

    setInterval(updateChart, 5000);
});
