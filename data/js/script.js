function onButton()
{  /* Fonction pour le bouton ON */
    console.log('send led on');
    connection.send("{\"led\":1}");
}
function offButton()
{  /* Fonction pour le bouton OFF */
    console.log('send led off');
    connection.send("{\"led\":0}");
}

var connection = new WebSocket('ws://' + location.hostname + '/ws');
connection.onopen = function ()
{
    connection.send('{\"Connect\":\"' + new Date() + '\"}');
};

connection.onerror = function (error)
{
    console.log('WebSocket Error ', error);
};
/*
connection.onclose = function (event)
{
    switch (event.code)
    {
        case 1000: console.log("Normal Closure"); break;
        case 1001: console.log("Going Away"); break;
        case 1002: console.log("Protocol Error"); break;
        case 1003: console.log("Unsupported Data"); break;
        case 1005: console.log("No Status Received"); break;
        case 1006: console.log("Abnormal Closure"); break;
        case 1007: console.log("Invalid frame payload data"); break;
        case 1008: console.log("Policy Violation"); break;
        case 1009: console.log("Message too big"); break;
        case 1010: console.log("Missing Extension"); break;
        case 1011: console.log("Internal Error"); break;
        case 1012: console.log("Service Restart"); break;
        case 1013: console.log("Try Again Later"); break;
        case 1014: console.log("Bad Gateway"); break;
        case 1015: console.log("TLS Handshake"); break;
        default: console.log("fin de web socket autre");
    }
    if (event.wasClean)
        console.log("arrêt nettoyé");
    else
        console.log("arrêt non nettoyé");
}*/

connection.onmessage = function (event)
{
    try
    {
        var msg = JSON.parse(event.data);
    }
    catch (e)
    {
        console.error("Parsing error:", e);
        console.log(event.data);
    }
    var text = "";
    if (typeof msg.stamp != 'undefined')
    {
        var time = new Date(msg.stamp);
        var timeStr = time.toLocaleTimeString();

        text = "<b>donnée à " + timeStr + "</b><br>";
        text += "<b>température " + msg.temp + " °C</b><br>";
        text += "<b>hygrométrie " + msg.hydr + " %HR</b><br>";

        document.jaugeTEMP.series[0].setData([msg.temp], true);
        document.jaugeHR.series[0].setData([msg.hydr], true);
        document.jaugePRESSION.series[0].setData([msg.pression], true);
        document.jaugeGAZ.series[0].setData([msg.gas_r], true);

        if (text.length) {
            document.getElementById("display").innerHTML = text;
        }
    }
    console.log('Server: ', event.data);
};

var urlInput = location.href + 'temperature.csv';

//*****************************************************************************
//  Graphes & jauges
//-----------------------------------------------------------------------------
document.histoTEMP = new Highcharts.chart({
    chart: {
        renderTo: 'container',
        type: 'spline',
        zoomType: 'x'
    },
    title: {
        text: 'Environnement',
        align: 'left'
    },
    credits: {
        enabled: false
    },
    accessibility: {
        announceNewData: {
            enabled: true,
            minAnnounceInterval: 15000,
            announcementFormatter: function (allSeries, newSeries, newPoint) {
                if (newPoint) {
                    return 'New point added. Value: ' + newPoint.y;
                }
                return false;
            }
        }
    },
    yAxis: [{// premier axe, la température
        min: -10,
        max: 40,
        labels: {
            format: '{value}°C'
        },
        title: {
            text: 'température',
            style: {
                color: Highcharts.getOptions().colors[2]
            }
        },
        opposite: true
    }, {    // Second axe, %HR
        gridLineWidth: 0,
        min: 0,
        max: 100,
        labels: {
            format: '{value} %HR',
        },
        title: {
            text: 'hygrométrie',
            style: {
                color: Highcharts.getOptions().colors[0]
            }
        }
    }, {     // Troisième axe, pression atmosphérique hPa
        gridLineWidth: 0,
        min: 920,
        max: 1060,
        title: {
            text: 'Pression atmosphérique',
            style: {
                color: Highcharts.getOptions().colors[1]
            }
        },

        labels: {
            format: '{value} hPa',
            style: {
                color: Highcharts.getOptions().colors[1]
            }
        },
        opposite: true
    }, {       // Quatrième axe, qualité de l'air
        gridLineWidth: 0,
        min: 0,
        max: 30000,
        labels: {
            format: '{value} Ohm',
        },
        title: {
            text: "Qualité de l'air",
            style: {
                color: Highcharts.getOptions().colors[3]
            }
        }
    }],
    data: {
        csvURL: urlInput,
        enablePolling: true,
        dataRefreshRate: 120
    },
    series: [{
        yAxis: 0
    }, {
        yAxis: 1
    }, {
        yAxis: 2
    }, {
        yAxis: 3
    }]
});

