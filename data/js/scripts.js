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
    ColorPicker(document.getElementById('color-picker'));

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
