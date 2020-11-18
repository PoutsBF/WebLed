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

domReady(function () {
    var idTimer;
    var connection = new WebSocket('ws://' + location.hostname + '/ws');
    //    var connection = new WebSocket('ws://' + location.hostname + ':81');
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
        if (connection.readyState === 1)
        {
            var choixMode = document.getElementById("idSelectionMode").selectedIndex;
            if (choixMode != -1)
            {
                connection.send("{mode:" + document.getElementById("idSelectionMode").selectedIndex + "}");
                document.getElementById("affModeSelect").innerText = document.getElementById("idSelectionMode").options[choixMode].label;
            }
            else
            {
                document.getElementById("affModeSelect").innerText = "pas de mode sélectionné";
            }
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
            var luminosite = document.getElementById("idLuminosite").value * 255 / 100;
            connection.send("{lum:" + luminosite + "}");
        }
    }

    document.getElementById("idSelectionMode").addEventListener("change", onSelectionMode);
    document.getElementById("idCouleur").addEventListener("change", onCouleur);

    function onTimerWS()
    {
      switch(connection.readyState)
      {
        case 0:
              document.getElementById("btInfoConnection").innerHTML = "en connexion";
              document.getElementById("btInfoConnection").setAttribute("title", "socket créé mais la connexion n'est pas encore ouverte");          
          break;
        case 1:
              document.getElementById("btInfoConnection").innerHTML = "connecté";
              document.getElementById("btInfoConnection").setAttribute("title", "Connexion ouverte et prête à communiquer");          
          break;
        case 2:
              document.getElementById("btInfoConnection").innerHTML = "fermeture en cours";
              document.getElementById("btInfoConnection").setAttribute("title", "Connexion encore ouverte, mais en cours de fermeture");          
          break;
        case 3:
              document.getElementById("btInfoConnection").innerHTML = "déconnecté";
              document.getElementById("btInfoConnection").setAttribute("title", "Connexion fermée ou ne pouvant être ouverte");          
          break;
      };
    }
    
    connection.onopen = function (evt)
    {
        connection.send('{\"Connect\":\"' + new Date() + '\"}');
        document.getElementById("btInfoConnection").classList.remove("btn-danger");
        document.getElementById("btInfoConnection").classList.add("btn-success");
        document.getElementById("IPserveurWS").innerHTML = location.hostname;
        idTimer = setInterval(onTimerWS, 3000);
    };

    connection.onerror = function (error)
    {
        console.log('WebSocket Error ', error);
    };

    connection.onclose = function (evt)
    {
        console.log('WebSocket fermeture ', evt);
        document.getElementById("btInfoConnection").classList.remove("btn-success");
        document.getElementById("btInfoConnection").classList.add("btn-danger");
        clearInterval(idTimer);
        onTimerWS();
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
//----------------------- range -----------------------------------------------
    var slider = new Slider('#idVitesse', {
        formatter: function (value) {
            return 'délai (ms): ' + value;
        },
        min: 1,
        max: 2000
    }).on("change", onVitesse);
    var slider = new Slider('#idLuminosite', {
        formatter: function (value) {
            return '%: ' + value;
        },
        min: 1,
        max: 100
    }).on("change", onLuminosite);
});  // fin domReady
