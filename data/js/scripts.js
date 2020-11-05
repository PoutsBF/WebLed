// Empty JS for your own code to be here

function domReady(f) {
  if (document.readyState === 'complete') {
    f();
  } else {
    document.addEventListener('DOMContentLoaded', f);
  }
}

function hexToDec(hex) {
    var result = 0, digitValue;
    hex = hex.toLowerCase();
    for (var i = 0; i < hex.length; i++)
    {
        if (hex[i] != '#')
        {
            digitValue = '0123456789abcdefgh'.indexOf(hex[i]);
            result = result * 16 + digitValue;
        }
    }
    return result;
}

domReady(function()
{
    var connection = new WebSocket('ws://' + location.hostname + '/ws');
    function onCouleur(color) {
        if (connection.readyState === 1)
        {
            document.getElementById("idCouleur").value = color;
            var couleur = hexToDec(color);
            connection.send("{couleur:" + couleur + "}");
        }
    }
    $.farbtastic('#picker').linkTo(onCouleur);

    function onSelectionMode() {
        if (connection.readyState === 1) {
            connection.send("{mode:" + document.getElementById("idSelectionMode").selectedIndex + "}");
        }
    }
    function onVitesse() {
        if (connection.readyState === 1)
        {
            var vitesse = document.getElementById("idVitesse").value;
            connection.send("{speed:" + vitesse + "}");
        }
    }
    function onLuminosite() {
        if (connection.readyState === 1)
        {
            var luminosite = document.getElementById("idLuminosite").value;
            connection.send("{lum:" + luminosite + "}");
        }
    }

    document.getElementById("idSelectionMode").addEventListener("change", onSelectionMode);
    document.getElementById("idCouleur").addEventListener("change", onCouleur);
    document.getElementById("idVitesse").addEventListener("change", onVitesse);
    document.getElementById("idLuminosite").addEventListener("change", onLuminosite);

    connection.onopen = function ()
    {
        connection.send('{\"Connect\":\"' + new Date() + '\"}');
    };

    connection.onerror = function (error)
    {
        console.log('WebSocket Error ', error);
    };

    connection.onmessage = function (event)
    {
        try
        {
            var msg = JSON.parse(event.data);
            if (msg.hasOwnProperty("modes"))
            {
                var text = "";
                msg["modes"].forEach(function (element) {
                    text += "<option>" + element + "</option>";
                });
                document.getElementById("idSelectionMode").innerHTML = text;
            }
            if (msg.hasOwnProperty("mode"))
            {
                document.getElementById("idSelectionMode").value = msg["mode"];
            }
            if (msg.hasOwnProperty("speed"))
            {
                document.getElementById("idVitesse").value = msg["speed"];
            }
            if (msg.hasOwnProperty("lum"))
            {
                document.getElementById("idLuminosite").value = msg["lum"];
            }
            if (msg.hasOwnProperty("couleur"))
            {
                $('#picker').farbtastic('#idColor').color = msg["couleur"];
            }
        }
        catch (e)
        {
            console.error("Parsing error:", e);
            console.log(event.data);
        }
    };
});
