// Audio playing APIs - concept
var player;

function setup(){
    setTextSize(2);
    setTextColor(color(255,255,255));
    drawString("Audio playing POC", 4, 4);
    player = AudioPlayer();
    player.playSD("/Beyond-Single-海阔天空.mp3");
    player.setVolume(40);
    player.playMode(1);
}

function loop(){
    var status = getKeyStatus();
    var keys = status.keys;
    for (var i = 0; i < keys.length; i++) {
        if (keys[i] === " ") {
            if (player) {
                player.stop();
            }
        }
    }
}
