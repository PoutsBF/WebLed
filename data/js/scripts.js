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
    $('#picker').farbtastic('#idColor');

    function onSelectionMode() {
        if (connection.readyState === 1) {
            connection.send("{mode:" + document.getElementById("idSelectionMode").selectedIndex + "}");
        }
    }
    function onCouleur() {
        if (connection.readyState === 1)
        {
            var couleur = hexToDec(document.getElementById("idColor").value);
            connection.send("{couleur:" + couleur + "}");
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
    document.getElementById("idColor").addEventListener("change", onCouleur);
    document.getElementById("idVitesse").addEventListener("change", onVitesse);
    document.getElementById("idLuminosite").addEventListener("change", onLuminosite);

    var connection = new WebSocket('ws://' + location.hostname + '/ws');
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
        var text = "";
        try
        {
            var msg = JSON.parse(event.data, (key, value) => {
                text += "<option id=\"" + key + "\">" + value + "</option>"; });
        }
        catch (e)
        {
            console.error("Parsing error:", e);
            console.log(event.data);
        }
        
        if (text.length)
        {
            document.getElementById("idSelectionMode").innerHTML = text;
        }
};
    
});
