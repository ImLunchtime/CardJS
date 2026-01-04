var menu = Menu();
var showingMenu = true;
var tracks = [];
var currentTrackIndex = -1;

function loadTracks() {
    var files = listFilesSD("/");
    for (var i = 0; i < files.length; i++) {
        var name = files[i];
        if (name.length > 4 && name.substring(name.length - 4).toLowerCase() === ".mp3") {
            var label = name.substring(0, name.length - 4);
            var index = tracks.length;
            tracks.push({ label: label, path: name });
            menu.addButtonItem(label, function(idx) {
                return function() {
                    currentTrackIndex = idx;
                    showingMenu = false;
                    menu.close();
                    drawTrackScreen(tracks[idx].label);
                };
            }(index));
        }
    }
    menu.refresh();
}

function drawTrackScreen(label) {
    var w = width();
    var h = height();
    var black = color(0, 0, 0);
    var white = color(255, 255, 255);
    drawFillRect(0, 0, w, h, black);
    setTextColor(white);
    setTextSize(2);
    drawString(label, 4, 4);
}

function setup() {
    loadTracks();
}

function loop() {
    if (showingMenu) {
        menu.update();
    } else {
        var status = getKeyStatus();
        if (status.esc) {
            showingMenu = true;
            menu.open();
            return;
        }
    }
}
