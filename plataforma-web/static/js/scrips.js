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

                compareValues(data.message);
            }
        });
    }, 1000);
});

$(document).ready(function (){
    setInterval(function (){
        $.ajax({
            url: "/get_previous_message",
            type: "GET",
            success: function(data){
                $("#temperature_p").text(data.message.temperature_p);
                $("#umidade_ar_p").text(data.message.umidade_ar_p);
                $("#umidade_solo1_p").text(data.message.umidade_solo1_p);
                $("#umidade_solo2_p").text(data.message.umidade_solo2_p);
                $("#umidade_solo3_p").text(data.message.umidade_solo3_p);
                $("#umidade_solo4_p").text(data.message.umidade_solo4_p);
            }
        });
    }, 1000);
});

function compareValues(latest) {
    compareAndDisplay("temperature", latest.temperature, $("#temperature_p").text(), "#temperature_comparison");
    compareAndDisplay("umidade_ar", latest.umidade_ar, $("#umidade_ar_p").text(), "#umidade_ar_comparison");
    compareAndDisplay("umidade_solo1", latest.umidade_solo1, $("#umidade_solo1_p").text(), "#umidade_solo1_comparison");
    compareAndDisplay("umidade_solo2", latest.umidade_solo2, $("#umidade_solo2_p").text(), "#umidade_solo2_comparison");
    compareAndDisplay("umidade_solo3", latest.umidade_solo3, $("#umidade_solo3_p").text(), "#umidade_solo3_comparison");
    compareAndDisplay("umidade_solo4", latest.umidade_solo4, $("#umidade_solo4_p").text(), "#umidade_solo4_comparison");
}

function compareAndDisplay(latestValue, previousValue, comparisonElementId) {
    
    if (latestValue > previousValue) {
        $(comparisonElementId).html(
            '<div style="color: rgb(43, 255, 0);">'+
            '<span id="temperature_p">' + previousValue + '</span>'+
            '<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" fill="currentColor" class="bi bi-arrow-up" viewBox="0 0 16 16">'+
            '<path fill-rule="evenodd" d="M8 15a.5.5 0 0 0 .5-.5V2.707l3.146 3.147a.5.5 0 0 0 .708-.708l-4-4a.5.5 0 0 0-.708 0l-4 4a.5.5 0 1 0 .708.708L7.5 2.707V14.5a.5.5 0 0 0 .5.5"/>'+
            '</svg>'+
            '</div>'
            );
    } else if (latestValue < previousValue) {
        $(comparisonElementId).html(
            '<div style="color: rgb(255, 0, 0);">'+
            '<span id="temperature_p">' + previousValue + '</span>'+
            '<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" fill="currentColor" class="bi bi-arrow-down" viewBox="0 0 16 16">'+
            '<path fill-rule="evenodd" d="M8 1a.5.5 0 0 1 .5.5v11.793l3.146-3.147a.5.5 0 0 1 .708.708l-4 4a.5.5 0 0 1-.708 0l-4-4a.5.5 0 0 1 .708-.708L7.5 13.293V1.5A.5.5 0 0 1 8 1"/>'+
            '</svg>'+
            '</div>'
            );
    } else {
        $(comparisonElementId).html(
            '<div style="color: rgb(255, 0, 0);">'+
            '<span id="temperature_p">' + previousValue + '</span>'+
            '<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" fill="currentColor" class="bi bi-arrows" viewBox="0 0 16 16">'+
            '<path d="M1.146 8.354a.5.5 0 0 1 0-.708l2-2a.5.5 0 1 1 .708.708L2.707 7.5h10.586l-1.147-1.146a.5.5 0 0 1 .708-.708l2 2a.5.5 0 0 1 0 .708l-2 2a.5.5 0 0 1-.708-.708L13.293 8.5H2.707l1.147 1.146a.5.5 0 0 1-.708.708z"/>'+
            '</svg>'+
            '</div>'
            );
    }

}
