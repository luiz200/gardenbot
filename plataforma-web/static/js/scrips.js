$(document).ready(function () {
    setInterval(function () {
        $.ajax({
            url: "/get_latest_message",
            type: "GET",
            success: function (data) {
                $("#temperature").text(data.message.temperature);
                $("#umidade_ar").text(data.message.umidade_ar);
                $("#umidade_solo1").text(data.message.umidade_solo1);
                $("#umidade_solo2").text(data.message.umidade_solo2);
                $("#umidade_solo3").text(data.message.umidade_solo3);
                $("#umidade_solo4").text(data.message.umidade_solo4);
                $("#temperature_comparison").text(data.message.temperature_comparison);
                $("#previous_temperature").text(data.message.previous_temperature)
            }
        });
    }, 1000);
});
