$(document).ready(function () {
    var previousValues = [null, null]; // Array para armazenar os dois últimos valores recebidos

    setInterval(function () {
        $.ajax({
            url: "/get_latest_message",
            type: "GET",
            success: function (data, textStatus, jqXHR) {
                console.log("Status da resposta:", jqXHR.status);
                console.log("Texto da resposta:", textStatus);
                console.log("Dados recebidos:", data);

                // Atualize a interface com os novos dados
                if (data && data.message) {
                    $("#temperature").text(data.message.temperature || "No data");
                    $("#umidade_ar").text(data.message.umidade_ar || "No data");
                    $("#umidade_solo1").text(data.message.umidade_solo1 || "No data");
                    $("#umidade_solo2").text(data.message.umidade_solo2 || "No data");
                    $("#umidade_solo3").text(data.message.umidade_solo3 || "No data");
                    $("#umidade_solo4").text(data.message.umidade_solo4 || "No data");
                    $("#chuva_status").text(chuvaFront(data.message.chuva));
                    $("#intensidade_status").text(intensidadeFront(data.message.intensidade_chuva));


                    var temperatureComparison = compareValues(previousValues[1]?.temperature, data.message.temperature);
                    var umidade_arComparison = compareValues(previousValues[1]?.umidade_ar, data.message.umidade_ar);
                    var umidade_solo1Comparison = compareValues(previousValues[1]?.umidade_solo1, data.message.umidade_solo1);
                    var umidade_solo2Comparison = compareValues(previousValues[1]?.umidade_solo2, data.message.umidade_solo2);
                    var umidade_solo3Comparison = compareValues(previousValues[1]?.umidade_solo3, data.message.umidade_solo3);
                    var umidade_solo4Comparison = compareValues(previousValues[1]?.umidade_solo4, data.message.umidade_solo4);

                    updateComparisonIcons(temperatureComparison, umidade_arComparison, umidade_solo1Comparison, umidade_solo2Comparison, umidade_solo3Comparison, umidade_solo4Comparison);

                    // Atualize o array de valores anteriores
                    previousValues.shift(); // Remove o primeiro elemento do array
                    previousValues.push(data.message); // Adiciona o novo valor ao final do array
                }
            },
            error: function (jqXHR, textStatus, errorThrown) {
                console.error("Erro na requisição:", textStatus, errorThrown);
            }
        });
    }, 5000); // Intervalo de 5 segundos

    function intensidadeFront(valor) {
        if (valor === null || valor === undefined) return "Sem dados";

        valor = Number(valor);

        if (valor < 1000) {
            return "Chuva intensa";
        } else if (valor >= 1000 && valor <= 3000) {
            return "Chuva Moderada ou Chuvisco";
        } else if (valor > 3000) {
            return "Sem previsão de chuva";
        }
    }

    function chuvaFront(valor) {
        if (valor === null || valor === undefined) return "Sem dados";

        valor = Number(valor);

        if (valor === 0) {
            return "Sim";
        } else if (valor === 1) {
            return "Não";
        }
    }


    // Função para comparar dois valores e retornar o tipo de comparação
    function compareValues(previous, current) {
        if (previous === null || current === null) {
            return "No data";
        } else if (current > previous) {
            return "subiu";
        } else if (current < previous) {
            return "desceu";
        } else {
            return "permaneceu o mesmo";
        }
    }

    // Função para atualizar os ícones de comparação na interface
    function updateComparisonIcons(temperatureComparison, umidade_arComparison, umidade_solo1Comparison, umidade_solo2Comparison, umidade_solo3Comparison, umidade_solo4Comparison) {
        $("#temperature_comparison").html(getComparisonIcon(temperatureComparison));
        $("#umidade_ar_comparison").html(getComparisonIcon(umidade_arComparison));
        $("#umidade_solo1_comparison").html(getComparisonIcon(umidade_solo1Comparison));
        $("#umidade_solo2_comparison").html(getComparisonIcon(umidade_solo2Comparison));
        $("#umidade_solo3_comparison").html(getComparisonIcon(umidade_solo3Comparison));
        $("#umidade_solo4_comparison").html(getComparisonIcon(umidade_solo4Comparison));
    }

    // Função para obter o ícone de comparação com base no valor recebido
    function getComparisonIcon(comparison) {
        var iconHtml = '';
        switch (comparison) {
            case "No data":
                iconHtml = `<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" fill="currentColor" class="bi bi-database-fill-exclamation" viewBox="0 0 16 16" style="color: red;">
                                <path d="M8 1c-1.573 0-3.022.289-4.096.777C2.875 2.245 2 2.993 2 4s.875 1.755 1.904 2.223C4.978 6.711 6.427 7 8 7s3.022-.289 4.096-.777C13.125 5.755 14 5.007 14 4s-.875-1.755-1.904-2.223C11.022 1.289 9.573 1 8 1"/>
                                <path d="M2 7v-.839c.457.432 1.004.751 1.49.972C4.722 7.693 6.318 8 8 8s3.278-.307 4.51-.867c.486-.22 1.033-.54 1.49-.972V7c0 .424-.155.802-.411 1.133a4.51 4.51 0 0 0-4.815 1.843A12 12 0 0 1 8 10c-1.573 0-3.022-.289-4.096-.777C2.875 8.755 2 8.007 2 7m6.257 3.998L8 11c-1.682 0-3.278-.307-4.51-.867-.486-.22-1.033-.54-1.49-.972V10c0 1.007.875 1.755 1.904 2.223C4.978 12.711 6.427 13 8 13h.027a4.55 4.55 0 0 1 .23-2.002m-.002 3L8 14c-1.682 0-3.278-.307-4.51-.867-.486-.22-1.033-.54-1.49-.972V13c0 1.007.875 1.755 1.904 2.223C4.978 15.711 6.427 16 8 16c.536 0 1.058-.034 1.555-.097a4.5 4.5 0 0 1-1.3-1.905"/>
                                <path d="M16 12.5a3.5 3.5 0 1 1-7 0 3.5 3.5 0 0 1 7 0m-3.5-2a.5.5 0 0 0-.5.5v1.5a.5.5 0 0 0 1 0V11a.5.5 0 0 0-.5-.5m0 4a.5.5 0 1 0 0-1 .5.5 0 0 0 0 1"/>
                            </svg>`;
                break;
            case "subiu":
                iconHtml = `<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" fill="currentColor" class="bi bi-arrow-up" viewBox="0 0 16 16" style="color: green;">
                                <path fill-rule="evenodd" d="M8 15a.5.5 0 0 0 .5-.5V2.707l3.146 3.147a.5.5 0 0 0 .708-.708l-4-4a.5.5 0 0 0-.708 0l-4 4a.5.5 0 1 0 .708.708L7.5 2.707V14.5a.5.5 0 0 0 .5.5"/>
                            </svg>`;
                break;
            case "desceu":
                iconHtml = `<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" fill="currentColor" class="bi bi-arrow-down" viewBox="0 0 16 16" style="color: red;">
                                <path fill-rule="evenodd" d="M8 1a.5.5 0 0 1 .5.5v11.793l3.146-3.147a.5.5 0 0 1 .708.708l-4 4a.5.5 0 0 1-.708 0l-4-4a.5.5 0 0 1 .708-.708L7.5 13.293V1.5A.5.5 0 0 1 8 1"/>
                            </svg>`;
                break;
            case "permaneceu o mesmo":
                iconHtml = `<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" fill="currentColor" class="bi bi-arrow-right" viewBox="0 0 16 16" style="color: gray;">
                                <path fill-rule="evenodd" d="M1 8a.5.5 0 0 1 .5-.5h11.793l-3.147-3.146a.5.5 0 0 1 .708-.708l4 4a.5.5 0 0 1 0 .708l-4 4a.5.5 0 0 1-.708-.708L13.293 8.5H1.5a.5.5 0 0 1-.5-.5"/>
                            </svg>`;
                break;
            default:
                iconHtml = '';
                break;
        }
        return iconHtml;
    }
});