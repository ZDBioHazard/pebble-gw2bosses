Pebble.addEventListener("ready", function( e ){
    var date = new Date();
    var offset = date.getTimezoneOffset();
    console.log("Sending offset " + offset + " to watch.");
    Pebble.sendAppMessage({"tz_offset": offset});
});