//-----------------------------------------------------------------------------
document.jaugeTEMP = new Highcharts.chart({
    chart: {
        renderTo: "containerTEMP",
        type: 'gauge',
        plotBorderWidth: 1,
        plotBackgroundColor: {
            linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1 },
            stops: [
                [0, '#FFF4C6'],
                [0.3, '#FFFFFF'],
                [1, '#FFF4C6']
            ]
        },
        plotBackgroundImage: null,
        height: 200
    },

    title: {
        text: 'Température'
    },

    pane: [{
        startAngle: -45,
        endAngle: 45,
        background: null,
        center: ['50%', '145%'],
        size: 300
    }],
    exporting: {
        enabled: false
    },
    credits: {
        enabled: false
    },
    tooltip: {
        enabled: false
    },

    yAxis: [{
        min: -10,
        max: 40,
        minorTickPosition: 'outside',
        tickPosition: 'outside',
        labels: {
            formatter: function () {
                return this.value + '°';
            },
            rotation: 'auto',
            distance: 10
        },
        plotBands: [{
            from: 30,
            to: 40,
            color: '#C02316',
            innerRadius: '100%',
            outerRadius: '105%'
        }, {
            from: 25,
            to: 30,
            color: '#EA4D40',
            innerRadius: '100%',
            outerRadius: '105%'
        }, {
            from: -10,
            to: 0,
            color: '#00FFFF',
            innerRadius: '100%',
            outerRadius: '105%'
        }, {
            from: 0,
            to: 10,
            color: '#0080FF',
            innerRadius: '100%',
            outerRadius: '105%'
        }],
        pane: 0,
        title: {
            text: '°C<br/>température',
            y: -40
        }
    }],

    plotOptions: {
        gauge: {
            dataLabels: {
                enabled: false
            },
            dial: {
                radius: '100%'
            }
        }
    },

    series: [{
        name: 'temperature',
        data: [10],
        dataLabels: {
            formatter: function () {
                var temperature = this.y;
                return '<span style="color:#339">' + temperature + ' °C</span>';
            }
        },
        yAxis: 0
    }]
});

//-----------------------------------------------------------------------------
document.jaugeHR = new Highcharts.chart({
    chart: {
        renderTo: "containerHR",
        type: 'gauge',
        plotBorderWidth: 1,
        plotBackgroundColor: {
            linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1 },
            stops: [
                [0, '#FFF4C6'],
                [0.3, '#FFFFFF'],
                [1, '#FFF4C6']
            ]
        },
        plotBackgroundImage: null,
        height: 200
    },

    title: {
        text: 'Hygrométrie'
    },

    pane: [{
        startAngle: -45,
        endAngle: 45,
        background: null,
        center: ['50%', '145%'],
        size: 300
    }],
    exporting: {
        enabled: false
    },
    credits: {
        enabled: false
    },
    tooltip: {
        enabled: false
    },

    yAxis: [{
        min: 0,
        max: 100,
        minorTickPosition: 'outside',
        tickPosition: 'outside',
        labels: {
            formatter: function () {
                return this.value + '%';
            },
            rotation: 'auto',
            distance: 10
        },
        plotBands: [{
            from: 0,
            to: 30,
            color: '#808000',
            innerRadius: '100%',
            outerRadius: '105%'
        }, {
            from: 60,
            to: 100,
            color: '#0000FF',
            innerRadius: '100%',
            outerRadius: '105%'
        }],
        pane: 0,
        title: {
            text: '%HR<br/>humidité relative',
            y: -40
        }
    }],

    plotOptions: {
        gauge: {
            dataLabels: {
                enabled: false
            },
            dial: {
                radius: '100%'
            }
        }
    },

    series: [{
        name: 'HR',
        data: [10],
        yAxis: 0
    }]
});

