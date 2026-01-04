var menu = Menu();
var showingMenu = true;
var tracks = [];
var currentTrackIndex = -1;
var player = null;
var currentVolume = 10;

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
                    startTrack(idx);
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
    drawString("[-][+] Volume-/+ [Space] Stop", 4, 28);
    drawString("Volume: " + currentVolume + "%", 4, 50);
}

function startTrack(index) {
    currentTrackIndex = index;
    showingMenu = false;
    menu.close();
    if (!player) {
        player = AudioPlayer();
        player.setVolume(currentVolume);
    } else {
        player.stop();
    }
    player.playMode(0);
    player.playSD("/"+tracks[index].path);
    drawTrackScreen(tracks[index].label);
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
        var keys = status.keys;
        for (var i = 0; i < keys.length; i++) {
            var k = keys[i];
            if (k === " ") {
                if (player) {
                    player.stop();
                }
            } else if (k === "-") {
                currentVolume -= 10;
                if (currentVolume < 0) {
                    currentVolume = 0;
                }
                if (player) {
                    player.setVolume(currentVolume);
                }
                if (currentTrackIndex >= 0) {
                    drawTrackScreen(tracks[currentTrackIndex].label);
                }
            } else if (k === "=") {
                currentVolume += 10;
                if (currentVolume > 100) {
                    currentVolume = 100;
                }
                if (player) {
                    player.setVolume(currentVolume);
                }
                if (currentTrackIndex >= 0) {
                    drawTrackScreen(tracks[currentTrackIndex].label);
                }
            }
        }
    }
}
