// Empty JS for your own code to be here

function domReady(f) {
  if (document.readyState === 'complete') {
    f();
  } else {
    document.addEventListener('DOMContentLoaded', f);
  }
}

domReady(function()
{
    function onSelectionMode() {
        if (connection.readyState === 1) {
            connection.send("{\"mode\":" + document.getElementById("idSelectionMode").selectedIndex + "}");
        }
    }

    document.getElementById("idSelectionMode").addEventListener("change", onSelectionMode);

    ColorPicker(document.getElementById('color-picker'), function (hex, hsv, rgb) {
        connection.send("{\"couleur\":" + ((rgb.r << 16) + (rgb.g << 8) + (rgb.b)) + "}");
    });

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