//-----------------------------------------------------------------------------
document.jaugePRESSION = new Highcharts.chart({
    chart: {
        renderTo: "containerPRESSION",
        type: 'gauge',
        height: 200
    },

    title: {
        text: 'Pression Atmosphérique'
    },

    //    pane: 0,
    pane: [{
        startAngle: -90,
        endAngle: 90,
        center: ['50%', '110%'],
        size: 300,
        background: null
    }],
    exporting: {
        enabled: false
    },
    credits: {
        enabled: false
    },
    tooltip: {
        enabled: false
    },

    yAxis: [{
        min: 920,
        max: 1060,
        //        minorTickPosition: 'outside',
        //        tickPosition: 'outside',
        labels: {
            formatter: function () {
                return this.value + 'hPa';
            },
            rotation: 'auto',
            distance: 10
        },
        plotBands: [{
            from: 920,
            to: 940,
            //            label:{text:"tempête !"},
            color: '#000000',
            innerRadius: '1%',
            outerRadius: '100%'
        }, {
            from: 940,
            to: 960,
            //            label:{text:"tempête !"},
            color: '#101010',
            innerRadius: '1%',
            outerRadius: '100%'
        }, {
            from: 960,
            to: 980,
            //            label:{text:"tempête !"},
            color: '#808080',
            innerRadius: '1%',
            outerRadius: '100%'
        }, {
            from: 980,
            to: 1000,
            //            label:{text:"pluie"},
            color: '#0000FF',
            innerRadius: '1%',
            outerRadius: '100%'
        }, {
            from: 1000,
            to: 1020,
            //            label:{text:"variable"},
            color: '#0080FF',
            innerRadius: '1%',
            outerRadius: '100%'
        }, {
            from: 1020,
            to: 1040,
            //            label:{text:"beau"},
            color: '#EA4D40',
            innerRadius: '1%',
            outerRadius: '100%'
        }, {
            from: 1040,
            to: 1060,
            //            label:{text:"beau"},
            color: '#FF0000',
            innerRadius: '1%',
            outerRadius: '100%'
        }],
        title: {
            text: 'hPa<br/>pression',
            y: -40
        }
    }],

    plotOptions: {
        gauge: {
            dataLabels: {
                enabled: false
            },
            dial: {
                radius: '100%'
            }
        }
    },

    series: [{
        name: 'pression',
        data: [1013.25],
        dataLabels: {
            formatter: function () {
                var temperature = this.y;
                return '<span style="color:#339">' + temperature + ' hPa</span>';
            }
        },
        yAxis: 0
    }]
});

//-----------------------------------------------------------------------------
document.jaugeGAZ = new Highcharts.chart({
    chart: {
        renderTo: "containerGAZ",
        type: 'gauge',
        plotBorderWidth: 1,
        plotBackgroundColor: {
            linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1 },
            stops: [
                [0, '#FFF4C6'],
                [0.3, '#FFFFFF'],
                [1, '#FFF4C6']
            ]
        },
        plotBackgroundImage: null,
        height: 200
    },

    title: {
        text: "Qualité de l'air"
    },

    pane: [{
        startAngle: -45,
        endAngle: 45,
        background: null,
        center: ['50%', '145%'],
        size: 300
    }],
    exporting: {
        enabled: false
    },
    credits: {
        enabled: false
    },
    tooltip: {
        enabled: false
    },

    yAxis: [{
        min: 0,
        max: 30000,
        minorTickPosition: 'outside',
        tickPosition: 'outside',
        labels: {
            rotation: 'auto',
            distance: 10
        },
        plotBands: [{
            from: 0,
            to: 1000,
            color: '#C02316',
            innerRadius: '100%',
            outerRadius: '105%'
        }, {
            from: 1000,
            to: 10000,
            color: '#EA4D40',
            innerRadius: '100%',
            outerRadius: '105%'
        }, {
            from: 10000,
            to: 30000,
            color: '#0080FF',
            innerRadius: '100%',
            outerRadius: '105%'
        }],
        pane: 0,
        title: {
            text: "qualité de l'air",
            y: -40
        }
    }],

    plotOptions: {
        gauge: {
            dataLabels: {
                enabled: false
            },
            dial: {
                radius: '100%'
            }
        }
    },

    series: [{
        name: 'HR',
        data: [10],
        dataLabels: {
            formatter: function () {
                var temperature = this.y;
                return '<span style="color:#339">' + temperature + ' °C</span>';
            }
        },
        yAxis: 0
    }]
});
